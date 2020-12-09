#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <omp.h>
#include <stack>
#include <fstream>
#include <string>
#include <omp.h>

#include "BlebTrack.h"

#pragma warning(disable : 4996)

#define NUM 4
#define CIR 360

using namespace std;
using namespace cv;

#define FILENUM 3

//#define IN_Ena "../input/EnaRFP/"
//#define OUT_Ena "../result/EnaRFP/"

//#define IN_Vasp "../input/VaspRFP/"
//#define OUT_Vasp "../result/VaspRFP/"

//#define IN_OnlyRFP "../input/OnlyRFP/"
//#define OUT_OnlyRFP "../result/OnlyRFP/"

#define IN "../input/"
#define OUT "../result/"

#define SAMPLE1 "Sample/"

#define FILE "/distance.csv"
#define SEG_FILE "/segmented.tif"

#define IMG_EXTENSION ".tif"
#define CSV_EXTENSION ".csv"
#define TXT_EXTENSION ".txt"



#define IMG_EXTENSION ".tif"
#define CSV_EXTENSION ".csv"

const int BINARY_VALUE = 10;

int const AllFrameNum = 121;

//int morph_size = 6;
int morph_size = 8;
int morph_size1 = 3;
int morph_size2 = 1;
Mat kernel = getStructuringElement(MORPH_ELLIPSE, cv::Size(2 * morph_size + 1, 2 * morph_size + 1), cv::Point(morph_size, morph_size));
Mat kernel1 = getStructuringElement(MORPH_ELLIPSE, cv::Size(2 * morph_size1 + 1, 2 * morph_size1 + 1), cv::Point(morph_size1, morph_size1));
Mat kernel2 = getStructuringElement(MORPH_CROSS, cv::Size(2 * morph_size2 + 1, 2 * morph_size2 + 1), cv::Point(morph_size2, morph_size2));


