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
#define RR 200	//重心からの最長辺 *要確認

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
	BlebTrack::BlebTrack();		//Snake変数の初期化
	void SnakePre(cv::Mat& src_img, CvPoint *contour);	//snakeの準備
	cv::Mat GrayScale(cv::Mat& src_img);	//グレースケール
	cv::Mat ChooseCol(cv::Mat& src_img, int n); //色抽出
	cv::Mat NoiseReduction(cv::Mat& src_img);	//ノイズ除去
	//cv::Mat threshold(cv::Mat& src_img);	//2値化
	Mat FindMaxSpace(cv::Mat& src_img, int num);	//最大連続領域抽出
	void Snake(cv::Mat& src_img, CvPoint *contour, int length);	//動的輪郭法
	CvPoint CenterOfGravity(CvPoint *contour, int length);	//輪郭線から重心を算出
	//cv::Mat DPforOutline(CvPoint contours[CIR], cv::Mat& binary_img, Point2d Grav_point, double **distances, int frame_count);	//DPを用いた輪郭抽出
	int MinFromMask(int cost[HAL][RR][RR], int theta, int x, int y);	//マスク内から最小値を取得
	void CurvatureMap(double **distances, int NumOfFrames);	//曲率マップの生成		
	void DistanceMap(double **distances, int NumOfFrames);	//距離マップの生成
	Mat ContourExtraction(cv::Mat& binary_img, int NumOfRegions);	//外郭抽出
	Mat NewDP(vector<vector<CvPoint>> &contours, cv::Mat& src_img, Point Grav_point, vector<vector<double>> &distances, int frame_num, cv::Mat& row_img);
	CvPoint CenterOfGravityFromBinary(Mat& src_img);
};