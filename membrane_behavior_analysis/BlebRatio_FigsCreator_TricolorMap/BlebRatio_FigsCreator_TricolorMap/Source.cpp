#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <omp.h>
#include <stack>
#include <fstream>
#include <string>
#include <omp.h>
#include <Labeling/Labeling.h>


#pragma warning(disable : 4996)

#define NUM 4
#define CIR 360

#define RegionSize 20
//#define RegionSize 30

#define Threshold_positive 254
#define Threshold_negative 254

#define Blur_positive 3
#define Blur_negative 3

using namespace std;
using namespace cv;

#define FILENUM 3

#define IN "../input/"
#define OUT "../result/"

#define FILE "/distance.csv"
#define SEG_FILE "/segmented.tif"

#define IMG_EXTENSION ".tif"
#define CSV_EXTENSION ".csv"
#define TXT_EXTENSION ".txt"

#define CASE1 "Sample/"


#define IMG_EXTENSION ".tif"
#define CSV_EXTENSION ".csv"

const int BINARY_VALUE = 10;

int const AllFrameNum = 121;

template <typename List>
void split(const std::string& s, const std::string& delim, List& result)
{
	result.clear();

	using string = std::string;
	string::size_type pos = 0;

	while (pos != string::npos)
	{
		string::size_type p = s.find(delim, pos);

		if (p == string::npos)
		{
			result.push_back(s.substr(pos));
			break;
		}
		else {
			result.push_back(s.substr(pos, p - pos));
		}

		pos = p + delim.size();
	}
}

const auto Access = [](cv::Mat& m, int x, int y, int c)
-> uchar&{
	return m.data[y*m.step + x*m.elemSize() + c];
};


Mat DeleteSmall(cv::Mat& binary_img) {
	int nop;

	cv::threshold(binary_img, binary_img, 0, 255, cv::THRESH_BINARY_INV | cv::THRESH_OTSU);
	LabelingBS::RegionInfo *regInfo;
	cv::Mat labelarea;
	// Labelingの結果を受け取る
	cv::Mat label(binary_img.size(), CV_16SC1);

	// ラベリングを実施 ２値化した画像に対して実行する。
	LabelingBS	labeling;
	RegionInfoBS *ri;
	labeling.Exec(binary_img.data, (short *)label.data, binary_img.cols, binary_img.rows, true, 0);

	// ラベリング結果を出力する、真っ白な状態で初期化
	cv::Mat outimg(binary_img.size(), CV_8UC1, 255);
	cv::Mat result_img = cv::Mat::ones(label.rows, label.cols, CV_8UC1) * 255; //黒塗りの画像を作成、確認用

	for (int i = 0; i < labeling.GetNumOfRegions(); i++) {
		//ri = labeling.GetResultRegionInfo(num); //最大面積の領域の情報を取り出す。0は背景っぽい、要確認。
		//ri->GetResult()　でラベル番号が分かる。

		regInfo = labeling.GetResultRegionInfo(i);

		nop = regInfo->GetNumOfPixels(); /* 領域の面積(画素数) */
		//cout << nop << endl;
		//getchar();


		if (nop > RegionSize){
			//if (i == 0){
			for (int y = 0; y < label.rows; y++){	//labelの全画素のラベル番号を調べる
				for (int x = 0; x < label.cols; x++){
					if (label.at<short>(y, x) == regInfo->GetResult())
						result_img.at<unsigned char>(y, x) = 0;	//samplesの同じ座標の画素を白く塗る
				}
			}
		}
	}


	//imwrite("lab.png", result_img);

	return result_img;
}


