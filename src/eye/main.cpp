#include <iostream>
#include <functional>
#include <ctime>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/ml.hpp>
#include <opencv2/video.hpp>
#include <opencv2/calib3d/calib3d.hpp>

#include "./EyeUtils.h"
#include "../shared_lib/rabbitmq/RabbitMQSender.h"
#include "../shared_lib/ChessBoard.h"

#include <boost/archive/binary_iarchive.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

const bool USE_STATIC_IMAGE = false;
const boost::filesystem::path imagePath = "./output/";

const boost::filesystem::path iniPath = "../config/eye.ini";
boost::filesystem::path staticImagePath = "../train/chessBoard/chess.png";
boost::filesystem::path calibPath = "../config/logitech_c920.yml";
int cameraIndex = 0;
bool debugDraw = false;

// chessboard detection, default values if config file not found
double chessCannyThreshold1 = 50;
double chessCannyThreshold2 = 150;
double chessHoughLinesRho = 1;
double chessHoughLinesTheta = 0.0174532930056254;
double chessHoughLinesThreshold = 100;

// quad detection
double cannyThreshold1 = 50;
double cannyThreshold2 = 515;
double houghLinesRho = 0.89;
double houghLinesTheta = 1.566;
double houghLinesThreshold = 61;
double minPeri = 220;
double maxPeri = 288;
double periScale = 0.05;

bool writeNextFigures = false;
cv::Mat lastUndistortedBoard;
int framesSinceLastBoard = 0;
time_t currentTimestamp;

Ptr<cv::ml::SVM> bishopSvm;
Ptr<cv::ml::SVM> knightSvm;
Ptr<cv::ml::SVM> pawnSvm;
Ptr<cv::ml::SVM> queenSvm;
Ptr<cv::ml::SVM> rookSvm;
Ptr<cv::ml::SVM> kingSvm;
Ptr<cv::ml::SVM> blackSvm;

// forward declaration functions
cv::Mat FindChessboard(cv::Mat originalImage, cv::Mat grayImage);
std::vector<std::vector<cv::Point> > GetChessQuads(cv::Mat grayImage);
void DrawConotursRandomColor(cv::Mat image, std::vector<std::vector<cv::Point> > contours);
void DetectFigures(Mat originalImage, Mat inputImage, std::vector<std::vector<Point>> chessContours,
                   std::array<ChessFigure, 64> &board);

void LoadConfig()
{
    // load the config ini file
    boost::property_tree::ptree config;

    if (!boost::filesystem::exists(iniPath))
    {
        std::cerr << "Can't find the config file" << std::endl;
    }
    else
    {
        try
        {
            boost::property_tree::read_ini(iniPath.generic_string(), config);

            staticImagePath = config.get("general.staticImage", staticImagePath);
            calibPath = config.get("general.calibPath", calibPath);
            cameraIndex = config.get("general.cameraIndex", cameraIndex);

            chessCannyThreshold1 = config.get("chessDetection.chessCannyThreshold1", chessCannyThreshold1);
            chessCannyThreshold2 = config.get("chessDetection.chessCannyThreshold2", chessCannyThreshold2);
            chessHoughLinesRho = config.get("chessDetection.chessHoughLinesRho", chessHoughLinesRho);
            chessHoughLinesTheta = config.get("chessDetection.chessHoughLinesTheta", chessHoughLinesTheta);
            chessHoughLinesThreshold = config.get("chessDetection.chessHoughLinesThreshold", chessHoughLinesThreshold);

            cannyThreshold1 = config.get("quadDetection.cannyThreshold1", cannyThreshold1);
            cannyThreshold2 = config.get("quadDetection.cannyThreshold2", cannyThreshold2);
            houghLinesRho = config.get("quadDetection.houghLinesRho", houghLinesRho);
            houghLinesTheta = config.get("quadDetection.houghLinesTheta", houghLinesTheta);
            houghLinesThreshold = config.get("quadDetection.houghLinesThreshold", houghLinesThreshold);
            minPeri = config.get("quadDetection.minPeri", minPeri);
            maxPeri = config.get("quadDetection.maxPeri", maxPeri);
            periScale = config.get("quadDetection.periScale", periScale);

            std::cout << "Loaded config file." << std::endl;
        }
        catch (std::exception &ex)
        {
            std::cerr << ex.what() << std::endl;
        }
    }
}

