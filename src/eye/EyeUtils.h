//
// Created by marco on 15.06.2017.
//

#ifndef CHESS_AR_EYEUTILS_H
#define CHESS_AR_EYEUTILS_H

#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect.hpp>
#include "./FourierDescriptor.h"

using namespace cv;

namespace EyeUtils
{

cv::Mat GetThresholdImage(cv::Mat &image)
{
    Mat eqHist, blur, thres;
    equalizeHist(eqHist, eqHist);
    GaussianBlur(image, blur, Size(5, 5), 0);
    cv::threshold(blur, thres, 0, 255, THRESH_BINARY | THRESH_OTSU);

    return thres;
}

void ConvertVectortoMatrix(std::vector<float> &vector, cv::Mat &mat)
{
    for (int i = 0; i < vector.size(); i++)
    {
        mat.at<float>(0, i) = vector[i];
    }
}

void MergeMatrixVector(Mat &mat, std::vector<Mat> &vector)
{
    try
    {
        for (int i = 0; i < vector.size(); i++)
        {
            vector.at(i).copyTo(mat.row(i));
        }
    } catch (Exception e)
    {
        std::cout << e.msg << std::endl;
    }
}

cv::Mat GetDescriptor(cv::Mat &image)
{

    HOGDescriptor hog(
        Size(60, 60), //winSize
        Size(8, 8), //blocksize
        Size(4, 4), //blockStride,
        Size(8, 8), //cellSize,
        9, //nbins,
        1, //derivAper,
        -1, //winSigma,
        0, //histogramNormType,
        0.2, //L2HysThresh,
        0,//gammal correction,
        64,//nlevels=64
        1);

    std::vector<float> descriptors;
    hog.compute(image, descriptors);
    Mat descriptorMat(1, descriptors.size(), CV_32FC1);
    ConvertVectortoMatrix(descriptors, descriptorMat);

    return descriptorMat;

//    return calcFourierDescriptor(image);
}

cv::Mat GetColorDescriptor(cv::Mat &image)
{
    int histSize = 256;
    float range[] = {0, 256};
    const float *histRange = {range};
    bool uniform = true;
    bool accumulate = false;

    cv::Mat images[] = {image};
    cv::Mat hist;
    calcHist(images, 1, 0, Mat(), hist, 1, &histSize, &histRange, uniform, accumulate);
    transpose(hist, hist);
    return hist;

//    cv::Scalar mean, stddev;
//
//    cv::Mat mask = cv::Mat::zeros(image.size(), CV_8UC1);
//    cv::Point circleCenter(mask.cols / 2, mask.rows / 2);
//    int radius = 10;
//    cv::circle(mask, circleCenter, radius, CV_RGB(255, 255, 255));
//    cv::Mat imagePart = cv::Mat::zeros(image.size(), image.type());
//    image.copyTo(imagePart, mask);
//    meanStdDev(image, mean, stddev, mask);
//
//    Mat descriptor(1, 2, CV_32FC1);
//    std::vector<float> values;
//    values.push_back((float)mean.val[0]);
//    values.push_back((float)stddev.val[0]);
//
//    EyeUtils::ConvertVectortoMatrix(values, descriptor);
//
//    return descriptor;

}

}

#endif //CHESS_AR_EYEUTILS_H
