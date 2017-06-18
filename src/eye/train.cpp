#include <iostream>
#include <fstream>
#include <ctime>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include "opencv2/objdetect.hpp"
#include <opencv2/ml/ml.hpp>

#include <boost/serialization/vector.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>

#include "EyeUtils.h"

using namespace std;
using namespace cv;
using namespace cv::ml;
namespace fs = boost::filesystem;

void LoadImageDescriptors(std::string const &pathName, vector<Mat> &trainCells)
{
    // load all files from the given path
    boost::filesystem::directory_iterator end_itr; // Default ctor yields past-the-end
    for (boost::filesystem::directory_iterator i(pathName); i != end_itr; ++i)
    {
        // load it and create an edge image
        //Mat img = imread(pathName, CV_LOAD_IMAGE_GRAYSCALE);
        Mat image = imread((*i).path().string(), CV_8UC1);
        Mat edges;

        if ((!image.data))
        {
            continue;
        }

        //image = EyeUtils::GetThresholdImage(image);
        //imshow("test", edges);
        //waitKey(0);

        cv::Mat descriptor = EyeUtils::GetDescriptor(image);

        trainCells.push_back(descriptor);
    }
}

void LoadImageColors(std::string const &pathName, vector<Mat> &trainCells)
{
    // load all files from the given path
    boost::filesystem::directory_iterator end_itr; // Default ctor yields past-the-end
    for (boost::filesystem::directory_iterator i(pathName); i != end_itr; ++i)
    {
        // load it and create an edge image
        //Mat img = imread(pathName, CV_LOAD_IMAGE_GRAYSCALE);
        Mat src = imread((*i).path().string(), CV_8UC1);

        if ((!src.data))
        {
            continue;
        }

        //src = EyeUtils::GetThresholdImage(src);
        Mat descriptor = EyeUtils::GetColorDescriptor(src);

        //descriptor = EyeUtils::GetDescriptor(edges);
        trainCells.push_back(descriptor);
    }
}

void getSVMParams(SVM *svm)
{
    cout << "Kernel type     : " << svm->getKernelType() << endl;
    cout << "Type            : " << svm->getType() << endl;
    cout << "C               : " << svm->getC() << endl;
    cout << "Degree          : " << svm->getDegree() << endl;
    cout << "Nu              : " << svm->getNu() << endl;
    cout << "Gamma           : " << svm->getGamma() << endl;
}

void SVMtrain(Mat &trainMat, vector<int> &trainLabels, Mat &testResponse, Mat &testMat, std::string const &savePath)
{
    Ptr<SVM> svm = SVM::create();
    svm->setGamma(0.9);
    svm->setC(10);
    svm->setKernel(SVM::    LINEAR );
    svm->setType(SVM::C_SVC);
    Ptr<TrainData> td = TrainData::create(trainMat, ROW_SAMPLE, trainLabels);
    svm->train(td);
    //svm->trainAuto(td);
    svm->save(savePath);
    svm->predict(testMat, testResponse);

    //getSVMParams(svm);
}

void SVMevaluate(Mat &testResponse, float &count, float &accuracy, vector<int> &testLabels)
{
    for (int i = 0; i < testResponse.rows; i++)
    {
        //cout << testResponse.at<float>(i,0) << " " << testLabels[i] << endl;
        if (testResponse.at<float>(i, 0) == testLabels[i])
        {
            count = count + 1;
        }
    }
    accuracy = (count / testResponse.rows) * 100;
}

void CreateSVM(std::string const &savePath, std::vector<Mat> const &firstClass,
               std::initializer_list<std::vector<Mat>> const &otherClasses)
{
    Mat testResponse;
    std::vector<Mat> completeSet;
    std::vector<int> completeLabels;

    for (auto currMat: firstClass)
    {
        completeSet.push_back(currMat);
        completeLabels.push_back(1);
    }
    for (auto elem : otherClasses)
    {
        for (auto currMat: elem)
        {
            completeSet.push_back(currMat);
            completeLabels.push_back(0);
        }
    }

    //split the data into training and test data
//    std::size_t const half_size = completeSet.size() / 2;
//    std::vector<Mat> trainingData(completeSet.begin(), completeSet.begin() + half_size);
//    std::vector<Mat> testData(completeSet.begin() + half_size, completeSet.end());
//
//    std::vector<int> trainingLabels(completeLabels.begin(), completeLabels.begin() + half_size);
//    std::vector<int> testLabels(completeLabels.begin() + half_size, completeLabels.end());
//

    // dont split
    std::vector<Mat> trainingData = completeSet;
    std::vector<Mat> testData = completeSet;

    std::vector<int> trainingLabels = completeLabels;
    std::vector<int> testLabels = completeLabels;

    // convert it into a single Mat
    int colSize = completeSet[0].cols;
    Mat trainMat(completeSet.size(), colSize, CV_32FC1);
    Mat testMat(completeSet.size(), colSize, CV_32FC1);
    EyeUtils::MergeMatrixVector(trainMat, trainingData);
    EyeUtils::MergeMatrixVector(testMat, testData);

    SVMtrain(trainMat, completeLabels, testResponse, testMat, savePath);

    float count = 0;
    float accuracy = 0;
    SVMevaluate(testResponse, count, accuracy, testLabels);
    cout << savePath << ": the accuracy is :" << accuracy << endl;
}

int main()
{
    vector<Mat> kingDescritpors, pawnDescritpors, emptyDescritpors, queenDescritpors, rookDescritpors,
        bishopDescritpors, knightDescritpors, blackDescriptors, whiteDescriptors;

    LoadImageDescriptors("../train/chessBoard/empty/", emptyDescritpors);
    LoadImageDescriptors("../train/chessBoard/king/", kingDescritpors);
    LoadImageDescriptors("../train/chessBoard/queen/", queenDescritpors);
    LoadImageDescriptors("../train/chessBoard/rook/", rookDescritpors);
    LoadImageDescriptors("../train/chessBoard/bishop/", bishopDescritpors);
    LoadImageDescriptors("../train/chessBoard/knight/", knightDescritpors);
    LoadImageDescriptors("../train/chessBoard/pawn/", pawnDescritpors);

    LoadImageColors("../train/chessBoard/black/", blackDescriptors);
    LoadImageColors("../train/chessBoard/white/", whiteDescriptors);

    // one vs Rest
    CreateSVM("king.yml", kingDescritpors, {queenDescritpors, rookDescritpors, bishopDescritpors,
                                            knightDescritpors, pawnDescritpors, emptyDescritpors});
    CreateSVM("queen.yml", queenDescritpors, {kingDescritpors, rookDescritpors, bishopDescritpors,
                                              knightDescritpors, pawnDescritpors, emptyDescritpors});
    CreateSVM("rook.yml", rookDescritpors, {kingDescritpors, queenDescritpors, bishopDescritpors,
                                            knightDescritpors, pawnDescritpors, emptyDescritpors});
    CreateSVM("bishop.yml", bishopDescritpors, {kingDescritpors, queenDescritpors, rookDescritpors,
                                                knightDescritpors, pawnDescritpors, emptyDescritpors});
    CreateSVM("knight.yml", knightDescritpors, {kingDescritpors, queenDescritpors, rookDescritpors, bishopDescritpors,
                                                pawnDescritpors, emptyDescritpors});
    CreateSVM("pawn.yml", pawnDescritpors, {kingDescritpors, queenDescritpors, rookDescritpors, bishopDescritpors,
                                            knightDescritpors, emptyDescritpors});

    CreateSVM("black.yml", blackDescriptors, {whiteDescriptors});

    return 0;

}

