#include <iostream>
#include <unistd.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;
using namespace std;

const bool USE_STATIC_IMAGE = true;

int main() {

    VideoCapture capture(0);
    Mat cameraFrame;
    bool staticImage = false;

    // open the camera
    if (USE_STATIC_IMAGE || !capture.isOpened()) {
        cout << "cannot open camera, using the static image \n";
        staticImage = true;
        cameraFrame = imread("chess.png", 1);

        if ( !cameraFrame.data )
        {
            printf("No image data \n");
            return -1;
        }
        //return 0;
    }

    // main loop
    while(true)
    {
        if (!staticImage)
            capture.read(cameraFrame);

        // make a gray image
        // find the quads
        // find the figures
        // recreate the chessboard
        // send the chessboard via rabbitMQ

        // show the input image in a window
        imshow("cam", cameraFrame);
        if (waitKey(30) == 27)
            break;

    }
    return 0;
}