int main(int argc, char *argv[])
{
	cout << "全フレーム数: " << AllFrameNum << endl;

	string SAMPLE;

	for (size_t NumOfSamples = 1; NumOfSamples < 2; NumOfSamples++){

		switch (NumOfSamples){
		case 1:
			SAMPLE = SAMPLE1;
			break;
		}

		for (size_t file_num = 0; file_num < 1; file_num++){
			BlebTrack bleb;
			cout << "file_num" << file_num << endl;
			/*画像読み込み*/
			Vector<Mat> gfp_frame, rfp_frame;
			for (int num = 0; num < AllFrameNum; num++){
				Mat gfp_temp, rfp_temp;

				gfp_temp = imread(IN + SAMPLE + to_string(file_num + 1) + "/" +  to_string(num + 1) + IMG_EXTENSION, 0);
				rfp_temp = imread(IN + SAMPLE + to_string(file_num + 1) + "/" +to_string(num + 1) + IMG_EXTENSION, 0);

				gfp_frame.push_back(gfp_temp);
				rfp_frame.push_back(rfp_temp);
			}
			cout << "GFP画像数: " << gfp_frame.size() << endl;
			cout << "RFP画像数: " << rfp_frame.size() << endl;


			/*重心算出(RFP画像より)*/
			int Grav_sum_x = 0, Grav_sum_y = 0;
			for (size_t num = 0; num < AllFrameNum; num++){
				Mat binary_temp;
				Mat temp_img;
				binary_temp = bleb.NoiseReduction(rfp_frame[num]);
				morphologyEx(binary_temp, binary_temp, cv::MORPH_OPEN, kernel2);
				threshold(binary_temp, binary_temp, BINARY_VALUE, 255, cv::THRESH_BINARY);
				temp_img = binary_temp.clone();
				morphologyEx(binary_temp, binary_temp, cv::MORPH_CLOSE, kernel);

				erode(binary_temp, binary_temp, kernel2, Point(-1,-1), 1, BORDER_CONSTANT, 0);
				Mat binary_src = binary_temp.clone();
				Mat binary_temp_inv;
				floodFill(binary_temp, cv::Point(0, 0), Scalar(255));
				bitwise_not(binary_temp, binary_temp_inv);
				Mat im_out = (binary_src | binary_temp_inv);
				binary_temp = im_out.clone();
				morphologyEx(binary_temp, binary_temp, cv::MORPH_OPEN, kernel);
				threshold(binary_temp, binary_temp, BINARY_VALUE, 255, cv::THRESH_BINARY_INV);
				imwrite("../" + SAMPLE + to_string(file_num + 1) + "/" + to_string(num) + ".tif", binary_temp);

				CvPoint Grav_point_of_a_frame = bleb.CenterOfGravityFromBinary(binary_temp);
				Grav_sum_x += Grav_point_of_a_frame.x;
				Grav_sum_y += Grav_point_of_a_frame.y;
			}
			//continue;
			CvPoint Grav_point;
			Grav_point.x = round(Grav_sum_x / AllFrameNum);
			Grav_point.y = round(Grav_sum_y / AllFrameNum);
			cout << "重心: (" << Grav_point.x << ", " << Grav_point.y << ")" << endl;
			ofstream fout_cog(OUT + SAMPLE + to_string(file_num + 1) + "/CenterOfGravity" + CSV_EXTENSION, ios::trunc);
			fout_cog << Grav_point.x << ", " << Grav_point.y << endl;


			/*輪郭抽出*/
			vector<vector<double>> Distances(AllFrameNum, vector<double>(CIR + 1, 0));
			vector<vector<CvPoint>> Contours(AllFrameNum, vector<CvPoint>(CIR + 1));
			int count = 0;
			//#pragma omp parallel for
			for (int num = 0; num < AllFrameNum; num++){
				cout << "i" <<num << endl;
				//cout << count + 1 << "/" << AllFrameNum << endl;

				/*下準備*/
				Mat binary_temp;
				Mat temp_img;
				binary_temp = rfp_frame[num].clone();
				//binary_temp = bleb.NoiseReduction(rfp_frame[num]);
				morphologyEx(binary_temp, binary_temp, cv::MORPH_OPEN, kernel2);
				threshold(binary_temp, binary_temp, BINARY_VALUE, 255, cv::THRESH_BINARY);
				temp_img = binary_temp.clone();
				morphologyEx(binary_temp, binary_temp, cv::MORPH_CLOSE, kernel);

				erode(binary_temp, binary_temp, kernel2, Point(-1, -1), 1, BORDER_CONSTANT, 0);
				Mat binary_src = binary_temp.clone();
				Mat binary_temp_inv;
				floodFill(binary_temp, cv::Point(0, 0), Scalar(255));
				bitwise_not(binary_temp, binary_temp_inv);
				Mat im_out = (binary_src | binary_temp_inv);
				binary_temp = im_out;
				morphologyEx(binary_temp, binary_temp, cv::MORPH_OPEN, kernel);
				threshold(binary_temp, binary_temp, BINARY_VALUE, 255, cv::THRESH_BINARY_INV);
				temp_img = binary_temp.clone();
				//previous source
				//temp_img = bleb.NoiseReduction(rfp_frame[num]);
				//threshold(temp_img, temp_img, 0, 255, cv::THRESH_BINARY_INV | cv::THRESH_OTSU);
				//threshold(temp_img, temp_img, BINARY_VALUE, 255, cv::THRESH_BINARY_INV);

				//medianBlur(temp_img, temp_img, 3);

				//imwrite("../result/binary/" + to_string(file_num + 1) + "/binary"  + to_string(num + 1) + ".tif", temp_img);


				/*DPによる輪郭抽出*/
				
				Mat dp_img;
				dp_img = bleb.NewDP(Contours, temp_img, Grav_point, Distances, num, rfp_frame[num]);
				imwrite("../workspace/ExtractedContour/"+ SAMPLE + to_string(file_num + 1) + "/" + to_string(num + 1) + ".tif", dp_img);
//				imwrite(OUT + SAMPLE + to_string(file_num + 1) + "/contour_img/" + to_string(num + 1) + IMG_EXTENSION, dp_img);
				//count++;

				//getchar();
			}
			/*結果出力*/
			cout << "結果出力" << endl;
			ofstream fout_dis(OUT + SAMPLE + to_string(file_num + 1) + "/distance" + CSV_EXTENSION, ios::trunc);
			fout_dis << "ImgNum";
			for (size_t i = 0; i < CIR; i++) { fout_dis << "," << i; } fout_dis << endl;
			for (size_t imgNum = 0; imgNum < AllFrameNum; imgNum++){
				fout_dis << imgNum + 1;
				for (size_t angle = 0; angle < CIR; angle++){
					fout_dis << "," << Distances[imgNum][angle];
				}
				fout_dis << endl;
			}
			cout << "contour出力" << endl;
			ofstream fout_con(OUT + SAMPLE + to_string(file_num + 1) + "/contour" + CSV_EXTENSION, ios::trunc);
			fout_con << "ImgNum" << ",";
			for (size_t i = 0; i < CIR; i++) { fout_con << i << "," << ","; } fout_con << endl;
			for (size_t imgNum = 0; imgNum < AllFrameNum; imgNum++){
				fout_con << imgNum + 1;
				for (size_t angle = 0; angle < CIR; angle++){
					fout_con << "," << Contours[imgNum][angle].x << "," << Contours[imgNum][angle].y;
				}
				fout_con << endl;
			}

			//getchar();
		}
		cout << "繰り返します" << endl;

	}

	return 0;
}