int main(int argc, char *argv[])
{
	string CASE;
	for (size_t NumOfSamples = 0; NumOfSamples <1; NumOfSamples++){

		switch (NumOfSamples){
		case 0:
			CASE = CASE1;
			break;
		}
		for (size_t file_num = 0; file_num < 1; file_num++){
			double abs_biggest = 0.0, red_biggest = 0.0, blue_smallest = 0.0;
			int abs_p, red_p, blue_p;

			vector<vector<double>> distances(AllFrameNum, vector<double>(CIR));
			ifstream fin_dis(IN + CASE + to_string(file_num + 1) + "/distance.csv");
			string str_dis;
			getline(fin_dis, str_dis);
			for (size_t imgNum = 0; imgNum < AllFrameNum; imgNum++){
				getline(fin_dis, str_dis);
				istringstream stream(str_dis);
				vector<string> result;
				split(str_dis, ",", result);
				for (size_t angleNum = 0; angleNum < CIR; angleNum++){
					distances[imgNum][angleNum] = stod(result[angleNum + 1]);
				}
			}
			fin_dis.close();


			vector<vector<double>> differences(AllFrameNum - 1, vector<double>(CIR, 0));
			for (size_t i = 0; i < AllFrameNum - 1; i++) {
				for (size_t j = 0; j < CIR; j++) {
					differences[i][j] = distances[i + 1][j] - distances[i][j];

					if (abs_biggest < abs(differences[i][j]))
						abs_biggest = differences[i][j];
					if (differences[i][j]<0.0&&blue_smallest > differences[i][j])
						blue_smallest = differences[i][j];
					if (differences[i][j] >= 0.0&&red_biggest < differences[i][j])
						red_biggest = differences[i][j];
				}
			}

			//Mat result_map(cv::Size(CIR, AllFrameNum - 1), CV_8UC3, cv::Scalar(255, 255, 255));
			//cvtColor(result_map, result_map, CV_BGR2HSV);

			//for (size_t i = 0; i < AllFrameNum-1; i++){
			//	for (size_t j = 0; j < CIR; j++){
			//		if (differences[i][j] > 0){
			//			Access(result_map, j, i, 0) = 0;		//H
			//			Access(result_map, j, i, 1) = 255;		//S
			//			Access(result_map, j, i, 2) = 255;		//V
			//		}
			//		else if (differences[i][j] < 0){
			//			Access(result_map, j, i, 0) = 120;		//H
			//			Access(result_map, j, i, 1) = 255;		//S
			//			Access(result_map, j, i, 2) = 255;		//V
			//		}
			//	}
			//}
			//cvtColor(result_map, result_map, CV_HSV2BGR);
			//imwrite("../TricolorMap.tif", result_map);







			Mat dis_map(cv::Size(CIR, AllFrameNum - 1), CV_8UC3, cv::Scalar(255, 255, 255));
			Mat red_map(cv::Size(CIR, AllFrameNum - 1), CV_8UC3, cv::Scalar(255, 255, 255));
			Mat blue_map(cv::Size(CIR, AllFrameNum - 1), CV_8UC3, cv::Scalar(255, 255, 255));

			cvtColor(dis_map, dis_map, CV_BGR2HSV);
			cvtColor(red_map, red_map, CV_BGR2HSV);
			cvtColor(blue_map, blue_map, CV_BGR2HSV);

			for (size_t i = 0; i < AllFrameNum - 1; i++){
				for (size_t j = 0; j < CIR; j++){
					abs_p = round(((differences[i][j]) / (abs_biggest)) * 255.0);
					//red_p = round(((differences[i][j]) / (red_biggest)) * 255.0);
					red_p = round(((differences[i][j]) / (abs_biggest)) * 255.0);
					//blue_p = round(((differences[i][j]) / (blue_smallest)) * 255.0);
					blue_p = round(((differences[i][j]) / (abs_biggest)) * 255.0);

					if (differences[i][j] > 0.0){
						Access(red_map, j, i, 0) = 0;			//H
						Access(red_map, j, i, 1) = red_p;			//S
						Access(red_map, j, i, 2) = 255;			//V

						Access(dis_map, j, i, 0) = 0;			//H
						Access(dis_map, j, i, 1) = red_p;			//S
						Access(dis_map, j, i, 2) = 255;			//V
					}
					else if (differences[i][j] < 0.0){
						Access(blue_map, j, i, 0) = 120;			//H
						Access(blue_map, j, i, 1) = abs(blue_p);	//S
						Access(blue_map, j, i, 2) = 255;			//V

						Access(dis_map, j, i, 0) = 120;			//H
						Access(dis_map, j, i, 1) = abs(blue_p);		//S
						Access(dis_map, j, i, 2) = 255;			//V
					}
				}
			}

			cvtColor(blue_map, blue_map, CV_HSV2BGR);
			cvtColor(red_map, red_map, CV_HSV2BGR);
			cvtColor(dis_map, dis_map, CV_HSV2BGR);

			imwrite(OUT + CASE + to_string(file_num + 1) + "/BlueMap.tif", blue_map);
			imwrite(OUT + CASE + to_string(file_num + 1) + "/RedMap.tif", red_map);
			imwrite(OUT + CASE + to_string(file_num + 1) + "/TricolorMap.tif", dis_map);

			Mat blue_binary = imread(OUT + CASE + to_string(file_num + 1) + "/BlueMap.tif", 0);
			////cv::threshold(blue_binary, blue_binary, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
			//cv::threshold(blue_binary, blue_binary, Threshold_negative, 255, cv::THRESH_BINARY);
			//medianBlur(blue_binary, blue_binary, Blur_negative);
			//sprintf(output_place, "../Map/WT_%d/BlueBinary.png", num);
			//imwrite(output_place, blue_binary);

			cv::threshold(blue_binary, blue_binary, Threshold_negative, 255, cv::THRESH_BINARY);

			blue_binary = DeleteSmall(blue_binary);
			//imwrite(OUT + CASE + to_string(file_num + 1) + "/BlueBinary_buff.tif", blue_binary);
			medianBlur(blue_binary, blue_binary, Blur_negative);
			//imwrite(OUT + CASE + to_string(file_num + 1) + "/BlueBinary.tif", blue_binary);


			Mat red_binary = imread(OUT + CASE + to_string(file_num + 1) + "/RedMap.tif", 0);
			//getchar();
			////cv::threshold(red_binary, red_binary, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
			//cv::threshold(red_binary, red_binary, Threshold_positive, 255, cv::THRESH_BINARY);
			//medianBlur(red_binary, red_binary, Blur_positive);
			//sprintf(output_place, "../Map/WT_%d/RedBinary.png", num);
			//imwrite(output_place, red_binary);

			//cv::threshold(red_binary, red_binary, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
			cv::threshold(red_binary, red_binary, Threshold_positive, 255, cv::THRESH_BINARY);
			red_binary = DeleteSmall(red_binary);
			medianBlur(red_binary, red_binary, Blur_positive);

			Mat result_map(cv::Size(CIR, AllFrameNum - 1), CV_8UC3, cv::Scalar(255, 255, 255));

			cvtColor(result_map, result_map, CV_BGR2HSV);

			for (size_t i = 0; i < AllFrameNum - 1; i++){
				for (size_t j = 0; j < CIR; j++){
					if (red_binary.at<unsigned char>(i, j) == 0){
						Access(result_map, j, i, 0) = 0;		//H
						Access(result_map, j, i, 1) = 255;		//S
						Access(result_map, j, i, 2) = 255;		//V
					}
					else if (blue_binary.at<unsigned char>(i, j) == 0){
						Access(result_map, j, i, 0) = 120;		//H
						Access(result_map, j, i, 1) = 255;		//S
						Access(result_map, j, i, 2) = 255;		//V
					}
				}
			}


			cvtColor(result_map, result_map, CV_HSV2BGR);
			//resize(result_map, result_map, cv::Size(), 3, 3);
			imwrite(OUT + CASE + to_string(file_num + 1) + "/trans_TricolorMap.tif", result_map);

		}
	}
	return 0;
}