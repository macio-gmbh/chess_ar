#include <iostream>
#include <functional>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/ml.hpp>
#include <opencv2/video.hpp>

#include "./EyeUtils.h"
#include "../shared_lib/rabbitmq/RabbitMQSender.h"
#include "../shared_lib/ChessBoard.h"

#include <boost/archive/binary_iarchive.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

const bool USE_STATIC_IMAGE = false;

// chessboard detection, default values if config file not found
double cannyThreshold1 = 50;
double cannyThreshold2 = 515;

double houghLinesRho = 0.89;
double houghLinesTheta = 1.566;
double houghLinesThreshold = 61;

// quad detection
double minPeri = 220;
double maxPeri = 288;
double periScale = 0.05;

bool writeNextFigures = false;

Ptr<cv::ml::SVM> bishopSvm;
Ptr<cv::ml::SVM> knightSvm;
Ptr<cv::ml::SVM> pawnSvm;
Ptr<cv::ml::SVM> queenSvm;
Ptr<cv::ml::SVM> rookSvm;
Ptr<cv::ml::SVM> kingSvm;
Ptr<cv::ml::SVM> blackSvm;

// forward declaration functions
std::vector<std::vector<cv::Point> > GetChessQuads(cv::Mat grayImage);
void DrawConotursRandomColor(cv::Mat image, std::vector<std::vector<cv::Point> > contours);
void DetectFigures(ChessFigure chessFigures[], Mat originalImage, Mat inputImage, std::vector<std::vector<Point>> chessContours);

int main()
{
    // load the config ini file
    boost::property_tree::ptree config;
    boost::filesystem::path iniPath = "../config/eye.ini";

    if (!boost::filesystem::exists(iniPath))
    {
        fprintf(stderr, "Can't find the config file \n");
    }
    else
    {
        try
        {
            boost::property_tree::read_ini(iniPath.generic_string(), config);

            config.put("a.value", 3.14f);
            boost::property_tree::write_ini(iniPath.generic_string(), config);

            cannyThreshold1 = config.get("canny.cannyThreshold1", cannyThreshold1);
            cannyThreshold2 = config.get("canny.cannyThreshold2", cannyThreshold2);

            houghLinesRho = config.get("houghLines.houghLinesRho", houghLinesRho);
            houghLinesTheta = config.get("houghLines.houghLinesTheta", houghLinesTheta);
            houghLinesThreshold = config.get("houghLines.houghLinesThreshold", houghLinesThreshold);

            minPeri = config.get("quadDetection.minPeri", minPeri);
            maxPeri = config.get("quadDetection.maxPeri", maxPeri);
            periScale = config.get("quadDetection.periScale", periScale);
        }
        catch (std::exception &ex)
        {
            std::cerr << ex.what() << std::endl;
        }

    }

    // load the pretrained SVMs
    bishopSvm = bishopSvm->load("bishop.yml");
    knightSvm = knightSvm->load("knight.yml");
    pawnSvm = pawnSvm->load("pawn.yml");
    queenSvm = queenSvm->load("queen.yml");
    rookSvm = rookSvm->load("rook.yml");
    kingSvm = kingSvm->load("king.yml");
    blackSvm = blackSvm->load("black.yml");

    RabbitMQSender sender("localhost", 5672, "EyeToController");

    std::string initialBoard = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    ChessBoard currentBoard = ChessBoard(initialBoard);
    ChessFigure board[64];

    cv::VideoCapture capture(0);
    cv::Mat camFrame;
    cv::Mat chessImage, originalImage;
    bool staticImage = false;

    // open the camera
    if (USE_STATIC_IMAGE || !capture.isOpened())
    {
        std::cout << "cannot open camera, using the static image \n";
        staticImage = true;
        chessImage = cv::imread("../train/chessBoard/chess_8.png", 1);

        if (!chessImage.data)
        {
            printf("No image data \n");
            return -1;
        }

        capture.release();
        //return 0;
    }

    capture.set(CV_CAP_PROP_FRAME_WIDTH, 1280);
    capture.set(CV_CAP_PROP_FRAME_HEIGHT, 720);

    // main loop
    while (true)
    {
        if (!staticImage)
        {
            capture.read(camFrame);
        }
        else
        {
            // copy the opened test image to the camFrame so it wont be overwritten
            chessImage.copyTo(camFrame);
        }

        camFrame.copyTo(originalImage);

        // make a gray image
        cv::Mat gray;
        cvtColor(camFrame, gray, cv::COLOR_RGB2GRAY);
        std::vector<std::vector<cv::Point> > quads;

        try
        {
            // find the quads
            quads = GetChessQuads(gray);
            // test draw the contours onto the original image
            DrawConotursRandomColor(camFrame, quads);
        }
        catch (cv::Exception &e)
        {
            std::cerr << e.msg << std::endl; // output exception message
        }

        try
        {
            // find the figures
            DetectFigures(board, camFrame, gray, quads);
        }
        catch (cv::Exception &e)
        {
            std::cerr << "Error detecting figures: " << e.msg << std::endl; // output exception message
        }

        // recreate the chessboard

        // send the chessboard via rabbitMQ
        sender.Send(currentBoard.toString().c_str());

        // show the input image in a window
        imshow("cam", camFrame);
        //imshow("gray", gray);

        // check for key inputs
        char key = cv::waitKey(30);
        if (key == 'c')
        {
            imwrite("camFrame.png", originalImage);
        }
        if (key == 'f')
        {
            writeNextFigures = true;
        }
        else if (key == 27)
        {
            break;
        }
    }

    bishopSvm.release();
    knightSvm.release();
    pawnSvm.release();
    queenSvm.release();
    rookSvm.release();
    kingSvm.release();
    blackSvm.release();

    return 0;
}

