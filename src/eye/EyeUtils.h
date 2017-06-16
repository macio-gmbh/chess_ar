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
    Mat blur, thres;
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
    for (int i = 0; i < vector.size(); i++)
    {
        vector[i].copyTo(mat.row(i));
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

    return calcFourierDescriptor(image);
}

}

#endif //CHESS_AR_EYEUTILS_H
