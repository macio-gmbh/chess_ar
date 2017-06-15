#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/ml.hpp>

#include "./EyeUtils.h"
#include "./FourierDescriptor.h"
#include "../shared_lib/rabbitmq/RabbitMQSender.h"
#include "../shared_lib/ChessBoard.h"

#include <boost/archive/binary_iarchive.hpp>
#include <boost/filesystem.hpp>

const bool USE_STATIC_IMAGE = false;

double minPeri = 200;
double maxPeri = 288;
double periScale = 0.05;
std::vector<std::pair<cv::Mat, int>> dict;
bool writeNextFigures = false;

// forward declaration functions
std::vector<std::vector<cv::Point> > GetChessQuads(cv::Mat grayImage);
void DrawConotursRandomColor(cv::Mat image, std::vector<std::vector<cv::Point> > contours);
void DetectFigures(cv::Mat &originalImage, cv::Mat &inputImage, std::vector<std::vector<cv::Point>> const &chessContours,
                   cv::ml::SVM *svm);

int main()
{
    Ptr<cv::ml::SVM> svm;

    if (!boost::filesystem::exists("model4.yml"))
    {
        std::cout << "Can't find the SVM Training file, starting without piece recognition" << std::endl;
    }
    else
    {
        svm = svm->load("model4.yml");
        std::cout << "Var count: " << svm->getVarCount() << std::endl;
    }

    RabbitMQSender sender("localhost", 5672, "EyeToController");

    std::string initialBoard = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    ChessBoard currentBoard = ChessBoard(initialBoard);

    cv::VideoCapture capture(1);
    cv::Mat camFrame;
    cv::Mat chessImage;
    bool staticImage = false;

    // open the camera
    if (USE_STATIC_IMAGE || !capture.isOpened())
    {
        std::cout << "cannot open camera, using the static image \n";
        staticImage = true;
        chessImage = cv::imread("chessBoard/chess.png", 1);

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

        // make a gray image
        cv::Mat gray;
        cvtColor(camFrame, gray, cv::COLOR_RGB2GRAY);

        // find the quads
        std::vector<std::vector<cv::Point> > quads = GetChessQuads(gray);

        // test draw the contours onto the original image
        DrawConotursRandomColor(camFrame, quads);

        // find the figures
        DetectFigures(camFrame, gray, quads, svm);

        // recreate the chessboard

        // send the chessboard via rabbitMQ
        sender.Send(currentBoard.toString().c_str());

        // show the input image in a window
        imshow("cam", camFrame);
        //imshow("gray", gray);
        char key = cv::waitKey(30);
        if (key == 'c')
        {
            imwrite("camFrame.png", camFrame);
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

    svm.release();

    return 0;
}

std::vector<std::vector<cv::Point> > GetChessQuads(cv::Mat grayImage)
{
    // first: find the hough lines and draw em in a seperate mat
    cv::Mat edges;
    cv::Mat linesMat = cv::Mat(grayImage.rows, grayImage.cols, CV_8UC1, cv::Scalar(0, 0, 0));
    Canny(grayImage, edges, 50, 515, 3);

    std::vector<cv::Vec2f> lines;
    // detect lines
    HoughLines(edges, lines, 0.89, 1.56, 61);

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

std::vector<std::vector<cv::Point>> MergeContours(std::vector<std::vector<cv::Point>> contours)
{

    std::vector<std::vector<cv::Point>> hull;
    std::vector<cv::Point> points;

    for (auto contour : contours)
    {
        for (auto point : contour)
        {
            points.push_back(point);
        }
    }
    // merge contours
    double peri = arcLength(points, true);

    std::vector<cv::Point> approxContour;
    approxPolyDP(points, approxContour, peri * 0.001, true);

    hull.push_back(approxContour);

    return hull;
}

double getPeriSum(std::vector<std::vector<cv::Point>> contours)
{
    double peri = 0;
    for (auto contour : contours)
    {
        peri += arcLength(contour, true) * 2;
    }
    // merge contours
    return peri;
}

int computeOutput(int x, int r1, int s1, int r2, int s2)
{
    float result;
    if (0 <= x && x <= r1)
    {
        result = s1 / r1 * x;
    }
    else if (r1 < x && x <= r2)
    {
        result = ((s2 - s1) / (r2 - r1)) * (x - r1) + s1;
    }
    else if (r2 < x && x <= 255)
    {
        result = ((255 - s2) / (255 - r2)) * (x - r2) + s2;
    }
    return (int) result;
}

void MergeMatrixVector(Mat &mat, std::vector<cv::Mat> &vector)
{
    for (int i = 0; i < vector.size(); i++)
    {
        vector[i].copyTo(mat.row(i));
    }
}

void DetectFigures(cv::Mat &originalImage, cv::Mat &inputImage, std::vector<std::vector<cv::Point>> const &chessContours,
                   cv::ml::SVM *svm)
{
    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::Mat mTest, mThresh, mConnected;
    cv::Mat edges, inputCopy, m1;
    //cv::Mat inputCopy = cv::imread("scene2.jpg", CV_LOAD_IMAGE_GRAYSCALE);
    inputImage.copyTo(inputCopy);

    std::vector<cv::Mat> cells;
    std::vector<cv::Rect> cellsRect;

    // use the same size
    int rectSize = 60;

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

        cv::Mat descriptor = EyeUtils::GetDescriptor(mask);
        cells.push_back(descriptor);
        cellsRect.push_back(boundRect);

        if (writeNextFigures)
        {
            imwrite("image" + std::to_string(i) + ".png", imageRoi);
            imwrite("image" + std::to_string(i) + "_masked.png", mask);
        }
    }

    Mat mergedMat(cells.size(), cells[0].cols, CV_32FC1);
    MergeMatrixVector(mergedMat, cells);

    if (svm->isTrained() && mergedMat.data && svm->getVarCount() == mergedMat.cols)
    {
        Mat svmResponse;
        svm->predict(mergedMat, svmResponse);
        std::string figureName = "";

        for (int i = 0; i < svmResponse.rows; i++)
        {

            int id = round(svmResponse.at<float>(0, i));
            std::cout << id << std::endl;

            try
            {
                cv::Rect boundRect = cellsRect.at(i);

                switch (id)
                {
                case KING:figureName = "King";
                    break;
                case QUEEN:figureName = "QUEEN";
                    break;
                case ROOK:figureName = "ROOK";
                    break;
                case BISHOP:figureName = "BISHOP";
                    break;
                case KNIGHT:figureName = "KNIGHT";
                    break;
                case PAWN:figureName = "PAWN";
                    break;
                }

                std::cout << std::to_string(id) << std::endl;

                putText(originalImage, figureName, cv::Point(boundRect.x, boundRect
                    .y), FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 255), 2);
            }
            catch (const std::out_of_range &ex)
            {
                std::cerr << "Out of Range error: " << ex.what() << '\n';
            }

        }
    }
    else
    {
        //std::cout << "No data " << std::endl;
    }

    writeNextFigures = false;
}
