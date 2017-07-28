#include "opencv2/core/core.hpp"
//#include <opencv2/imgproc/imgproc.hpp>
#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/core/utility.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"

#include <vector>
#include <cstdio>

using namespace std;
using namespace cv;

int FisherFaceTrain(Mat img)
{
    vector<Mat> images;
    vector<int> labels;
    int predicted;
    // images for first person
    images.push_back(imread("face/1.jpg", 0));      // 0 灰度图
    labels.push_back(0);
    images.push_back(imread("face/2.jpg", 0));
    labels.push_back(0);
    images.push_back(imread("face/3.jpg", 0));
    labels.push_back(0);
    images.push_back(imread("face/4.jpg", 0));
    labels.push_back(0);
    images.push_back(imread("face/5.jpg", 0));
    labels.push_back(0);
    images.push_back(imread("face/6.jpg", 0));
    labels.push_back(0);
    /*
    // images for second person
    images.push_back(imread("person1/0.jpg", CV_LOAD_IMAGE_GRAYSCALE));
    labels.push_back(1);
    images.push_back(imread("person1/1.jpg", CV_LOAD_IMAGE_GRAYSCALE));
    labels.push_back(1);
*/
    Ptr<FaceRecognizer> model = createFisherFaceRecognizer();
    model->train(images, labels);

 //   Mat img = imread("person1/2.jpg", CV_LOAD_IMAGE_GRAYSCALE);
    predicted = model->predict(img);
    return predicted;
}


int main(void)
{
    // 【1】加载分类器
    CascadeClassifier cascade;
    cascade.load("haarcascades/haarcascade_frontalface_alt2.xml");

    Mat srcImage, grayImage,dstImage;
    // 【2】读取图片
    srcImage = imread("image.jpg");
    dstImage = srcImage.clone();
//    imshow("【原图】", srcImage);

    grayImage.create(srcImage.size(), srcImage.type());
    cvtColor(srcImage, grayImage, COLOR_BGR2GRAY); // 生成灰度图，提高检测效率
    imwrite( "gray.jpg", grayImage);
    // 定义7种颜色，用于标记人脸
    Scalar colors[] =
    {
        // 红橙黄绿青蓝紫
        {255, 0, 0},
        {255, 97, 0},
        {255, 255, 0},
        {0, 255, 0},
        {0, 255, 255},
        {0, 0, 255},
        {160, 32, 240},
    };

    // 【3】检测
    vector<Rect> faces;
    double t = 0;
    t = (double)getTickCount();
    cascade.detectMultiScale(grayImage, faces, 1.1, 3, 0);  // 分类器对象调用
//    cascade.detectMultiScale(grayImage, rect,1.1, 2, 0,Size(20, 20));
//    detectMultiScale( smallImg, faces,1.3, 2, CV_HAAR_SCALE_IMAGE,Size(30, 30) );
    printf("face number: %d\n", faces.size());
    //切图
    /*
    IplImage *img = cvLoadImage("image.jpg", 0);
    CvRect roi = cvRect(1, 1, 50, 50);
    IplImage *newImg = cvCreateImage(cvSize(roi.width, roi.height),img->depth, img->nChannels);
    cvSetImageROI(img, roi);
    cvCopy(newImg, img);
    cvResetImageROI(img);
*/
    Mat fisherImg;
    Mat srcCuteimg = srcImage(Rect(faces[0].x,faces[0].y,faces[0].width,faces[0].height));
    cvtColor(srcCuteimg, fisherImg, COLOR_BGR2GRAY);
    printf("predicted: %d\n",FisherFaceTrain(fisherImg));
    imwrite( "roi.jpg", fisherImg);

    t = (double)getTickCount() - t;
    printf("execution time = %gms\n", t*1000. / getTickFrequency());
    // 【4】标记--在脸部画圆
    for (int i = 0; i < faces.size();i++)
    {
        Point  center;
        int radius;
        center.x = cvRound((faces[i].x + faces[i].width * 0.5));
        center.y = cvRound((faces[i].y + faces[i].height * 0.5));

        radius = cvRound((faces[i].width + faces[i].height) * 0.25);
        circle(dstImage, center, radius, colors[i % 7], 2);
        printf("srcCuteimg w:%d  h:%d\n",faces[i].width,faces[i].height);
    }

    // 【5】显示
 //   imshow("【人脸识别detectMultiScale】", dstImage);
     imwrite( "over.jpg", dstImage);
   // waitKey(0);
    return 0;
}
