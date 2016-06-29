// GaussianBlur.cpp : 定义控制台应用程序的入口点。
/// <summary>
/// 实现传统高斯(未经任何优化的高斯)，最新整理时间 2016.6.29。
/// 参考博客：http://www.cnblogs.com/tntmonks/p/5123854.html 
/// </summary>
/// <param name=" pixels">源图像数据在内存的起始地址。</param>
/// <param name="width">源和目标图像的宽度。</param>
/// <param name="height">源和目标图像的高度。</param>
/// <param name=" channels">通道数，灰度图像cn=1,彩色图像cn=3</param>
/// <param name="sigma">sigma的平方是高斯函数的方差</param>
/// <remarks> 1: 能处理8位灰度和24位图像。</remarks>
//  以下为参考函数实现的整个过程
//（1）建立工程，复制粘贴博客代码。
// (2) 添加malloc()和free()函数的头文件
// (3) exp()函数的头文件
// (4) 修改Gasussblur中形参int sigma为float sigma，更加符合实际情况
// (5) 配置OpenCV
// (6) 调用函数 
#include "stdafx.h"
#include<stdlib.h>  //malloc(),free()函数需要的头文件
#include<math.h>
#include <opencv2/opencv.hpp>
using namespace std;
using namespace cv;
inline int* buildGaussKern(int winSize, int sigma)
{
    int wincenter, x;
    float   sum = 0.0f;
    wincenter = winSize / 2;
    float *kern = (float*)malloc(winSize*sizeof(float));
    int *ikern = (int*)malloc(winSize*sizeof(int));
    float SQRT_2PI = 2.506628274631f;
    float sigmaMul2PI = 1.0f / (sigma * SQRT_2PI);
    float divSigmaPow2 = 1.0f / (2.0f * sigma * sigma);
    for (x = 0; x < wincenter + 1; x++)
    {
        kern[wincenter - x] = kern[wincenter + x] = exp(-(x * x)* divSigmaPow2) * sigmaMul2PI;
        sum += kern[wincenter - x] + ((x != 0) ? kern[wincenter + x] : 0.0);
    }
    sum = 1.0f / sum;
    for (x = 0; x < winSize; x++)
    {
        kern[x] *= sum;
        ikern[x] = kern[x] * 256.0f;
    }
    free(kern);
    return ikern;
}
void GaussBlur(unsigned char*  pixels, unsigned int    width, unsigned int  height, unsigned  int channels, float sigma)
{
    width = 3 * width;
    if ((width % 4) != 0) width += (4 - (width % 4));
 
    unsigned int  winsize = (1 + (((int)ceil(3 * sigma)) * 2));
    int *gaussKern = buildGaussKern(winsize, sigma);
    winsize *= 3;
    unsigned int  halfsize = winsize / 2;
       
    unsigned char *tmpBuffer = (unsigned char*)malloc(width * height* sizeof(unsigned char));
 
    for (unsigned int h = 0; h < height; h++)
    {
        unsigned int  rowWidth = h * width;
 
        for (unsigned int w = 0; w < width; w += channels)
        {
            unsigned int rowR = 0;
            unsigned int rowG = 0;
            unsigned int rowB = 0;
            int * gaussKernPtr = gaussKern;
            int whalfsize = w + width - halfsize;
            unsigned int  curPos = rowWidth + w;
            for (unsigned int k = 1; k < winsize; k += channels)
            {
                unsigned int  pos = rowWidth + ((k + whalfsize) % width);
                int fkern = *gaussKernPtr++;
                rowR += (pixels[pos] * fkern);
                rowG += (pixels[pos + 1] * fkern);
                rowB += (pixels[pos + 2] * fkern);
            }
 
            tmpBuffer[curPos] = ((unsigned char)(rowR >> 8));
            tmpBuffer[curPos + 1] = ((unsigned char)(rowG >> 8));
            tmpBuffer[curPos + 2] = ((unsigned char)(rowB >> 8));
 
        }
    }
    winsize /= 3;
    halfsize = winsize / 2;
    for (unsigned int w = 0; w < width; w++)
    {
        for (unsigned int h = 0; h < height; h++)
        {
            unsigned    int col_all = 0;
            int hhalfsize = h + height - halfsize;
            for (unsigned int k = 0; k < winsize; k++)
            {
                col_all += tmpBuffer[((k + hhalfsize) % height)* width + w] * gaussKern[k];
            }
            pixels[h * width + w] = (unsigned char)(col_all >> 8);
        }
    }
    free(tmpBuffer);
    free(gaussKern);
}

int _tmain(int argc, _TCHAR* argv[])
{
	
	 const char* imagename = "emosue.jpg";
    //从文件中读入图像
    Mat img = imread(imagename);
	Mat dst = imread(imagename);
    //如果读入图像失败
    if(img.empty())
    {
        fprintf(stderr, "Can not load image %s\n", imagename);
        return -1;
    }
	GaussBlur(img.data,img.cols,img.rows,3,3.33);
    //显示图像
	imshow("原图像",dst);
    imshow("模糊图像", img);
    //此函数等待按键，按键盘任意键就返回
    waitKey();
	return 0;
}

