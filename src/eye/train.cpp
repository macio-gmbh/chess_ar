#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include "FourierDescriptor.h"

#include "cvmat_serialization.h"
#include <boost/serialization/vector.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>

using namespace cv;

Mat getMask(Mat &image);

/**
 * calculates the contour line of all objects in an image
 * @param img  the input image
 * @param objList  vector of contours, each represented by a two-channel matrix
 * @param thresh  threshold used to binarize the image
 * @param k  number of applications of the erosion operator
 */
void getContourLine(const Mat &matImg,
                    std::vector<Mat> &vmatObjList,
                    const int &iThresh,
                    const int &k)
{
    // copy image as the following methods altered it
    Mat imageCopy = matImg.clone();

    //image preparation
    threshold(imageCopy, imageCopy, iThresh,
              255, //set all points meeting the threshold to 255
              THRESH_BINARY //output is a binary image
    );

    //perform closing --> dilate first, then erode
    dilate(imageCopy, imageCopy,
           Mat(), //3x3 square kernel
           Point(-1, -1), //upper left corner
           1);

    erode(imageCopy, imageCopy,
          Mat(), //Mat() leads to a 3x3 square kernel
          Point(-1, -1), //upper corner
          k);

    /*namedWindow("test", CV_WINDOW_AUTOSIZE);
    imshow("test", img);
    waitKey(0);*/

    findContours(imageCopy, vmatObjList,
                 CV_RETR_LIST, //only outer contours
                 CV_CHAIN_APPROX_NONE //no approximation
    );
}

int test_opencv()
{
    Mat image, image9;
    image = imread("chessBoard/image13.png", 1);
    image9 = imread("chessBoard/image16.png", 1);

    if (!image.data)
    {
        printf("No image data \n");
        return -1;
    }
    getMask(image);

    waitKey(0);

    return 0;
}

// parameters
//! Threshold for image binarization.
static const int iBinThreshold = 98;
//! Number of applications of the erosion operator.
static const int iNumOfErosions = 2;
//! Number of dimensions in the FD.
static int iFDNormDimensions = 128;
//! Threshold for detection.
static const double dDetectionThreshold = 0.005;
//! Delay before program is resumed after image display.
static const int iImageDelay = 0;

/**
 * plot fourier descriptor
 * @param fd  the fourier descriptor to be displayed
 * @param win  the window name
 * @param dur  wait number of ms or until key is pressed
 */

void plotFD(const Mat matFD,
            const std::string &szWindowName,
            const double &dDuration)
{
    Mat matInvFD;
    float fMaxX = -HUGE_VAL; //later image dimensions
    float fMaxY = -HUGE_VAL;
    int iOffsetX = 0; //scaled and rounded offset
    int iOffsetY = 0;

    matInvFD = matFD.clone();
    dft(matInvFD, matInvFD, DFT_INVERSE | DFT_SCALE); //inverse dft

    for (int i = 0; i < matInvFD.rows; i++) //find maximum and minimum image dimensions
    {
        fMaxX = max(matInvFD.at<Vec2f>(0, i)[0], fMaxX);
        fMaxY = max(matInvFD.at<Vec2f>(0, i)[1], fMaxY);
    }

    float fScale = 100.0 / min(fMaxX, fMaxY); //maximum image dimension = 100px
    std::cout << "plotFD: Scale factor is " << fScale << std::endl;
    matInvFD *= fScale;

    matInvFD.convertTo(matInvFD, CV_32SC2);

    for (int i = 0; i < matInvFD.rows; i++) //find maximum and minimum image dimensions
    {
        iOffsetX = min(matInvFD.at<Vec2i>(0, i)[0], iOffsetX);
        iOffsetY = min(matInvFD.at<Vec2i>(0, i)[1], iOffsetY);
    }

    Mat img = Mat::zeros(ceil(fMaxY * fScale) - iOffsetY, ceil(fMaxX * fScale) - iOffsetX, CV_8UC3);
#if DEBUG_MODE
    cout << "plotFD: Offsets are (" << iOffsetX << " | " << iOffsetY << "), image dimensions are (" << ceil(fMaxX*fScale)-iOffsetX << " | " << ceil(fMaxY*fScale)-iOffsetY << ")" << endl;
#endif
    for (int i = 0; i < matInvFD.rows; i++)
    {
        //we still may have negative coordiates at this point as we centered our contour around (0,0) --> use the offset
        img.at<Vec3b>(
            matInvFD.at<Vec2i>(0, i)[1] - iOffsetY, matInvFD.at<Vec2i>(0, i)[0] - iOffsetX) = Vec3b(255, 255, 255);
    }

    //namedWindow(szWindowName);
    //imshow(szWindowName, img);
    imwrite(szWindowName, img);
    //waitKey(dDuration);

}

