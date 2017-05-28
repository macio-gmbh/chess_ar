#include <iostream>
#include <unistd.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "../shared_lib/rabbitmq/RabbitMQSender.h"
#include "../shared_lib/ChessBoard.h"
#include "../shared_lib/Zobrist.h"
const bool USE_STATIC_IMAGE = true;


double minPeri = 100;
double maxPeri = 250;
double periScale = 0.05;

// forward declaration functions
std::vector<std::vector<cv::Point> > GetChessQuads(cv::Mat grayImage);

int main()
{
    RabbitMQSender sender("localhost", 5672, "eyeExchange");

    std::string initialBoard = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
    Zobrist initialZobrist = Zobrist(initialBoard);
    ChessBoard currentBoard = ChessBoard(initialBoard, initialZobrist.zobristHash);

    cv::VideoCapture capture(0);
    cv::Mat camFrame;
    cv::Mat chessImage;
    bool staticImage = false;

    // open the camera
    if (USE_STATIC_IMAGE || !capture.isOpened())
    {
        std::cout << "cannot open camera, using the static image \n";
        staticImage = true;
        chessImage = cv::imread("chess.png", 1);

        if (!chessImage.data)
        {
            printf("No image data \n");
            return -1;
        }

        capture.release();
        //return 0;
    }

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
        //equalizeHist(gray, gray);

        // find the quads
        std::vector<std::vector<cv::Point> > quads = GetChessQuads(gray);
        // test draw the contours onto the original image

        // always create the same random Range so its deterministic
        cv::RNG rng(12345);
        for (int i = 0; i < quads.size(); i++)
        {
            cv::Scalar color = cv::Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
            drawContours(camFrame, quads, i, color, 2, 8, NULL, 0, cv::Point());
        }

        // find the figures

        // recreate the chessboard

        // send the chessboard via rabbitMQ
        sender.Send(currentBoard.toString().c_str());

        // show the input image in a window
        imshow("cam", camFrame);
        imshow("gray", gray);
        if (cv::waitKey(30) == 27)
            break;

    }
    return 0;
}

std::vector<std::vector<cv::Point> > GetChessQuads(cv::Mat grayImage)
{
    // first: find the hough lines and draw em in a seperate mat
    cv::Mat edges;
    cv::Mat linesMat = cv::Mat(grayImage.rows, grayImage.cols, CV_8UC1, cv::Scalar(0, 0, 0));
    Canny(grayImage, edges, 50, 200, 3);

    std::vector<cv::Vec2f> lines;
    // detect lines
    HoughLines(edges, lines, 1, CV_PI / 180, 130);

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
        std::cout << peri << "\n";
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


//    Point[][]
//    contours = FindQuads(linesMat, src);
//
//    if (contours.Length == 64)
//    {
//        Debug.Log("Found all quads");
//    }
//    else
//    {
//        Debug.Log("Not all quads, need to interpolate some");
//    }
}

