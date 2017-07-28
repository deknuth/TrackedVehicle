#include "opencv2/core/utility.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include <stdio.h>
using namespace cv;
using namespace std;

int main(void)
{
    // Build video capturer
 //   VideoCapture cap("/dev/video1"); // read from file
    cv::VideoCapture cap(1);
    if(!cap.isOpened()){
        printf("Cannot open video/camera!\n" );
        return -1;
    }
    // Read frames
    Mat frame;
    // read frame and check
    cap >> frame;
    if(frame.empty())
        printf("empty!\n");
    else
        imwrite("get.bmp", frame);
}
