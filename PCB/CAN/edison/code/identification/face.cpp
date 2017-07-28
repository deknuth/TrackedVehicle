#include "opencv2/core/core.hpp"
//#include <opencv2/imgproc/imgproc.hpp>
#include "opencv2/opencv.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/core/utility.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/objdetect/objdetect.hpp"


#include <iostream>
#include <stdio.h>

using namespace std;
using namespace cv;
typedef struct _IplImage
{
int nSize; /* IplImage大小 */
int ID; /* 版本 (=0)*/
int nChannels; /* 大多数OPENCV函数支持1,2,3 或 4 个通道 */
int alphaChannel; /* 被OpenCV忽略 */
int depth; /* 像素的位深度，主要有以下支持格式： IPL_DEPTH_8U, IPL_DEPTH_8S, IPL_DEPTH_16U,IPL_DEPTH_16S, IPL_DEPTH_32S,
IPL_DEPTH_32F 和IPL_DEPTH_64F */
char colorModel[4]; /* 被OpenCV忽略 */
char channelSeq[4]; /* 同上 */
int dataOrder; /* 0 - 交叉存取颜色通道, 1 - 分开的颜色通道.
cvCreateImage只可以创建交叉存取图像 */
int origin; /*图像原点位置： 0表示顶-左结构,1表示底-左结构 */
int align; /* 图像行排列方式 (4 or 8)，在 OpenCV 被忽略，使用 widthStep 代替 */
int width; /* 图像宽像素数 */
int height; /* 图像高像素数*/
struct _IplROI *roi; /* 图像感兴趣区域，当该值非空时，
只对该区域进行处理 */
struct _IplImage *maskROI; /* 在 OpenCV中必须为NULL */
void *imageId; /* 同上*/
struct _IplTileInfo *tileInfo; /*同上*/
int imageSize; /* 图像数据大小(在交叉存取格式下ImageSize=image->height*image->widthStep），单位字节*/
char *imageData; /* 指向排列的图像数据 */
int widthStep; /* 排列的图像行大小，以字节为单位 */
int BorderMode[4]; /* 边际结束模式, 在 OpenCV 被忽略*/
int BorderConst[4]; /* 同上 */
char *imageDataOrigin; /* 指针指向一个不同的图像数据结构（不是必须排列的），是为了纠正图像内存分配准备的 */
} IplImage;

String cascadeName = "haarcascades/haarcascade_frontalface_alt2.xml";

IplImage* cutImage(IplImage* src, CvRect rect) {
    cvSetImageROI(src, rect);
    // Mat(int _rows, int _cols, int _type, void* _data, size_t _step=AUTO_STEP);
    IplImage* dst = cvCreateImage(cvSize(rect.width, rect.height),
            src->depth,
            src->nChannels);

    cvCopy(src,dst,0);
    cvResetImageROI(src);
    return dst;
}

IplImage* detect( Mat& img, CascadeClassifier& cascade, double scale)
{
    int i = 0;
    double t = 0;
    vector<Rect> faces;
    Mat gray, smallImg( cvRound (img.rows/scale), cvRound(img.cols/scale), CV_8UC1 );

    cvtColor( img, gray, CV_BGR2GRAY );
    resize( gray, smallImg, smallImg.size(), 0, 0, INTER_LINEAR );
    equalizeHist( smallImg, smallImg );

    t = (double)cvGetTickCount();
    cascade.detectMultiScale( smallImg, faces,
        1.3, 2, CV_HAAR_SCALE_IMAGE,
        Size(30, 30) );
    t = (double)cvGetTickCount() - t;
    printf( "detection time = %g ms\n", t/((double)cvGetTickFrequency()*1000.) );
    for( vector<Rect>::const_iterator r = faces.begin(); r != faces.end(); r++, i++ )
    {
        IplImage* temp = cutImage(&(IplImage(img)), cvRect(r->x, r->y, r->width, r->height));
        return temp;
    }

    return NULL;
}
//画直方图用
int HistogramBins = 256;
float HistogramRange1[2]={0,255};
float *HistogramRange[1]={&HistogramRange1[0]};
int CompareHist(IplImage* image1, IplImage* image2)
{
    IplImage* srcImage;
    IplImage* targetImage;
    if (image1->nChannels != 1) {
        srcImage = cvCreateImage(cvSize(image1->width, image1->height), image1->depth, 1);
        cvCvtColor(image1, srcImage, CV_BGR2GRAY);
    } else {
        srcImage = image1;
    }

    if (image2->nChannels != 1) {
        targetImage = cvCreateImage(cvSize(image2->width, image2->height), srcImage->depth, 1);
        cvCvtColor(image2, targetImage, CV_BGR2GRAY);
    } else {
        targetImage = image2;
    }

    CvHistogram *Histogram1 = cvCreateHist(1, &HistogramBins, CV_HIST_ARRAY,HistogramRange);
    CvHistogram *Histogram2 = cvCreateHist(1, &HistogramBins, CV_HIST_ARRAY,HistogramRange);

    cvCalcHist(&srcImage, Histogram1);
    cvCalcHist(&targetImage, Histogram2);

    cvNormalizeHist(Histogram1, 1);
    cvNormalizeHist(Histogram2, 1);

    // CV_COMP_CHISQR,CV_COMP_BHATTACHARYYA这两种都可以用来做直方图的比较，值越小，说明图形越相似
    printf("CV_COMP_CHISQR : %.4f\n", cvCompareHist(Histogram1, Histogram2, CV_COMP_CHISQR));
    printf("CV_COMP_BHATTACHARYYA : %.4f\n", cvCompareHist(Histogram1, Histogram2, CV_COMP_BHATTACHARYYA));


    // CV_COMP_CORREL, CV_COMP_INTERSECT这两种直方图的比较，值越大，说明图形越相似
    printf("CV_COMP_CORREL : %.4f\n", cvCompareHist(Histogram1, Histogram2, CV_COMP_CORREL));
    printf("CV_COMP_INTERSECT : %.4f\n", cvCompareHist(Histogram1, Histogram2, CV_COMP_INTERSECT));

    cvReleaseHist(&Histogram1);
    cvReleaseHist(&Histogram2);
    if (image1->nChannels != 1) {
        cvReleaseImage(&srcImage);
    }
    if (image2->nChannels != 1) {
        cvReleaseImage(&targetImage);
    }
    return 0;
}
String srcImage = "image.jpg";
String targetImage = "t-image.jpg";
int main(int argc, char* argv[])
{
    CascadeClassifier cascade;
//    namedWindow("image1");
//    namedWindow("image2");
    if( !cascade.load( cascadeName ) )
    {
        return -1;
    }

    Mat srcImg, targetImg;
    IplImage* faceImage1;
    IplImage* faceImage2;
    srcImg = imread(srcImage);
    targetImg = imread(targetImage);
    faceImage1 = detect(srcImg, cascade, 1);
    if (faceImage1 == NULL) {
        return -1;
    }
//    cvSaveImage("d:\\face.jpg", faceImage1, 0);
    faceImage2 = detect(targetImg, cascade, 1);
    if (faceImage2 == NULL) {
        return -1;
    }
//    cvSaveImage("d:\\face1.jpg", faceImage2, 0);
//    imshow("image1", Mat(faceImage1));
//    imshow("image2", Mat(faceImage2));
    imwrite( "over.jpg", faceImage2);
    CompareHist(faceImage1, faceImage2);
    cvWaitKey(0);
    cvReleaseImage(&faceImage1);
    cvReleaseImage(&faceImage2);
    return 0;
}