std::vector<std::vector<cv::Point> > GetChessQuads(cv::Mat grayImage)
{
    // first: find the hough lines and draw em in a seperate mat
    cv::Mat edges;
    cv::Mat linesMat = cv::Mat(grayImage.rows, grayImage.cols, CV_8UC1, cv::Scalar(0, 0, 0));
    Canny(grayImage, edges, cannyThreshold1, cannyThreshold2, 3);

    std::vector<cv::Vec2f> lines;
    // detect lines
    HoughLines(edges, lines, houghLinesRho, houghLinesTheta, houghLinesThreshold);

    // draw the lines in a seperate mat for better rectangle finding
    for (size_t i = 0; i < lines.size(); i++)
    {
        float rho = lines[i][0], theta = lines[i][1];
        cv::Point pt1, pt2;
        double a = cos(theta), b = sin(theta);
        double x0 = a * rho, y0 = b * rho;
        pt1.x = cvRound(x0 + 1000 * (-b));
        pt1.y = cvRound(y0 + 1000 * (a));
        pt2.x = cvRound(x0 - 1000 * (-b));
        pt2.y = cvRound(y0 - 1000 * (a));
        line(linesMat, pt1, pt2, cv::Scalar(255, 255, 255));
    }

    if (linesMat.data)
        imshow("lines", linesMat);

    // then find the contours

    /// Find contours
    std::vector<std::vector<cv::Point> > contours;
    std::vector<cv::Vec4i> hierarchy;
    findContours(linesMat, contours, hierarchy,
                 CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);

    std::vector<std::vector<cv::Point> > quads;
    for (auto contour : contours)
    {
        double peri = arcLength(contour, true);
        //std::cout << peri << "\n";
        if (peri > maxPeri || peri < minPeri)
            continue;

        std::vector<cv::Point> approxContour;
        approxPolyDP(contour, approxContour, peri * periScale, true);

        // check if its a quad
        if (approxContour.size() == 4)
        {
            quads.push_back(approxContour);
        }
    }

    // sort the qauds
    std::sort(quads.begin(), quads.end(),
              [](const std::vector<cv::Point> &a, const std::vector<cv::Point> &b) -> bool
              {
                  // same row
                  if (abs(a[0].y - b[0].y) < 5)
                  {
                      return a[0].x < b[0].x;
                  }
                  else
                  {
                      return a[0].y < b[0].y;
                  }
              });

    return quads;
}

void DrawConotursRandomColor(cv::Mat image, std::vector<std::vector<cv::Point> > contours)
{
// always create the same random Range so its deterministic
    cv::RNG rng(12345);
    for (int i = 0; i < contours.size(); i++)
    {
        cv::Scalar color = cv::Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
        drawContours(image, contours, i, color, 2, 8);
    }
}

bool AreSvmsTrained()
{
    return bishopSvm->isTrained() && knightSvm->isTrained() && pawnSvm->isTrained() &&
           queenSvm->isTrained() && rookSvm->isTrained() && kingSvm->isTrained();

}

