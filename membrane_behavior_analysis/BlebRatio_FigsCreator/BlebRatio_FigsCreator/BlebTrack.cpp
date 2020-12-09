#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/legacy/legacy.hpp >
#include <opencv2/contrib/contrib.hpp>
#include <omp.h>

#include "BlebTrack.h"

#define LEN 70
#define RR 200	//�d�S����̍Œ��� *�v�m�F

#define MSK 32 //27
#define PI 3.1415926535897932384
#define HAL 180
#define CIR 360

#define INTERVAL_ANGLE_FOR_CURVATURE 4

using namespace std;
using namespace cv;


//+++++++++++++++++++++++++++++++++++++++++++++++++
//DP��p�����֊s���o
//+++++++++++++++++++++++++++++++++++++++++++++++++
Mat BlebTrack::NewDP(vector<vector<CvPoint>> &contours, cv::Mat& src_img, Point Grav_point, vector<vector<double>> &distances, int frame_num, cv::Mat& row_img)
{
	//���v���V�A���t�B���^�ɂ��֊s���o(�A���̈悲��)
	Laplacian(src_img, src_img, CV_32F, 3);
	convertScaleAbs(src_img, src_img);


	//�ő�̈撊�o
	src_img = FindMaxSpace(src_img, 0);

	Mat temp_img = src_img.clone();
	Mat contour = temp_img.clone();
	cvtColor(temp_img, temp_img, CV_GRAY2RGB);
	circle(temp_img, Grav_point, 2, cv::Scalar(255,0,0), -1, CV_AA);	//�C�΁C��
	imwrite("../workspace/RealContour/" +to_string(frame_num+1)+".tif", temp_img);



	//�f�B�X�^���X�摜����
	Mat distance_img;
	distanceTransform(src_img, distance_img, CV_DIST_L2, 3);
	normalize(distance_img, distance_img, 0.0, 255.0, CV_MINMAX, NULL);

	temp_img = distance_img.clone();
	cvtColor(temp_img, temp_img, CV_GRAY2RGB);
	circle(temp_img, Grav_point, 2, cv::Scalar(255, 0, 0), -1, CV_AA);	//�C�΁C��
	imwrite("../distance.tif", temp_img);

	int table[CIR][RR][RR];

	int theta_p[HAL + 1][RR], theta_n[HAL + 1][RR];
	int x, y;

	//positive
	for (int theta = 0; theta <= HAL; theta++){
		for (int r = 0; r < RR; r++){
			x = round(r * cos(theta * PI / 180.0) + (double)Grav_point.x);
			y = round(r * sin(theta * PI / 180.0) + (double)Grav_point.y);

			if (0 <= x && x < distance_img.cols && 0 <= y && y < distance_img.rows){
				theta_p[theta][r] = distance_img.at<unsigned char>(y, x);
			}
			else{
				while (r < RR){
					theta_p[theta][r] = 1000;
					r++;
				}
				break;
			}
		}
	}

	//negative
	for (int theta = 0; theta <= HAL; theta++){
		for (int r = 0; r < RR; r++){
			x = round(r * cos(-(theta * PI / 180.0)) + (double)Grav_point.x);
			y = round(r * sin(-(theta * PI / 180.0)) + (double)Grav_point.y);

			if (0 <= x && x < distance_img.cols && 0 <= y && y < distance_img.rows){
				theta_n[theta][r] = distance_img.at<unsigned char>(y, x);
			}
			else{
				while (r < RR){
					theta_n[theta][r] = 1000;
					r++;
				}
				break;
			}
		}
	}

	//table�쐬
	//#pragma omp parallel for
	for (int theta = 0; theta <= HAL; theta++){
		for (int r_p = 0; r_p < RR; r_p++){
			for (int r_n = 0; r_n < RR; r_n++){
				if (theta != 0 && theta != HAL){
					table[theta][r_p][r_n] = theta_p[theta][r_p] + theta_n[theta][r_n];	//stack overflow ������
				}
				else{
					if (r_p == r_n){	//���������
						table[theta][r_p][r_n] = theta_p[theta][r_p] + theta_n[theta][r_n];	//stack overflow ������
					}
					else{
						table[theta][r_p][r_n] = 5000;	//�������Ȃ����5000
					}
				}
			}
		}
	}

	int cost[HAL + 1][RR][RR];	//[theta][i][j]
	int i, j;

	for (i = 0; i < RR; i++){
		for (j = 0; j < RR; j++){
			cost[0][i][j] = table[0][i][j];
		}
	}

	for (int theta = 1; theta <= HAL; theta++){
		for (int i = 0; i < RR; i++){
			for (int j = 0; j < RR; j++){
				cost[theta][i][j] = table[theta][i][j] + MinFromMask(cost, theta, i, j);
			}
		}
	}

	int temp = 0;
	int min = cost[HAL][0][0];
	for (i = 1; i < RR; i++){
		if (min > cost[HAL][i][i]){
			min = cost[HAL][i][i];
			temp = i;
		}
	}

	int px = temp;
	int py = temp;

	int min_x, min_y;
	min_x = px;
	min_y = py;

	temp = min;

	CvPoint contour_p[HAL + 1], contour_n[HAL + 1];

	for (int theta = HAL + 1; theta > 0; theta--){
		temp = cost[theta - 1][px][py];
		px = min_x;
		py = min_y;

		for (i = -MSK; i <= MSK; i++){
			for (j = -MSK; j <= MSK; j++){
				if ((i != 0 && j != 0) && 0 <= px + i && px + i < RR && 0 <= py + i && py + i < RR) {
					if (cost[theta - 1][px + i][py + j] <= temp){
						temp = cost[theta - 1][px + i][py + j];
						min_x = px + i;
						min_y = py + j;
					}
				}
			}
		}

		contour_p[theta - 1].x = round(min_x * cos(((theta - 1) * PI / 180.0)) + (double)Grav_point.x);		//positive
		contour_p[theta - 1].y = round(min_x * sin(((theta - 1) * PI / 180.0)) + (double)Grav_point.y);

		contour_n[theta - 1].x = round(min_y * cos((-(theta - 1) * PI / 180.0)) + (double)Grav_point.x);	//negative
		contour_n[theta - 1].y = round(min_y * sin((-(theta - 1) * PI / 180.0)) + (double)Grav_point.y);
	}

	for (i = 0; i < HAL; i++) {
		contours[frame_num][i].x = contour_p[i].x;
		contours[frame_num][i].y = contour_p[i].y;
	}

	for (i = 0; i < HAL; i++) {
		contours[frame_num][HAL + i].x = contour_n[HAL - i].x;
		contours[frame_num][HAL + i].y = contour_n[HAL - i].y;
	}

	for (i = 0; i < CIR; i++) {
		distances[frame_num][i] = sqrt(pow(Grav_point.x - contours[frame_num][i].x, 2.0) + pow(Grav_point.y - contours[frame_num][i].y, 2.0));
	}


	//���ʏo��
	cvtColor(contour, contour, CV_GRAY2RGB);
	circle(contour, Grav_point, 2, cv::Scalar(255, 150, 50), 4, 4);
	for (int i = 0; i < CIR; i++) {
		line(contour, contours[frame_num][i], contours[frame_num][(i + 1) % (CIR - 1)], Scalar(255, 150, 50), 3, 0);
	}
	return contour;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++
//Snake�ϐ��̏�����(0.5,0.4,0.2)
//+++++++++++++++++++++++++++++++++++++++++++++++++
BlebTrack::BlebTrack() {
	snake_alpha = 0.5;
	snake_beta = 0.4;
	snake_gamma = 0.2;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++
//��l���摜����d�S���Z�o
//+++++++++++++++++++++++++++++++++++++++++++++++++
CvPoint BlebTrack::CenterOfGravityFromBinary(Mat& src_img){
	CvPoint center;
	int sum_x = 0, sum_y = 0, count = 0;

	for (int y = 0; y < src_img.rows; y++){
		for (int x = 0; x < src_img.cols; x++){
			if (src_img.at<unsigned char>(y, x) == 0){
				count++;
				sum_x += x;
				sum_y += y;
			}
		}
	}

	center.x = (int)sum_x / count;
	center.y = (int)sum_y / count;

	return center;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++
//�O���[�X�P�[��
//+++++++++++++++++++++++++++++++++++++++++++++++++
cv::Mat BlebTrack::GrayScale(cv::Mat& src_img) {
	cv::Mat gray_img;
	cvtColor(src_img, gray_img, CV_RGB2GRAY);
	return gray_img;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++
//�F���o
//+++++++++++++++++++++++++++++++++++++++++++++++++
cv::Mat BlebTrack::ChooseCol(cv::Mat& src_img, int n) {
	cv::Mat result_img = cv::Mat::zeros(src_img.size(), src_img.type());

	const auto Access = [](cv::Mat& m, int x, int y, int c)
		-> uchar&{
		return m.data[y*m.step + x*m.elemSize() + c];
	};

	for (int y = 0; y < src_img.rows; y++){
		for (int x = 0; x < src_img.cols; x++){
			uchar b = Access(src_img, x, y, 0);
			uchar g = Access(src_img, x, y, 1);
			uchar r = Access(src_img, x, y, 2);
			//p[0]:B p[1]:G p[2]:R

			switch (n)	{
			case 0:
				if (b > 150){
					Access(result_img, x, y, 0) = 255;
					Access(result_img, x, y, 1) = 255;
					Access(result_img, x, y, 2) = 255;
				}
				break;

			case 1:
				if (g > 20){
					Access(result_img, x, y, 0) = 255;
					Access(result_img, x, y, 1) = 255;
					Access(result_img, x, y, 2) = 255;
				}
				break;

			case 2:
				if (r > 10 || g > 10){
					Access(result_img, x, y, 0) = 255;
					Access(result_img, x, y, 1) = 255;
					Access(result_img, x, y, 2) = 255;
				}
				break;
			}


		}
	}


	return result_img;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++
//�m�C�Y����
//+++++++++++++++++++++++++++++++++++++++++++++++++
cv::Mat BlebTrack::NoiseReduction(cv::Mat& gray_img) {
	cv::Mat clean_img;
	cv::medianBlur(gray_img, clean_img, 9);
	return clean_img;
}

////+++++++++++++++++++++++++++++++++++++++++++++++++
////2�l��
////+++++++++++++++++++++++++++++++++++++++++++++++++
//cv::Mat BlebTrack::threshold(cv::Mat& gray_img) {
//	cv::Mat binary_img;
//	cv::threshold(gray_img, binary_img, 0, 255, cv::THRESH_BINARY_INV | cv::THRESH_OTSU);
//
//	for (int y = 0; y < gray_img.rows; y++){
//		for (int x = 0; x < gray_img.cols; x++){
//			if (binary_img.at<unsigned char>(y, x) == 0)
//				binary_img.at<unsigned char>(y, x) = 1;
//		}
//	}
//
//	return binary_img;
//}

//+++++++++++++++++++++++++++++++++++++++++++++++++
//�ő�A���̈撊�o
//+++++++++++++++++++++++++++++++++++++++++++++++++
Mat BlebTrack::FindMaxSpace(cv::Mat& binary_img, int num) {
	LabelingBS::RegionInfo *regInfo;
	cv::Mat labelarea;
	// Labeling�̌��ʂ��󂯎��
	cv::Mat label(binary_img.size(), CV_16SC1);

	// ���x�����O�����{ �Q�l�������摜�ɑ΂��Ď��s����B
	LabelingBS	labeling;
	RegionInfoBS *ri;
	labeling.Exec(binary_img.data, (short *)label.data, binary_img.cols, binary_img.rows, true, 0);

	// ���x�����O���ʂ��o�͂���A�^�����ȏ�Ԃŏ�����
	cv::Mat outimg(binary_img.size(), CV_8UC1, 255);

	ri = labeling.GetResultRegionInfo(num); //�ő�ʐς̗̈�̏������o���B0�͔w�i���ۂ��A�v�m�F�B
	//ri->GetResult()�@�Ń��x���ԍ���������B
	cv::Mat samples = cv::Mat::ones(label.rows, label.cols, CV_8UC1) * 255; //���h��̉摜���쐬�A�m�F�p
	for (int y = 0; y < label.rows; y++){	//label�̑S��f�̃��x���ԍ��𒲂ׂ�
		for (int x = 0; x < label.cols; x++){
			if (label.at<short>(y, x) == ri->GetResult()) //���ׂ���f�̔ԍ����ő�ʐς̗̈�Ɠ����ԍ��������火
				samples.at<unsigned char>(y, x) = 0;	//samples�̓������W�̉�f�𔒂��h��
		}
	}

	//cv::namedWindow("���H", CV_WINDOW_AUTOSIZE | CV_WINDOW_FREERATIO);
	//cv::imshow("���H", samples);

	return samples;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++
//Snake����
//+++++++++++++++++++++++++++++++++++++++++++++++++
void BlebTrack::SnakePre(cv::Mat& src_img, CvPoint *contour){
	CvPoint center;

	width = src_img.cols;
	height = src_img.rows;

	center.x = width / 2;
	center.y = height / 2;


	for (int i = 0; i < LEN; i++) {
		contour[i].x = (int)((center.x * cos(2 * CV_PI * i / LEN) + center.x) / 10 * 10);
		contour[i].y = (int)((center.y * sin(2 * CV_PI * i / LEN) + center.y) / 10 * 10);
	}
}

//+++++++++++++++++++++++++++++++++++++++++++++++++
//���I�֊s�@
//+++++++++++++++++++++++++++++++++++++++++++++++++
void BlebTrack::Snake(cv::Mat& src_img, CvPoint *contour, int length) {
	CvPoint center;

	width = src_img.cols;
	height = src_img.rows;

	center.x = width / 2;
	center.y = height / 2;

	for (int i = 0; i < length; i++) {
		contour[i].x = (int)((center.x * cos(2 * CV_PI * i / length) + center.x) / 1);
		contour[i].y = (int)((center.y * sin(2 * CV_PI * i / length) + center.y) / 1);
	}

	IplImage src = src_img;
	IplImage *iplImage;
	CvPoint PreContour[60];
	int i, sum, threshold = 100, sum_temp = 0;
	int x, y, j = 0, k = 0;

	iplImage = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);

	cvCopy(&src, iplImage);

	do{
		k++;
		sum = 0;

		for (i = 0; i < length; i++){
			x = contour[i].x;
			y = contour[i].y;
			PreContour[i].x = x;
			PreContour[i].y = y;
		}

		cvSnakeImage(iplImage, contour, length, &snake_alpha, &snake_beta, &snake_gamma, 1, cvSize(15, 15), cvTermCriteria(CV_TERMCRIT_ITER, 1, 0.0), 1);

		for (i = 0; i < length; i++){
			sum += pow(contour[i].x - PreContour[i].x, 2) + pow(contour[i].y - PreContour[i].y, 2);
		}
		threshold = sum;

		if (sum_temp == sum)
		{
			j++;
			if (j > 100){
				j = 0;
				break;
			}
		}

		if (k > 1000)break;

		sum_temp = sum;

	} while (threshold > 10);


}

//+++++++++++++++++++++++++++++++++++++++++++++++++
//�֊s������d�S���Z�o
//+++++++++++++++++++++++++++++++++++++++++++++++++
CvPoint BlebTrack::CenterOfGravity(CvPoint *contour, int length)	//�֊s������d�S���Z�o
{
	//	length = 60;
	CvPoint point;
	int sum_x = 0, sum_y = 0;

	for (int i = 0; i < length; i++){
		sum_x += contour[i].x;
		sum_y += contour[i].y;
	}

	point.x = (int)sum_x / length;
	point.y = (int)sum_y / length;

	return point;
}


//+++++++++++++++++++++++++++++++++++++++++++++++++
//�}�X�N������ŏ��l���擾
//+++++++++++++++++++++++++++++++++++++++++++++++++
int BlebTrack::MinFromMask(int cost[HAL][RR][RR], int theta, int x, int y) {
	int min;
	int i, j, temp;

	temp = cost[theta - 1][x][y];

	for (i = -MSK; i <= MSK; i++){
		for (j = -MSK; j <= MSK; j++){
			if ((i != 0 && j != 0) && 0 <= x + i && x + i < RR && 0 <= y + i && y + i < RR) {
				min = std::min(cost[theta - 1][x + i][y + j], temp);
				temp = min;
			}
		}
	}

	return min;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++
//�ȗ��}�b�v�̐���
//+++++++++++++++++++++++++++++++++++++++++++++++++
// �ȗ����a�Ƃ��̃s�[�N���o
//  �ȗ������߂����_B�ɂ��āC�����܂ł̒��SO����̋����� d2, 
//  ��������p�x�{�ƕ����̓_A�ɂ��Ē��SO����̋����� d1
//  ��������p�x�|�ƕ����̓_C�ɂ��Ē��SO����̋����� d3
//  �Ƃ���D
//  ���̎��CB�̋ȗ����aR�C���Ȃ킿�O�p�`ABC�̊O�ډ~���aR�͎����𖞂���
//                      a^2 b^2 c^2
//  R^2 = ------------------------------------   (1)
//          4 a^2 b^2 - (a^2 + b^2 - c^2)^2   
//  ������
//    a�͕�AB�̋����ŁC�]���藝���  a^2 = d1^2 + d2^2 - 2d1d2 cos��
//    b�͕�BC�̋����ŁC�]���藝���  b^2 = d2^2 + d3^2 - 2d2d3 cos��
//    c�͕�AC�̋����ŁC�]���藝���  c^2 = d1^2 + d3^2 - 2d1d3 cos2��
//  �܂��O�ډ~�̒��SQ�͈�ʂ�O�ƈ�v���Ȃ��D
//  �Ȃ��㎮(1)�͎��̂悤�ɓ��o�����D�܂������藝���
//            c                       c^2
//   2R = ---------  ==>  R^2 = -------------           (2)
//        sin ��ABC               1 - cos^2 ��ABC
//   ��������D����ɕ�AC�Ɋւ��������̗]���藝���
//       
//      c^2 = a^2 + b^2 - 2ab cos ��ABC   (3)
//
//    �Ȃ̂ŁC 
//                    (a^2 + b^2 - c^2)^2        (a^2 + b^2 - c^2)^2  
//     cos^2 ��ABC = ------------------    = --------------------------  (4)
//                         (2ab)^2                  4 a^2 b^2
//
//   (4)��(2)�ɑ�����Đ������邱�Ƃ�(1)��������D

void BlebTrack::CurvatureMap(double **distances, int NumOfFrames){
	Mat map(Size((CIR - 1), NumOfFrames - 1), CV_8UC3, cv::Scalar(255, 255, 255));
	int i, j, k, l;

	double **curvature = new double*[NumOfFrames];
	for (int i = 0; i < NumOfFrames; i++) {
		curvature[i] = new double[CIR - 1];
	}

	double d1, d2, d3, a2, b2, c2, r, p;
	double biggest = 0.0;

	double cos_theta = cos(2.0 * PI * (double)INTERVAL_ANGLE_FOR_CURVATURE / 360.0);
	double cos_2theta = cos(2.0 * PI * 2.0 * (double)INTERVAL_ANGLE_FOR_CURVATURE / 360.0);

#pragma omp parallel for
	for (i = 0; i < NumOfFrames; i++){
		for (j = 0; j < CIR - 1; j++){
			d2 = distances[i][j];
			d1 = distances[i][(j - INTERVAL_ANGLE_FOR_CURVATURE + (CIR - 1)) % (CIR - 1)];
			d3 = distances[i][(j + INTERVAL_ANGLE_FOR_CURVATURE) % (CIR - 1)];

			a2 = d1*d1 + d2*d2 - 2.0*d1*d2*cos_theta;
			b2 = d2*d2 + d3*d3 - 2.0*d2*d3*cos_theta;
			c2 = d1*d1 + d3*d3 - 2.0*d1*d3*cos_2theta;
			r = sqrt(((a2*b2*c2) / (4.0*a2*b2 - (a2 + b2 - c2)* (a2 + b2 - c2))));
			p = 2.0 * d1 * d3 * cos_theta / (d1 + d3);

			if (r == 0.0){
				curvature[i][j] = 0.0;
			}
			else {
				if (d2 > p){
					curvature[i][j] = 1.0 / r;
				}
				else {
					curvature[i][j] = -1.0 / r;
				}
			}

			if (biggest < curvature[i][j])
				biggest = curvature[i][j];
			//			cout << "curvature[]["<< j <<"]:"<< curvature[i][j] << endl;

			//			cout << "biggest:" << biggest << endl;
			//			cout << "curvature:" << ((curvature[i][j]) / (biggest)) * 255.0 << endl;
		}
		//break;
		//getchar();
	}

	const auto Access = [](cv::Mat& m, int x, int y, int c)
		-> uchar&{
		return m.data[y*m.step + x*m.elemSize() + c];
	};

	cvtColor(map, map, CV_BGR2HSV);

	int curvature_out;

	for (i = 0; i < NumOfFrames; i++){
		for (j = 0; j < (CIR - 1); j++){
			curvature_out = (int)(((curvature[i][j]) / (biggest)) * 255.0);
			//		cout << curvature_out << endl;
			if (curvature_out >= 0){
				if (i != 0){
					if (distances[i][j] >= distances[i - 1][j]){
						Access(map, j, i, 0) = 0;				//H
						Access(map, j, i, 1) = curvature_out;	//S
						Access(map, j, i, 2) = 255;				//V
					}
					else {
						Access(map, j, i, 0) = 120;				//H
						Access(map, j, i, 1) = curvature_out;	//S
						Access(map, j, i, 2) = 255;				//V
					}
				}
				else{
					Access(map, j, i, 0) = 0;				//H
					Access(map, j, i, 1) = curvature_out;	//S
					Access(map, j, i, 2) = 255;				//V
				}
			}
		}
	}

	cvtColor(map, map, CV_HSV2BGR);
	resize(map, map, cv::Size(), 3, 3);

	imwrite("CurvetureMap.png", map);

	cv::namedWindow("Curvature Map", CV_WINDOW_AUTOSIZE | CV_WINDOW_FREERATIO);
	cv::imshow("Curvature Map", map);

	for (int i = 0; i < NumOfFrames; i++) {
		delete[] curvature[i];
	}
	delete[] curvature;
}


//+++++++++++++++++++++++++++++++++++++++++++++++++
//�����}�b�v�̐���
//+++++++++++++++++++++++++++++++++++++++++++++++++
void BlebTrack::DistanceMap(double **distances, int NumOfFrames){
	Mat map(Size((CIR - 1), NumOfFrames - 1), CV_8UC3, cv::Scalar(255, 255, 255));
	cvtColor(map, map, CV_BGR2HSV);
	int i, j, k, l;

	double biggest = 0.0;

	double **difference = new double*[NumOfFrames];
	for (int i = 0; i < NumOfFrames - 1; i++) {
		difference[i] = new double[CIR - 1];
	}

#pragma omp parallel for
	for (i = 0; i < NumOfFrames - 1; i++){
		for (j = 0; j < CIR - 1; j++){
			difference[i][j] = distances[i + 1][j] - distances[i][j];
			if (biggest < difference[i][j])
				biggest = difference[i][j];
		}
	}

	//count << map.channels << endl;

	const auto Access = [](cv::Mat& m, int x, int y, int c)
		-> uchar&{
		return m.data[y*m.step + x*m.elemSize() + c];
	};

	int p;

	for (i = 0; i < NumOfFrames - 1; i++){
		for (j = 0; j < (CIR - 1); j++){
			p = (int)(((difference[i][j]) / (biggest)) * 255.0);

			//Access(map, j, i, 0) = j;			//H
			//Access(map, j, i, 1) = 255;// abs(p);			//S
			//Access(map, j, i, 2) = 255;			//V

			if (p > 0){
				//				cout << p << endl;

				Access(map, j, i, 0) = 0;			//H
				Access(map, j, i, 1) = p;			//S
				Access(map, j, i, 2) = 255;			//V
			}
			else{
				Access(map, j, i, 0) = 120;			//H
				Access(map, j, i, 1) = abs(p);		//S
				Access(map, j, i, 2) = 255;			//V
			}
		}
	}

	cvtColor(map, map, CV_HSV2BGR);

	resize(map, map, cv::Size(), 3, 3);

	imwrite("DistanceMap.png", map);

	cv::namedWindow("Distance Map", CV_WINDOW_AUTOSIZE | CV_WINDOW_FREERATIO);
	cv::imshow("Distance Map", map);

	for (int i = 0; i < NumOfFrames - 1; i++) {
		delete[] difference[i];
	}
	delete[] difference;
}


//+++++++++++++++++++++++++++++++++++++++++++++++++
//�O�s���o
//+++++++++++++++++++++++++++++++++++++++++++++++++
cv::Mat BlebTrack::ContourExtraction(cv::Mat& binary_img, int NumOfRegions) {
	LabelingBS::RegionInfo *regInfo;
	Mat labelarea;
	// Labeling�̌��ʂ��󂯎��
	Mat label(binary_img.size(), CV_16SC1);

	// ���x�����O�����{ �Q�l�������摜�ɑ΂��Ď��s����B
	LabelingBS	labeling;
	RegionInfoBS *ri;
	labeling.Exec(binary_img.data, (short *)label.data, binary_img.cols, binary_img.rows, true, 0);

	cout << labeling.GetNumOfResultRegions() << endl;

	Mat output_img = Mat::ones(label.rows, label.cols, CV_8UC1) * 255; //���h��̉摜���쐬�A�m�F�p

	for (size_t num = 0; num < NumOfRegions - 2; num++){
		ri = labeling.GetResultRegionInfo(num); //�ő�ʐς̗̈�̏������o���B0�͔w�i���ۂ��A�v�m�F�B
		//ri->GetResult()�@�Ń��x���ԍ���������B
		for (int y = 0; y < label.rows; y++){	//label�̑S��f�̃��x���ԍ��𒲂ׂ�
			for (int x = 0; x < label.cols; x++){
				if (label.at<short>(y, x) == ri->GetResult()) //���ׂ���f�̔ԍ����ő�ʐς̗̈�Ɠ����ԍ��������火
					output_img.at<unsigned char>(y, x) = 0;	//samples�̓������W�̉�f�𔒂��h��
			}
		}
	}

	//cv::namedWindow("���H", CV_WINDOW_AUTOSIZE | CV_WINDOW_FREERATIO);
	//cv::imshow("���H", samples);

	return output_img;
}
