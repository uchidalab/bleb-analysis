#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/legacy/legacy.hpp >
#include "Labeling/Labeling.h"
#include <cstdlib>
#include <cmath>
#include <sstream>
#include <string>

#define LEN 70
#define RR 200	//�d�S����̍Œ��� *�v�m�F

#define PI 3.141592
#define HAL 180
#define CIR 360

using namespace std;
using namespace cv;

class BlebTrack {
public:
	float snake_alpha, snake_beta, snake_gamma;
	int out_length, nuc_length;
	int width, height;

public:
	BlebTrack::BlebTrack();		//Snake�ϐ��̏�����
	void SnakePre(cv::Mat& src_img, CvPoint *contour);	//snake�̏���
	cv::Mat GrayScale(cv::Mat& src_img);	//�O���[�X�P�[��
	cv::Mat ChooseCol(cv::Mat& src_img, int n); //�F���o
	cv::Mat NoiseReduction(cv::Mat& src_img);	//�m�C�Y����
	//cv::Mat threshold(cv::Mat& src_img);	//2�l��
	Mat FindMaxSpace(cv::Mat& src_img, int num);	//�ő�A���̈撊�o
	void Snake(cv::Mat& src_img, CvPoint *contour, int length);	//���I�֊s�@
	CvPoint CenterOfGravity(CvPoint *contour, int length);	//�֊s������d�S���Z�o
	//cv::Mat DPforOutline(CvPoint contours[CIR], cv::Mat& binary_img, Point2d Grav_point, double **distances, int frame_count);	//DP��p�����֊s���o
	int MinFromMask(int cost[HAL][RR][RR], int theta, int x, int y);	//�}�X�N������ŏ��l���擾
	void CurvatureMap(double **distances, int NumOfFrames);	//�ȗ��}�b�v�̐���		
	void DistanceMap(double **distances, int NumOfFrames);	//�����}�b�v�̐���
	Mat ContourExtraction(cv::Mat& binary_img, int NumOfRegions);	//�O�s���o
	Mat NewDP(vector<vector<CvPoint>> &contours, cv::Mat& src_img, Point Grav_point, vector<vector<double>> &distances, int frame_num, cv::Mat& row_img);
	CvPoint CenterOfGravityFromBinary(Mat& src_img);
};