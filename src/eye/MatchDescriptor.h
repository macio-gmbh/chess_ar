//
// Created by marco on 28.05.2017.
// Code take from https://github.com/LaurentBerger/FourierDescriptors
//

#ifndef CHESS_AR_MATCHDESCRIPTOR_H
#define CHESS_AR_MATCHDESCRIPTOR_H

#include <opencv2/opencv.hpp>
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <iostream>
#include <fstream>

using namespace cv;
using namespace std;

class MatchDescriptor
{
 public :
  std::vector<Point2d> sContour;
  std::vector<complex<float>> b;
  std::vector<complex<float>> a;
  std::vector<float> frequence;
  std::vector<float> rho, psi;
  double pi;
  int nbDesFit;

  MatchDescriptor();
  float AjustementRtSafe(vector<Point2d> &c, float &alphaMin, float &phiMin, float &sMin);
  float Ajustement(vector<Point2d> &c, float &alphaMin, float &phiMin, float &sMin);
  void falpha(float x, float *fn, float *df);
  void InitFrequence();
  float rtsafe(float x1, float x2, float xacc);
  float Distance(complex<float> r, float alpha);
  static Point2d Echantillon(vector<Point> &c, long i, float l1, float l2, float s);
  static vector<Point2d> ReSampleContour(vector<Point> &c, int nbElt);

};

#endif //CHESS_AR_MATCHDESCRIPTOR_H