void DetectFigures(ChessFigure chessFigures[], Mat originalImage, Mat inputImage, std::vector<std::vector<Point>> chessContours)
{
    cv::Mat inputCopy, hsvImage;
    inputImage.copyTo(inputCopy);

    cvtColor(originalImage, hsvImage, cv::COLOR_BGR2HLS);
    std::vector<cv::Mat> hsv;
    cv::split(hsvImage, hsv);

    std::vector<cv::Mat> cells;
    std::vector<cv::Rect> cellsRect;
    std::vector<cv::Mat> cellsColor;

    // use the same size
    int rectSize = 65;

    for (int i = 0; i < chessContours.size(); ++i)
    {
        // seperate the figure fromt he rest
        cv::Rect boundRect = boundingRect(Mat(chessContours[i]));
        boundRect.x += 2;
        boundRect.y += 4;
        boundRect.width = rectSize;
        boundRect.height = rectSize;

        cv::Mat imageRoi = cv::Mat(inputCopy, boundRect);
        cv::Mat mask = EyeUtils::GetThresholdImage(imageRoi);
        cv::Mat descriptor = EyeUtils::GetDescriptor(imageRoi);
        cv::Mat colorDescriptor = EyeUtils::GetColorDescriptor(imageRoi);

        cells.push_back(descriptor);
        cellsRect.push_back(boundRect);
        cellsColor.push_back(colorDescriptor);

        if (writeNextFigures)
        {
            // save the image
            imwrite("image" + std::to_string(i) + ".png", imageRoi);
            imwrite("image" + std::to_string(i) + "_masked.png", mask);
        }
    }

    if (cells.size() > 0 && cells.size() == 64)
    {
        Mat mergedMat(cells.size(), cells[0].cols, CV_32FC1);
        EyeUtils::MergeMatrixVector(mergedMat, cells);

        Mat colorMergedMat(cellsColor.size(), cellsColor[0].cols, CV_32FC1);
        EyeUtils::MergeMatrixVector(colorMergedMat, cellsColor);

        // check the svms if a figure is detected
        if (AreSvmsTrained() && mergedMat.data)
        {
            Mat bishopResponse, knightResponse, pawnResponse, queenResponse,
                rookResponse, kingResponse, blackResponse;

            bishopSvm->predict(mergedMat, bishopResponse);
            knightSvm->predict(mergedMat, knightResponse);
            pawnSvm->predict(mergedMat, pawnResponse);
            queenSvm->predict(mergedMat, queenResponse);
            rookSvm->predict(mergedMat, rookResponse);
            kingSvm->predict(mergedMat, kingResponse);
            blackSvm->predict(colorMergedMat, blackResponse);

            int responseLength = bishopResponse.rows;

            for (int i = 0; i < responseLength; i++)
            {
                std::string figureName = "";
                ChessFigure figure;

                try
                {
                    auto isBishop = bishopResponse.at<float>(i, 0);
                    auto isKnight = knightResponse.at<float>(i, 0);
                    auto isPawn = pawnResponse.at<float>(i, 0);
                    auto isQueen = queenResponse.at<float>(i, 0);
                    auto isRook = rookResponse.at<float>(i, 0);
                    auto isKing = kingResponse.at<float>(i, 0);

                    cv::Rect boundRect = cellsRect.at(i);

                    if (isPawn)
                    {
                        figureName = "PAWN";
                        figure.figure_type = PAWN;
                    }
                    if (isKing)
                    {
                        figureName = "KING";
                        figure.figure_type = KING;
                    }
                    if (isQueen)
                    {
                        figureName = "QUEEN";
                        figure.figure_type = QUEEN;
                    }
                    if (isRook)
                    {
                        figureName = "ROOK";
                        figure.figure_type = ROOK;
                    }
                    if (isBishop)
                    {
                        figureName = "BISHOP";
                        figure.figure_type = BISHOP;
                    }
                    if (isKnight)
                    {
                        figureName = "KNIGHT";
                        figure.figure_type = KNIGHT;
                    }

                    if (!figureName.empty())
                    {
                        std::string figureColor = "white";

                        if (blackResponse.at<float>(i, 0))
                            figureColor = "black";

                        // Debug: write the figure name into the image
                        putText(originalImage, figureName, cv::Point(boundRect.x, boundRect
                            .y), FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255), 2);

                        // write the color
                        putText(originalImage, figureColor, cv::Point(boundRect.x, boundRect
                                                                                       .y
                                                                                   + 20), FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255), 2);

                        //std::cout << figureName << cellsColor.at(i) << std::endl;
                    }

                }
                catch (const std::out_of_range &ex)
                {
                    std::cerr << "Out of Range error: " << ex.what() << '\n';
                }
                catch (const cv::Exception &ex)
                {
                    std::cerr << "Out of Range error: " << ex.what() << '\n';
                }

                chessFigures[i] = figure;

            }
        }
        else
        {
            //std::cout << "No data " << std::endl;
        }
    }
    writeNextFigures = false;
}

