//
// Created by marco on 29.05.2017.
// Code taken and adpated from www.bu.edu/vip/files/pubs/reports/CCSSM13-04buece.pdf
//

#ifndef CHESS_AR_FOURIERDESCRIPTOR_H
#define CHESS_AR_FOURIERDESCRIPTOR_H

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>

using namespace cv;

Point findCenter(Mat wdw)
{
// find the center position of the hand
    double colSum = 0.f, rowSum = 0.f, totSum = 0.f;
    int ctrRow = 0, ctrCol = 0;
    Point ctrPt;
    for (int ii = 0; ii < wdw.cols; ii++)
    {
        for (int jj = 0; jj < wdw.rows; jj++)
        {
            if (wdw.at<uchar>(Point(ii, jj)) != 0)
            {
                colSum = colSum + ii;
                rowSum = rowSum + jj;
                totSum++;
            }
        }
    }
// center of the hand in cartesian coords
    ctrPt.x = static_cast<int>(colSum / totSum);
    ctrPt.y = static_cast<int>(rowSum / totSum);
    return ctrPt;
}


//////////////////////////////////////
// Basic classifier using Euclidean distance
// Inputs:
// lib = Lib loaded from premade dictionary
// src = Fourier descriptors for input image
// Outputs:
// Mat = Euclidean distance between the src Fourier descriptor and each label in the dictionary
//////////////////////////////////////
int classifier(std::vector <std::pair<Mat, int>> dict, Mat src)
{
    int ctr = 1;
    int minIdx = -1;
    double minVal = 99999;
    double currVal;
// Iterate through Dictionary
    for (auto fd : dict)
    {
        currVal = norm(fd.first, src, NORM_L2); // compute Euclidean distance
        if (currVal < minVal)
        {
            minVal = currVal;
            minIdx = fd.second;
        }
        ctr++;
    }
    return minIdx;
}

Mat calcFourierDescriptor(Mat wdw)
{
    // Find the center position of the hand
    Point ctrPt;
    ctrPt = findCenter(wdw);

    // Find the contours of the silhouette
    std::vector <std::vector<cv::Point>> contours;
    std::vector <Vec4i> hierarchy;

    findContours(wdw, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

    /// Approximate contours to polygons + get bounding rects and circles
    std::vector <std::vector<Point>> contours_poly(contours.size());
    std::vector <Rect> boundRect(1);
    double maxArea = 0.0f; // store the max area of all the convex hulls
    int maxInd = 0; // store contour id of max area
    for (int i = 0; i < contours.size(); i++)
    {
        approxPolyDP(Mat(contours[i]), contours_poly[i], 3, true);
        double currArea = cv::contourArea(contours_poly[i]);
        if (currArea > maxArea)
        {
            maxInd = i;
            maxArea = currArea;
        }
    }
    int lineThickness = -1;
// blank out the window
    wdw(Range::all(), Range::all()) = 0;
// maxInd = largest contour
    drawContours(wdw, contours, maxInd, 255, lineThickness, 8, std::vector<Vec4i>(), 0,
                 Point());
    Mat frame;
    wdw.copyTo(frame); // Make working copy
////////////////////////
// Repeat previous steps
///////////////////////
    ctrPt = findCenter(frame);


    // Find the contours of the silhouette
    contours.clear();
    hierarchy.clear();
    findContours(frame, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

    if (contours.size() == 0)
        return Mat();

// Remove contours that are along the bottom edge of the window
    size_t dim1 = contours[0].size(); // number of contour points
    int r = wdw.rows, c = wdw.cols;
    for (int ii = 0; ii < dim1; ii++)
    {
        if ((contours[0][ii].x == 0) || (contours[0][ii].x == (c - 1)) ||
            (contours[0][ii].y == 0) || (contours[0][ii].y == r - 1))
        {
            contours[0].erase(contours[0].begin() + ii);
        }
    }

// Find equidistant points along the contour
    int N = 16; // N point DFT
// size_t dim1 = contours.size();
    size_t dim2 = contours[0].size(); // number of    contour points
    int shift = static_cast<int>(floor((dim2 / (N)) + 0.5));
    // delta between contour samples along the contour
    std::vector <Point> K; // vector to store the sample contour points
    // Find and store equidistant points along the contour
    for (int ii = 0; ii < N; ii++)
    { // Get N         samples         along        the contour
        if (shift * ii > dim2)
        {
            break;
        }
        else
        {
            K.push_back(contours[0][shift * ii]);
        }
    }
    std::vector<double> rt; // Euclidean distance vector
    for (int ii = 0; ii < K.size(); ii++)
    {
// find distance from each point to the hand center
        K[ii] -= ctrPt;
// compute Euclidean distance, r(t)
        rt.push_back(norm(K[ii]));
    }
// take the magnitude of the N point FFT
    Mat img = Mat(rt);
    Mat planes[] = {Mat_<double>(img), Mat::zeros(img.size(), CV_64F)};
    Mat complexI;
    merge(planes, 2, complexI);
    dft(complexI, complexI, DFT_COMPLEX_OUTPUT);
    split(complexI, planes); // planes[0] = Re(DFT(I), planes[1] = Im(DFT(I))
    magnitude(planes[0], planes[1], planes[0]); // planes[0] = magnitude
    Mat f_all = planes[0].t();
    // Normalize by the f0 (DC value)

    f_all /= f_all.at<double>(0);
    // Note: This depends on the data type of planes[](BOTHpartsof the constructor)
    // Only store the first (N/2) unique coefficient values (excludes the DC coeff, which
    //is always 1)
    // Ex: If N = 16, then keep 8 of the coeffs.
    int offset = static_cast<int>(floor((N / 2) + 0.5) + 1); // offset =
    round(N / 2) + 1;
    // Note: Adding 1 to get all unique coeffs
    Mat ft = f_all.colRange(Range(1, offset));
    return ft;
}

#endif //CHESS_AR_FOURIERDESCRIPTOR_H
