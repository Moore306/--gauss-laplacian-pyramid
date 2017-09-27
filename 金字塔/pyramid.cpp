//
//#include "opencv2/imgproc/imgproc.hpp"  
//#include "opencv2/highgui/highgui.hpp"  
//#include <math.h>  
//#include <stdlib.h>  
//#include <stdio.h>  
//
//using namespace cv;
//
////全局变量  
//Mat src, dst, tmp;
//char* window_name = "pyrDown and pyrUp Demo";
//
//int main(int argc, char** argv)
//{
//
//	//测试图像
//	src = imread("testImg.jpg");
//	if (!src.data)
//	{
//		printf(" No data! -- Exiting the program \n");
//		return -1;
//	}
//
//	tmp = src;
//	dst = tmp;
//
//	//创建显示窗口  
//	namedWindow(window_name, CV_WINDOW_AUTOSIZE);
//	imshow(window_name, dst);
//
//	//循环  
//	while (true)
//	{
//		int c;
//		c=1;
//		if (c == 1)
//		{
//			pyrUp(tmp, dst, Size(tmp.cols * 2, tmp.rows * 2));
//			printf("pryUp : Image x 2 \n");
//		}
//		else if (c == 2)
//		{
//			pyrDown(tmp, dst, Size(tmp.cols / 2, tmp.rows / 2));
//			printf("pyrDown : Image / 2 \n");
//		}
//
//		imshow(window_name, dst);
//		 imwrite( "pyrImage.jpg", dst );
//		tmp = dst;
//		break;
//	}
//	return 0;
//}
//