int main()
{
    // create the image output folder
    boost::filesystem::create_directories(imagePath);

    LoadConfig();

    Mat cameraMatrix, distCoeffs, newCamMatix;
    Rect distortRoi;

    if (!boost::filesystem::exists(calibPath))
    {
        std::cerr << "Can't find the calibration file" << std::endl;
    }
    else
    {
        try
        {
            // undisort the camera with the given calibration matrix
            FileStorage fs2(calibPath.generic_string(), FileStorage::READ);

            fs2["camera_matrix"] >> cameraMatrix;
            fs2["distortion_coefficients"] >> distCoeffs;

            if (cameraMatrix.data && distCoeffs.data)
            {
                newCamMatix = getOptimalNewCameraMatrix(cameraMatrix, distCoeffs,
                                                        Size(640, 480), 1, Size(1280, 720), &distortRoi);
            }
            else
            {
                std::cerr << "Camera calibration is invalid, using normal camera output." << std::endl;
            }
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
    std::array<ChessFigure, 64> board;
    cv::VideoCapture capture;

    // -1 == dont use the camera
    if (cameraIndex != -1)
    {
        capture = cv::VideoCapture(cameraIndex);
    }

    // local variables need for the main loop
    cv::Mat camFrame, gray, chessImage, originalImage, camFrameUndistort;
    std::vector<std::vector<cv::Point> > quads;
    std::vector<cv::Mat> chessFields;

    bool staticImage = false;

    // open the camera
    if (USE_STATIC_IMAGE || !capture.isOpened())
    {
        std::cout << "cannot open camera, using the static image \n";
        staticImage = true;
        chessImage = cv::imread(staticImagePath.generic_string(), 1);

        if (!chessImage.data)
        {
            fprintf(stderr, "No image data \n");
            return -1;
        }

        capture.release();
        //return 0;
    }

    //&& set the resolution from the webcam to 1280 x 720
    capture.set(CV_CAP_PROP_FRAME_WIDTH, 1280);
    capture.set(CV_CAP_PROP_FRAME_HEIGHT, 720);

    // main loop
    while (true)
    {
        std::time(&currentTimestamp);

        // get the image, from the camera or use the static image
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

        // undistort the image
        if (newCamMatix.data)
        {
            undistort(camFrame, camFrameUndistort, cameraMatrix, distCoeffs, newCamMatix);

            // crop the image if we got an roi
            if (distortRoi.width > 0 && distortRoi.height > 0)
                camFrameUndistort = Mat(camFrameUndistort, distortRoi);
        }
        else
        {
            // no disortion, use the normal image
            camFrame.copyTo(camFrameUndistort);
        }

        // make a gray image
        cvtColor(camFrameUndistort, gray, cv::COLOR_RGB2GRAY);
        cv::Mat chessBoardRoi, grayBoardRoi;

        camFrame.copyTo(chessBoardRoi);

        {
            chessBoardRoi = FindChessboard(originalImage, gray);
        }

        cvtColor(chessBoardRoi, grayBoardRoi, cv::COLOR_RGB2GRAY);
        {
            // find the quads
            quads = GetChessQuads(grayBoardRoi);
            // test draw the contours onto the original image
            DrawConotursRandomColor(chessBoardRoi, quads);
        }

        {
            // find the figures
            DetectFigures(chessBoardRoi, grayBoardRoi, quads, board);
        }


        // TODO: Sanity checks for figure Creation

        {
            // send the chessboard via rabbitMQ
            auto boardBeofreString = currentBoard.toString();
            currentBoard.board = board;
            auto boardAfterString = currentBoard.toString();
            sender.Send(currentBoard.toString().c_str());
        }

        // show the input image in a window
        if (chessBoardRoi.data)
            imshow("cam", chessBoardRoi);
        //imshow("gray", gray);

        // check for key inputs
        char key = cv::waitKey(30);
        if (key == 'c')
        {
            imwrite(imagePath.generic_string() + "originalImage" +
                    boost::lexical_cast<std::string>(currentTimestamp) + ".png", originalImage);
            imwrite(imagePath.generic_string() + "chessBoardRoi" +
                    boost::lexical_cast<std::string>(currentTimestamp) + ".png", chessBoardRoi);

        }
        if (key == 'f')
        {
            writeNextFigures = true;
        }
        if (key == 'd')
        {
            debugDraw = !debugDraw;
        }
        if (key == 'r')
        {
            LoadConfig();
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

void findOuterPoints(Size size, std::vector<Vec4i> &lines, Point2f &topLeft, Point2f &topRight, Point2f &bottomLeft, Point2f &bottomRight)
{
    std::vector<Vec4i> filteredLines;
    // top and buttom in image coordinates, 0 is upper left corner
    // initial parameters ar flipped so it can find the max
    int leftX = size.width;
    int rightX = 0;
    int topY = 0;
    int bottomY = size.height;

    topLeft = Point2f(leftX, bottomY);
    topRight = Point2f(rightX, bottomY);
    bottomLeft = Point2f(leftX, topY);
    bottomRight = Point2f(rightX, topY);

    for (auto currentLine : lines)
    {
        Point pt1 = Point(currentLine[0], currentLine[1]);
        Point pt2 = Point(currentLine[2], currentLine[3]);


        // fite out lines at the edges of the image
        // needs some tweaking
        int filterWitdh = 200;
        if (pt1.x < filterWitdh || pt1.x > (size.width - filterWitdh) || pt2.x < filterWitdh || pt2.x > (size.width - filterWitdh) )
		{
			continue;
		}

        double angle = atan2((double) pt2.y - pt1.y,
                             (double) pt2.x - pt1.x);

        double degress = abs(angle * 180 / CV_PI);

        if (degress >= 83 && degress <= 97)
        { 
            //vertical line
            filteredLines.push_back(currentLine);
        }
        else if ((degress >= 0 && degress <= 7) || degress >= 353)
        {
            //horizontal line
            filteredLines.push_back(currentLine);
        }
        else
        {
            continue;
        }
        
		
        // find the outline, first check if its top or bottom
        if (pt1.y > topY)
        {
            topY = pt1.y;
        }
        else if (pt2.y > topY)
        {
            topY = pt2.y;
        }
        else if (pt1.y < bottomY)
        {
            bottomY = pt1.y;
        }
        else if (pt2.y < bottomY)
        {
            bottomY = pt2.y;
        }
    }

    // we need to loop through twice or we'll be missing some lines
    for (auto currentLine : lines)
    {
        Point pt1 = Point(currentLine[0], currentLine[1]);
        Point pt2 = Point(currentLine[2], currentLine[3]);

        // if its top or bottom, find left and right
        // Top and bottom right or left could be different (depends on camera angle)
        // also lines are short so the buttom y line could be in the middle, thats why we need to recheck the coordinates
        int tempLeftX, tempRightX;
        int maxDist = 100;
        bool bottom = false;
        if (abs(pt1.y - bottomY) < maxDist || abs(pt2.y - bottomY) < maxDist)
        {
            tempLeftX = bottomLeft.x;
            tempRightX = bottomRight.x;
            bottom = true;
        }
        else if (abs(pt1.y - topY) < maxDist || abs(pt2.y - topY) < maxDist)
        {
            tempLeftX = topLeft.x;
            tempRightX = topRight.x;
        }
        else
        {
            // must be a line somewhere else, but not at bottom or top
            continue;
        }

        if (pt1.x < tempLeftX)
        {
            tempLeftX = pt1.x;
        }
        else if (pt2.x < tempLeftX)
        {
            tempLeftX = pt2.x;
        }
        else if (pt1.x > tempRightX)
        {
            tempRightX = pt1.x;
        }
        else if (pt2.x > tempRightX)
        {
            tempRightX = pt2.x;
        }

        if (bottom)
        {
            bottomLeft.x = tempLeftX;
            bottomRight.x = tempRightX;
        }
        else
        {
            topLeft.x = tempLeftX;
            topRight.x = tempRightX;
        }

    }

    topLeft.y = topY;
    topRight.y = topY;
    bottomLeft.y = bottomY;
    bottomRight.y = bottomY;

    lines = filteredLines;
}

// TODO: needs tweaking, not really persistent
cv::Mat FindChessboard(cv::Mat originalImage, cv::Mat grayImage)
{
    cv::Mat edges, chessBoardRoi, lineDebug, undistorted, blur;
    cv::Mat linesMat = cv::Mat(grayImage.rows, grayImage.cols, CV_8UC1, cv::Scalar(0, 0, 0));

    GaussianBlur(grayImage, blur, Size(5, 5), 0);
    Canny(blur, edges, chessCannyThreshold1, chessCannyThreshold2);

    grayImage.copyTo(lineDebug);

    std::vector<Vec4i> lines;
    HoughLinesP(edges, lines, chessHoughLinesRho, chessHoughLinesTheta, chessHoughLinesThreshold, 5, 4);

    Point2f topLeft, topRight, bottomLeft, bottomRight;
    findOuterPoints(edges.size(), lines, topLeft, topRight, bottomLeft, bottomRight);

    for (auto currentLine : lines)
    {
        Point pt1 = Point(currentLine[0], currentLine[1]);
        Point pt2 = Point(currentLine[2], currentLine[3]);
        // draw the line for debugging into the debug window
        line(lineDebug, pt1, pt2, cv::Scalar(255, 255, 255), 3);
    }

    int size = 800;
    int height = abs(topLeft.y - bottomLeft.y);
    int width = abs(topLeft.x - topRight.x);

    // check if its quadratic
    //if (false)
    if (abs(height - width) < 40)
    {
        Point2f newbottomLeft = Point2f(0, 0);
        Point2f newbottomRight = Point2f(size, 0);
        Point2f newtopLeft = Point2f(0, size);
        Point2f newtopRight = Point2f(size, size);

        Point2f src[] = {topLeft, topRight, bottomLeft, bottomRight};
        Point2f dst[] = {newtopLeft, newtopRight, newbottomLeft, newbottomRight};

        Mat m = cv::getPerspectiveTransform(src, dst);
        cv::warpPerspective(originalImage, undistorted, m, cv::Size(size, size));

        undistorted.copyTo(lastUndistortedBoard);
        framesSinceLastBoard = 0;
    }
    else
    {
        // we havent found the boardboard (needs to be quadratic) so return the original or cached image

        if (framesSinceLastBoard < 25 && lastUndistortedBoard.data)
        {
            // use the cached board, brings better persistent images
            lastUndistortedBoard.copyTo(undistorted);
            framesSinceLastBoard++;
        }
        else
        {
            // to many frames since we found the board, show the input image
            undistorted = originalImage;
        }

    }

    if (debugDraw)
    {
        if (edges.data)
            imshow("chessCanny", edges);

        if (lineDebug.data)
            imshow("lineDebug", lineDebug);
    }

    return
        undistorted;
}

std::vector<std::vector<cv::Point> > GetChessQuads(cv::Mat grayImage)
{
    // first: find the hough lines and draw em in a seperate mat
    cv::Mat edges;
    cv::Mat linesMat = cv::Mat(grayImage.rows, grayImage.cols, CV_8UC1, cv::Scalar(0, 0, 0));
    Canny(grayImage, edges, cannyThreshold1, cannyThreshold2, 3);

    if (debugDraw)
    {
        imshow("canny", edges);
    }

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

    if (debugDraw)
    {
        if (linesMat.data)
            imshow("lines", linesMat);
    }

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

void DetectFigures(Mat originalImage, Mat inputImage, std::vector<std::vector<Point>> chessContours,
                   std::array<ChessFigure, 64> &board)
{
    cv::Mat inputCopy;
    inputImage.copyTo(inputCopy);

    std::vector<cv::Mat> cells;
    std::vector<cv::Rect> cellsRect;
    std::vector<cv::Mat> cellsColor;

    for (int i = 0; i < chessContours.size(); ++i)
    {
        // seperate the figure fromt he rest
        try
        {
            cv::Rect boundRect = boundingRect(Mat(chessContours.at(i)));


			// thus dont make any sense
            if (!(boundRect.width > 0 && boundRect.width > 0))
                continue;


            // check if the boundRect is inside the image plane
            if (!(0 <= boundRect.x
                  && 0 <= boundRect.width
                  && boundRect.x + boundRect.width <= inputCopy.cols
                  && 0 <= boundRect.y
                  && 0 <= boundRect.height
                  && boundRect.y + boundRect.height <= inputCopy.rows))
            {
                continue;
            }

            cv::Mat imageRoi = cv::Mat(inputCopy, boundRect);

            if (!imageRoi.data)
                continue;

            // resize the image so its always 65 (was trained with 65er size)
            resize(imageRoi, imageRoi, Size(65, 65), 0, 0);

            cv::Mat mask = EyeUtils::GetPreprocessedFigure(imageRoi);
            cv::Mat descriptor = EyeUtils::GetDescriptor(imageRoi);
            cv::Mat colorDescriptor = EyeUtils::GetColorDescriptor(imageRoi);

            cells.push_back(descriptor);
            cellsRect.push_back(boundRect);
            cellsColor.push_back(colorDescriptor);

            if (writeNextFigures)
            {
                // save the image
                imwrite(imagePath.generic_string() + "image" + std::to_string(i) +
                        boost::lexical_cast<std::string>(currentTimestamp) + ".png", imageRoi);

                //imwrite(imagePath.generic_string() + "image" + std::to_string(i) + "_masked.png", mask);
            }
        }
        catch (std::exception &ex)
        {
            std::cerr << ex.what() << '\n';
        }
    }

    if (cells.size() > 0 && cells.size() > 50)
    {
        try
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
                    if (i >= 64)
                    {
                        break;
                    }

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
                            figure.color = WHITE;

                            if (blackResponse.at<float>(i, 0))
                            {
                                figureColor = "black";
                                figure.color = BLACK;
                            }

                            // Debug: write the figure name into the image
                            putText(originalImage, figureName, cv::Point(boundRect.x, boundRect
                                .y), FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255), 2);

                            // write the color
                            putText(originalImage, figureColor, cv::Point(boundRect.x, boundRect
                                                                                           .y
                                                                                       + 20), FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255), 2);

                            //std::cout << figureName << cellsColor.at(i) << std::endl;


                            board.at(i) = figure;

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

                }
            }
            else
            {
                //std::cout << "No data " << std::endl;
            }
        }
        catch (Exception &ex)
        {
            std::cerr << "Error while detecting the figures: " << ex.what() << std::endl;
        }
    }
    writeNextFigures = false;
}

