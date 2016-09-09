// GaussianBlur.cpp : �������̨Ӧ�ó������ڵ㡣
/// <summary>
/// ʵ�ִ�ͳ��˹(δ���κ��Ż��ĸ�˹)����������ʱ�� 2016.6.29��
/// �ο����ͣ�http://www.cnblogs.com/tntmonks/p/5123854.html 
/// </summary>
/// <param name=" pixels">Դͼ���������ڴ����ʼ��ַ��</param>
/// <param name="width">Դ��Ŀ��ͼ��Ŀ��ȡ�</param>
/// <param name="height">Դ��Ŀ��ͼ��ĸ߶ȡ�</param>
/// <param name=" channels">ͨ�������Ҷ�ͼ��cn=1,��ɫͼ��cn=3</param>
/// <param name="sigma">sigma��ƽ���Ǹ�˹�����ķ���</param>
/// <remarks> 1: �ܴ���8λ�ҶȺ�24λͼ����Ҫ�ֿ����У������ϳ�һ������</remarks>
//  ����Ϊ�ο�����ʵ�ֵ���������
//��1���������̣�����ճ�����ʹ��롣
// (2) ����malloc()��free()������ͷ�ļ�
// (3) exp()������ͷ�ļ�
// (4) �޸�Gasussblur���β�int sigmaΪfloat sigma�����ӷ���ʵ�����
// (5) ����OpenCV
// (6) ���ú��� 
#include "stdafx.h"
#include<stdlib.h>  //malloc(),free()������Ҫ��ͷ�ļ�
#include<math.h>
#include<windows.h>  //����ʱ��ͷ�ļ�
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
void GaussBlur1D(unsigned char*  pixels,unsigned char*  pixelsout, unsigned int  width, unsigned int  height, float sigma)  //ɾ��unsigned  int channels,��Ϊ�ǵ�ͨ��û����
{
    width = 1 * width;  //3�޸�Ϊ1����Ϊ����ͨ����Ϊ��1��ͨ�����洢ÿ�����ݵĿ��ȱ�Ϊ��ԭ����1/3.
    if ((width % 4) != 0) width += (4 - (width % 4));
 
    unsigned int  winsize = (1 + (((int)ceil(3 * sigma)) * 2));  //���Ĵ�С
    int *gaussKern = buildGaussKern(winsize, sigma); //������˹�ˣ������˹ϵ��
    winsize *= 1; //3��Ϊ1����˹���Ŀ��ȱ�Ϊԭ����1/3
    unsigned int  halfsize = winsize / 2;  //���ıߵ����ĵľ���
       
    unsigned char *tmpBuffer = (unsigned char*)malloc(width * height* sizeof(unsigned char));  //�����µ��ڴ�洢������˹ģ���������
 
    for (unsigned int h = 0; h < height; h++)    //���ѭ����ͼ��ĸ߶�
    {
        unsigned int  rowWidth = h * width;     //��ǰ�еĿ���Ϊͼ��ĸ߶ȳ���ÿ��ͼ���������ռ�Ŀ��ȡ���Ϊ�ǰ��д洢�����顣
 
        for (unsigned int w = 0; w < width; w++) //w+=channels�������޸�Ϊw++����Ϊ�ǵ�ͨ�����ݣ���������ͨ������
        {
            unsigned int rowR = 0;  //�洢r����������
            int * gaussKernPtr = gaussKern;//����˹ϵ����ֵ��gaussKernPtr
            int whalfsize = w + width - halfsize;
            unsigned int  curPos = rowWidth + w;  //��ǰλ��
            for (unsigned int k = 1; k < winsize;k++) // k += channels�޸�Ϊk++
            {
                unsigned int  pos = rowWidth + ((k + whalfsize) % width);
                int fkern = *gaussKernPtr++;
                rowR += (pixels[pos] * fkern);  //��ǰ����ֵ���Ը�˹ϵ����rowR���˷�ָ��ͨ���ĵ�ǰ���ص��˹���������  
            }
 
            tmpBuffer[curPos] = ((unsigned char)(rowR >> 8)); //����256
 
        }
    }
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
            pixelsout[h * width + w] = (unsigned char)(col_all >> 8);
        }
    }
    free(tmpBuffer);
    free(gaussKern);
}

int _tmain(int argc, _TCHAR* argv[])
{
	
	 const char* imagename = "C:\\Users\\Administrator.IES7LSEJAZ1GGRL\\Desktop\\PureGaussian-master\\GaussianBlur\\GaussianBlur\\InputName.bmp";
    //���ļ��ж���ͼ��
    Mat img = imread(imagename);
	Mat dst = imread(imagename);
	Mat gray_img;
	Mat gray_dst;
	cvtColor(img, gray_img, CV_BGR2GRAY);
	cvtColor(dst, gray_dst, CV_BGR2GRAY);
    //�������ͼ��ʧ��
    if(img.empty())
    {
        fprintf(stderr, "Can not load image %s\n", imagename);
        return -1;
    }
	LARGE_INTEGER m_nFreq;
    LARGE_INTEGER m_nBeginTime;
    LARGE_INTEGER nEndTime;
    QueryPerformanceFrequency(&m_nFreq); // ��ȡʱ������
    QueryPerformanceCounter(&m_nBeginTime); // ��ȡʱ�Ӽ���
	  GaussBlur1D(gray_img.data,gray_dst.data,gray_img.cols,gray_img.rows,2);
	 QueryPerformanceCounter(&nEndTime);
     cout << (nEndTime.QuadPart-m_nBeginTime.QuadPart)*100/m_nFreq.QuadPart << endl;
    //��ʾͼ��
	imshow("ԭͼ��",gray_img);
    imshow("ģ��ͼ��", gray_dst);
    //�˺����ȴ�������������������ͷ���
    waitKey();
	return 0;
}
