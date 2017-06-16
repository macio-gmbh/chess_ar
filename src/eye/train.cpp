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

void train(std::string target_path, std::vector<std::pair<Mat, int>> &dict, int classId)
{

    boost::filesystem::directory_iterator end_itr; // Default ctor yields past-the-end
    for (boost::filesystem::directory_iterator i(target_path); i != end_itr; ++i)
    {
        //std::string file = "chessBoard/image" + std::to_string(i) + ".png";
        Mat matExC1 = imread((*i).path().string(), CV_8UC1);
        Mat edges;

        //imshow("matExC1", matExC1);

        if ((!matExC1.data))
        {
            continue;
        }

        matExC1 = EyeUtils::GetThresholdImage(matExC1);
        //imshow("test", matExC1);
        //waitKey(0);

        HOGDescriptor hog(
            Size(20, 20), //winSize
            Size(10, 10), //blocksize
            Size(5, 5), //blockStride,
            Size(10, 10), //cellSize,
            9, //nbins,
            1, //derivAper,
            -1, //winSigma,
            0, //histogramNormType,
            0.2, //L2HysThresh,
            1,//gammal correction,
            64,//nlevels=64
            1);//Use signed gradients
        vector<float> descriptors;
        hog.compute(matExC1, descriptors);

        Mat ft; // Fourier descriptor vector
        //int dist; // store the label of the minimum distance neighbor
//        ft = calcFourierDescriptor(matExC1); // compute the Fourier Descriptor for this image
//
//        if (ft.data)
//        {
//            std::pair<cv::Mat, int> fd;
//            fd.first = ft;
//            fd.second = classId;
//            dict.push_back(fd);
//        }
    }
}

void LoadImageDescriptors(std::string const &pathName, vector<Mat> &trainCells)
{
    // load all files from the given path
    boost::filesystem::directory_iterator end_itr; // Default ctor yields past-the-end
    for (boost::filesystem::directory_iterator i(pathName); i != end_itr; ++i)
    {
        // load it and create an edge image
        //Mat img = imread(pathName, CV_LOAD_IMAGE_GRAYSCALE);
        Mat matExC1 = imread((*i).path().string(), CV_8UC1);
        Mat edges;

        if ((!matExC1.data))
        {
            continue;
        }

        edges = EyeUtils::GetThresholdImage(matExC1);
        //imshow("test", edges);
        //waitKey(0);

        cv::Mat descriptor;
        descriptor = EyeUtils::GetDescriptor(edges);

        trainCells.push_back(descriptor);
    }
}

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

void CreateTrainTestHOG(vector<vector<float> > &trainHOG, vector<vector<float> > &testHOG,
                        vector<Mat> &deskewedtrainCells, vector<Mat> &deskewedtestCells)
{

    for (int y = 0; y < deskewedtrainCells.size(); y++)
    {
        vector<float> descriptors;
        hog.compute(deskewedtrainCells[y], descriptors);
        trainHOG.push_back(descriptors);
    }

    for (int y = 0; y < deskewedtestCells.size(); y++)
    {
        vector<float> descriptors;
        hog.compute(deskewedtestCells[y], descriptors);
        testHOG.push_back(descriptors);
    }
}

void ConvertVectortoMatrix(vector<vector<float> > &trainHOG, vector<vector<float> > &testHOG,
                           Mat &trainMat, Mat &testMat)
{

    int descriptor_size = trainHOG[0].size();

    for (int i = 0; i < trainHOG.size(); i++)
    {
        for (int j = 0; j < descriptor_size; j++)
        {
            trainMat.at<float>(i, j) = trainHOG[i][j];
        }
    }
    for (int i = 0; i < testHOG.size(); i++)
    {
        for (int j = 0; j < descriptor_size; j++)
        {
            testMat.at<float>(i, j) = testHOG[i][j];
        }
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
    //svm->setGamma(0.6);
    //svm->setC(10);
    svm->setKernel(SVM::LINEAR );
    svm->setType(SVM::C_SVC);
    Ptr<TrainData> td = TrainData::create(trainMat, ROW_SAMPLE, trainLabels);
    //svm->train(td);
    svm->trainAuto(td);
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
    vector<Mat> completeSet;
    vector<int> completeLabels;

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

    // split the data into training and test data
//    std::size_t const half_size = completeSet.size() / 2;
//    std::vector<Mat> trainingData(completeSet.begin(), completeSet.begin() + half_size);
//    std::vector<Mat> testData(completeSet.begin() + half_size, completeSet.end());
//
//    std::vector<int> trainingLabels(completeLabels.begin(), completeLabels.begin() + half_size);
//    std::vector<int> testLabels(completeLabels.begin() + half_size, completeLabels.end());


    // dont split
    std::vector<Mat> trainingData = completeSet;
    std::vector<Mat> testData = completeSet;

    std::vector<int> trainingLabels = completeLabels;
    std::vector<int> testLabels = completeLabels;

    // convert it into a single Mat
    Mat trainMat(completeSet.size(), completeSet[0].cols, CV_32FC1);
    Mat testMat(completeSet.size(), completeSet[0].cols, CV_32FC1);
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
        bishopDescritpors, knightDescritpors;

    //LoadImageDescriptors("chessBoard/empty/", emptyDescritpors);
    LoadImageDescriptors("chessBoard/king/", kingDescritpors);
    LoadImageDescriptors("chessBoard/queen/", queenDescritpors);
    LoadImageDescriptors("chessBoard/rook/", rookDescritpors);
    LoadImageDescriptors("chessBoard/bishop/", bishopDescritpors);
    LoadImageDescriptors("chessBoard/knight/", knightDescritpors);
    LoadImageDescriptors("chessBoard/pawn/", pawnDescritpors);

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

    return 0;

}

