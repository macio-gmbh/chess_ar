#include <iostream>
#include <unistd.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include "MatchDescriptor.h"
#include "../shared_lib/rabbitmq/RabbitMQSender.h"
#include "../shared_lib/ChessBoard.h"
#include "../shared_lib/Zobrist.h"
const bool USE_STATIC_IMAGE = true;

double minPeri = 100;
double maxPeri = 250;
double periScale = 0.05;

// forward declaration functions
std::vector<std::vector<cv::Point> > GetChessQuads(cv::Mat grayImage);
void DrawConotursRandomColor(cv::Mat image, std::vector<std::vector<cv::Point> > contours)
void DetectFigures(cv::Mat inputImage, std::vector<std::vector<cv::Point>> chessContours);

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
        //DrawConotursRandomColor(camFrame, quads);

        // find the figures
        DetectFigures(gray, quads);

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

void DetectFigures(cv::Mat inputImage, std::vector<std::vector<cv::Point>> chessContours)
{

    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;
    cv::Mat mTest, mThresh, mConnected;

    cv::Mat edges, m, m1, m2, m3, m4, m5;
    //cv::Mat m = cv::imread("scene2.jpg", CV_LOAD_IMAGE_GRAYSCALE);
    inputImage.copyTo(m);

    // todo better threshold here
    bitwise_not(m, m1);

    medianBlur(m1, m1, 3);
    morphologyEx(m1, m1, cv::MorphTypes::MORPH_OPEN, NULL);
    morphologyEx(m1, m1, cv::MorphTypes::MORPH_CLOSE, NULL);

    double minThreshold = 36;
    double maxThreshold = 129;
    int apertureSize = 3;

    Canny(m1, edges, minThreshold, maxThreshold, apertureSize);

    //threshold(m1, mThresh, 125, 255, cv::THRESH_BINARY);
    findContours(edges, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_NONE);
    DrawConotursRandomColor(inputImage, contours);

    if (mThresh.data)
        imshow("mThresh", mThresh);

    int sizeMax = 0, idx = 0;
    std::vector<int> ctrSelec;
    for (int i = 0; i < contours.size(); i++)
    {
        if (contours[i].size() >= 500)
        {
            ctrSelec.push_back(i);
        }
    }

    cv::Mat mc = cv::Mat::zeros(m.size(), CV_8UC3);
    std::vector<std::vector<cv::Point2d>> z;
    std::vector<std::vector<cv::Point2d>> Z;

    z.resize(ctrSelec.size());
    Z.resize(ctrSelec.size());
    for (int i = 0; i < ctrSelec.size(); i++)
    {
        std::vector<cv::Point2d> c = MatchDescriptor::ReSampleContour(contours[ctrSelec[i]], 1024);
        for (int j = 0; j < c.size(); j++)
            z[i].push_back(c[(j + i * 10) % c.size()]);
        dft(z[i], Z[i], cv::DFT_SCALE | cv::DFT_REAL_OUTPUT);
    }

    int indRef = 0;
    MatchDescriptor md;
    md.sContour = Z[indRef];
    md.nbDesFit = 20;
    std::vector<float> alpha, phi, s;
    std::vector<std::vector<Point>> ctrRotated;
    alpha.resize(ctrSelec.size());
    phi.resize(ctrSelec.size());
    s.resize(ctrSelec.size());


    // print the results

    for (int i = 0; i < ctrSelec.size(); i++)
    {
        md.AjustementRtSafe(Z[i], alpha[i], phi[i], s[i]);
        complex<float> expitheta = s[i] * complex<float>(cos(phi[i]), sin(phi[i]));
        cout << "Contour " << indRef << " with " << i << " origin " << alpha[i] << " and rotated of "
             << phi[i] * 180 / md.pi << " and scale " << s[i] << " Distance between contour is "
             << md.Distance(expitheta, alpha[i]) << " " << endl;
        for (int j = 1; j < Z[i].size(); j++)
        {
            complex<float> zr(Z[indRef][j].x, Z[indRef][j].y);
            zr = zr * expitheta * exp(alpha[i] * md.frequence[j] * complex<float>(0, 1));
            Z[i][j].x = zr.real();
            Z[i][j].y = zr.imag();
        }
        dft(Z[i], z[i], DFT_INVERSE);
        std::vector<Point> c;
        for (int j = 0; j < z[i].size(); j++)
            c.push_back(Point(z[i][j].x, z[i][j].y));
        ctrRotated.push_back(c);
    }
    for (int i = 0; i < ctrSelec.size(); i++)
    {
        if (i != indRef)
            drawContours(mc, contours, ctrSelec[i], Scalar(255, 0, 0));
        else
            drawContours(mc, contours, ctrSelec[i], Scalar(255, 255, 255));
        putText(mc, format("%d", i), Point(Z[i][0].x, Z[i][0].y), FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 0, 0));
    }

    for (int i = 0; i < ctrSelec.size(); i++)
    {
        drawContours(mc, ctrRotated, i, Scalar(0, 0, 255));
    }

    if (mc.data)
        imshow("mc", mc);

}

