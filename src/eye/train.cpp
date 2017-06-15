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

void loadTrainTestLabel(std::string const &pathName, int label, vector<Mat> &trainCells, vector<Mat> &testCells,
                        vector<int> &trainLabels, vector<int> &testLabels)
{

    // load all files from the given path
    boost::filesystem::directory_iterator end_itr; // Default ctor yields past-the-end
    int currentIndex = 0;
    for (boost::filesystem::directory_iterator i(pathName); i != end_itr; ++i)
    {
        // use every 3rd picture as test
        //bool useAsTest = (currentIndex % 3) == 0;

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
//
//        if (!useAsTest)
//        {
        trainCells.push_back(descriptor);
        trainLabels.push_back(label);
//        }
//        else
//        {
        testCells.push_back(descriptor);
        testLabels.push_back(label);
//        }
        currentIndex++;
        if (currentIndex > 4)
            break;
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

void SVMtrain(Mat &trainMat, vector<int> &trainLabels, Mat &testResponse, Mat &testMat)
{
    Ptr<SVM> svm = SVM::create();
    //svm->setGamma(0.6);
    //svm->setC(10);
    svm->setKernel(SVM::LINEAR);
    svm->setType(SVM::C_SVC);
    Ptr<TrainData> td = TrainData::create(trainMat, ROW_SAMPLE, trainLabels);
    //svm->train(td);
    svm->trainAuto(td);

    svm->save("model4.yml");

    svm->predict(testMat, testResponse);

    getSVMParams(svm);
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

void MergeMatrixVector(Mat &mat, vector<Mat> &vector)
{
    for (int i = 0; i < vector.size(); i++)
    {
        vector[i].copyTo(mat.row(i));
    }
}

int main()
{
    std::vector<std::pair<Mat, int>> dict;

    namespace fs = boost::filesystem;

    vector<Mat> trainCells;
    vector<Mat> testCells;
    vector<int> trainLabels;
    vector<int> testLabels;

    loadTrainTestLabel("chessBoard/king/", 1, trainCells, testCells, trainLabels, testLabels);
    loadTrainTestLabel("chessBoard/pawn/", 6, trainCells, testCells, trainLabels, testLabels);
    loadTrainTestLabel("chessBoard/bishop/", 4, trainCells, testCells, trainLabels, testLabels);
    loadTrainTestLabel("chessBoard/knight/", 5, trainCells, testCells, trainLabels, testLabels);
    loadTrainTestLabel("chessBoard/rook/", 3, trainCells, testCells, trainLabels, testLabels);
    loadTrainTestLabel("chessBoard/queen/", 2, trainCells, testCells, trainLabels, testLabels);
    loadTrainTestLabel("chessBoard/empty/", -1, trainCells, testCells, trainLabels, testLabels);

//    std::vector<std::vector<float> > trainHOG;
//    std::vector<std::vector<float> > testHOG;
//    CreateTrainTestHOG(trainHOG, testHOG, trainCells, testCells);
//
//    int descriptor_size = trainHOG[0].size();
//    cout << "Descriptor Size : " << descriptor_size << endl;
//
//    Mat trainMat(trainHOG.size(), descriptor_size, CV_32FC1);
//    Mat testMat(testHOG.size(), descriptor_size, CV_32FC1);
//
//    ConvertVectortoMatrix(trainHOG, testHOG, trainMat, testMat);

    Mat trainMat(trainCells.size(), trainCells[0].cols, CV_32FC1);
    Mat testMat(testCells.size(), testCells[0].cols, CV_32FC1);

    MergeMatrixVector(trainMat, trainCells);
    MergeMatrixVector(testMat, testCells);

    Mat testResponse;
    SVMtrain(trainMat, trainLabels, testResponse, testMat);

    float count = 0;
    float accuracy = 0;
    SVMevaluate(testResponse, count, accuracy, testLabels);

    cout << "the accuracy is :" << accuracy << endl;
    return 0;

}