Mat getMask(Mat &image)
{
    Mat image9;
    Mat blurred, Morpholige, edges, img1;
    image.copyTo(img1);

    //bitwise_not(imageRoi, m1);
    medianBlur(image, blurred, 3);
    morphologyEx(blurred, Morpholige, cv::MorphTypes::MORPH_OPEN, NULL);
    morphologyEx(Morpholige, Morpholige, cv::MorphTypes::MORPH_CLOSE, NULL);

    double minThreshold = 100;
    double maxThreshold = 200;
    int apertureSize = 3;

    // detect edges
    Canny(Morpholige, edges, minThreshold, maxThreshold, apertureSize);

    std::vector<std::vector<cv::Point>> contours;
    std::vector<cv::Vec4i> hierarchy;

    //threshold(m1, mThresh, 125, 255, cv::THRESH_BINARY);
    //findContours(edges, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

    findContours(edges, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

    // you could also reuse img1 here
    Mat mask = Mat::zeros(img1.rows, img1.cols, CV_8UC1);

    // CV_FILLED fills the connected components found
    drawContours(mask, contours, -1, Scalar(255), CV_FILLED);

    Mat kernel = getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(11, 11));
    morphologyEx(mask, mask, cv::MorphTypes::MORPH_CLOSE, kernel);

    //imshow("Display Image", edges);

    //if (mask.data)
    //imshow("contours", mask);

    return mask;
}

void train(std::string target_path, std::vector<std::pair<Mat, int>> &dict, int classId)
{

    boost::filesystem::directory_iterator end_itr; // Default ctor yields past-the-end
    for (boost::filesystem::directory_iterator i(target_path); i != end_itr; ++i)
    {
        //std::string file = "chessBoard/image" + std::to_string(i) + ".png";
        Mat matExC1 = imread((*i).path().string(), 1);
        Mat edges;

        //imshow("matExC1", matExC1);

        if ((!matExC1.data))
        {
            continue;
        }

        //std::vector<Mat> vmatContourLines1;
        //getContourLine(matExC1, vmatContourLines1, iBinThreshold, iNumOfErosions);

        //threshold(matExC1, matExC1, 5 * 0.98f, 255, CV_THRESH_BINARY);
        matExC1 = getMask(matExC1);
        imshow("test", matExC1);
        //waitKey(0);

        Mat ft; // Fourier descriptor vector
        //int dist; // store the label of the minimum distance neighbor
        ft = calcFourierDescriptor(matExC1); // compute the Fourier Descriptor for this image

        if (ft.data)
        {
            std::pair<cv::Mat, int> fd;
            fd.first = ft;
            fd.second = classId;
            dict.push_back(fd);
        }

        //dist = classifier(dictionary, ft); // returns detected gesture from dictionary
    }
}

int main()
{
    //return test_opencv ();
    std::vector<std::pair<Mat, int>> dict;

    namespace fs = boost::filesystem;
    train("chessBoard/empty/", dict, -1);
    train("chessBoard/king/", dict, 1);
    train("chessBoard/queen/", dict, 2);
    train("chessBoard/rook/", dict, 3);
    train("chessBoard/bishop/", dict,4);
    train("chessBoard/knight/", dict ,5);
    train("chessBoard/pawn/", dict, 6);

//
//    //0 - 63
//    for (int i = 9; i < 64; ++i)
//    {
//        std::string file = "chessBoard/image" + std::to_string(i) + ".png";
//        Mat matExC1 = imread(file, CV_LOAD_IMAGE_GRAYSCALE);
//
//        if ((!matExC1.data))
//        {
//            continue;
//        }
//        //std::vector<Mat> vmatContourLines1;
//        //getContourLine(matExC1, vmatContourLines1, iBinThreshold, iNumOfErosions);
//
//        matExC1 = getMask(matExC1);
//        imshow("test", matExC1);
//        //waitKey(0);
//
//        Mat ft; // Fourier descriptor vector
//        int dist; // store the label of the minimum distance neighbor
//        ft = calcFourierDescriptor(matExC1); // compute the Fourier Descriptor for this image
//
//
//        if (ft.data)
//        {
//            //imshow("test", ft);
//            dist = classifier(dict, ft); // returns detected gesture from dictionary
//            std::cout << std::to_string(dist) << std::endl;
//        }
//    }

    std::ofstream ofs("dict.bin", std::ios::out | std::ios::binary);
    { // use scope to ensure archive goes out of scope before stream
        boost::archive::binary_oarchive oa(ofs);
        oa << dict;

    }
    ofs.close();
}
