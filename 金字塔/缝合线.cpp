// SurfTest.cpp : 定义控制台应用程序的入口点。
//


#include <opencv2/opencv.hpp>
#include <string.h>
#include <atlstr.h>
#include <assert.h>
#include <math.h>
#include <float.h>
#include <limits.h>
#include <time.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <cstring>
//#include <opencv2/stitching.hpp>
#include <iostream>
#include <fstream>

using namespace cv;
using namespace std;




//int _tmain(int argc, _TCHAR* argv[])
//{
//   
//    CString imgpath="C:\\Users\\Administrator.UNFCVZJ5EXJ1VMM\\Desktop\\";
//    CString imgname="l.jpg";
//    CString filepath;
//    filepath=imgpath+imgname;
//    IplImage *testimg=cvLoadImage(filepath,-1);
//    CString savepath="C:\\Users\\Administrator.UNFCVZJ5EXJ1VMM\\Desktop\\";
//    for (int i=0;i<8;i++)
//    {
//        for (int j=0;j<7;j++)
//        {
//            CString saveimgpath;
//            CString saveimgname;
//            saveimgname.Format("0-%d-%d.jpg",i,j);
//            saveimgpath=savepath+saveimgname;
//            cvSaveImage(saveimgpath,testimg);
//        }
//    }
//    cvReleaseImage(&testimg);
//    printf("success");
//    system("pause");
//}

/************************************************************************/
/* 说明：
*金字塔从下到上依次为 [0,1，...，level-1] 层
*blendMask 为图像的掩模
*maskGaussianPyramid为金字塔每一层的掩模
*resultLapPyr 存放每层金字塔中直接用左右两图Laplacian变换拼成的图像
*/
/************************************************************************/


class LaplacianBlending {
private:
	Mat_<Vec3f> left;
	Mat_<Vec3f> right;
	Mat_<float> blendMask;

	vector<Mat_<Vec3f> > leftLapPyr,rightLapPyr,resultLapPyr;//Laplacian Pyramids
	Mat leftHighestLevel, rightHighestLevel, resultHighestLevel;
	vector<Mat_<Vec3f> > maskGaussianPyramid; //masks are 3-channels for easier multiplication with RGB

	int levels;

	void buildPyramids() 
	{
		buildLaplacianPyramid(left,leftLapPyr,leftHighestLevel);
		buildLaplacianPyramid(right,rightLapPyr,rightHighestLevel);
		buildGaussianPyramid();
	}

	void buildGaussianPyramid() 
	{//金字塔内容为每一层的掩模
		assert(leftLapPyr.size()>0);

		maskGaussianPyramid.clear();
		Mat currentImg;
		cvtColor(blendMask, currentImg, CV_GRAY2BGR);//store color img of blend mask into maskGaussianPyramid
		maskGaussianPyramid.push_back(currentImg); //0-level

		currentImg = blendMask;
		for (int l=1; l<levels+1; l++) {
			Mat _down;
			if (leftLapPyr.size() > l)
				pyrDown(currentImg, _down, leftLapPyr[l].size());
			else
				pyrDown(currentImg, _down, leftHighestLevel.size()); //lowest level

			Mat down;
			cvtColor(_down, down, CV_GRAY2BGR);
			maskGaussianPyramid.push_back(down);//add color blend mask into mask Pyramid
			currentImg = _down;
		}
	}

	void buildLaplacianPyramid(const Mat& img, vector<Mat_<Vec3f> >& lapPyr, Mat& HighestLevel)
	{
		lapPyr.clear();
		Mat currentImg = img;
		for (int l=0; l<levels; l++) 
		{
			Mat down,up;
			pyrDown(currentImg, down);
			pyrUp(down, up,currentImg.size());
			Mat lap = currentImg - up;
			lapPyr.push_back(lap);
			currentImg = down;
		}
		currentImg.copyTo(HighestLevel);
	}

	Mat_<Vec3f> reconstructImgFromLapPyramid() 
	{
		//将左右laplacian图像拼成的resultLapPyr金字塔中每一层
		//从上到下插值放大并相加，即得blend图像结果
		Mat currentImg = resultHighestLevel;
		for (int l=levels-1; l>=0; l--) 
		{
			Mat up;

			pyrUp(currentImg, up, resultLapPyr[l].size());
			currentImg = up + resultLapPyr[l];
		}
		return currentImg;
	}

	void blendLapPyrs() 
	{
		//获得每层金字塔中直接用左右两图Laplacian变换拼成的图像resultLapPyr
		resultHighestLevel = leftHighestLevel.mul(maskGaussianPyramid.back()) +
			rightHighestLevel.mul(Scalar(1.0,1.0,1.0) - maskGaussianPyramid.back());
		for (int l=0; l<levels; l++) 
		{
			Mat A = leftLapPyr[l].mul(maskGaussianPyramid[l]);
			Mat antiMask = Scalar(1.0,1.0,1.0) - maskGaussianPyramid[l];
			Mat B = rightLapPyr[l].mul(antiMask);
			Mat_<Vec3f> blendedLevel = A + B;

			resultLapPyr.push_back(blendedLevel);
		}
	}

public:
	LaplacianBlending(const Mat_<Vec3f>& _left, const Mat_<Vec3f>& _right, const Mat_<float>& _blendMask, int _levels)://construct function, used in LaplacianBlending lb(l,r,m,4);
	  left(_left),right(_right),blendMask(_blendMask),levels(_levels)
	  {
		  assert(_left.size() == _right.size());
		  assert(_left.size() == _blendMask.size());
		  buildPyramids();    //construct Laplacian Pyramid and Gaussian Pyramid
		  blendLapPyrs();    //blend left & right Pyramids into one Pyramid
	  };

	  Mat_<Vec3f> blend() {
		  return reconstructImgFromLapPyramid();//reconstruct Image from Laplacian Pyramid
	  }
};

Mat_<Vec3f> LaplacianBlend(const Mat_<Vec3f>& l, const Mat_<Vec3f>& r, const Mat_<float>& m) {
	LaplacianBlending lb(l,r,m,30);
	return lb.blend();
}

int main() 
{


	Mat l8u = imread("left.jpg");//左图
	Mat r8u = imread("right.jpg");//右图

	namedWindow("left",0);
	imshow("left",l8u); 

	namedWindow("right",0);
	imshow("right",r8u);

	Mat_<Vec3f> l; l8u.convertTo(l,CV_32F,1.0/255.0);//Vec3f表示有三个通道，即 l[row][column][depth]
	Mat_<Vec3f> r; r8u.convertTo(r,CV_32F,1.0/255.0);


	////create blend mask matrix m
	//Mat_<float> m(l.rows,l.cols,0.0);                    //将m全部赋值为0
	//m(Range::all(),Range(0,m.cols/2)) = 1.0;    //取m全部行&[0,m.cols/2]列，赋值为1.0


	Mat_<float> m(l.rows,l.cols,0.0);
	Mat C=imread("mask.jpg"); //模板
	for(int i=0;i<l.rows;i++)
	{
		for(int j=0;j<l.cols;j++)
		{
			if(C.at<Vec3b>(i,j)[0]!=0&&C.at<Vec3b>(i,j)[1]!=0&&C.at<Vec3b>(i,j)[2]!=0)  // 因为我要的只是位置
				m(i,j)=1.0;
		}
	}


	Mat_<Vec3f> blend = LaplacianBlend(l, r, m);

	Mat re;  
	blend.convertTo(re,CV_8UC3,255);  
	imwrite("blended.jpg",re); 

	namedWindow("blended",0);
	imshow("blended",blend);

	waitKey(0);
}