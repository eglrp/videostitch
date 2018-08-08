#include<iostream>
#include <opencv2/opencv.hpp>
#include "opencv2/core/core.hpp"
#include "highgui.h"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/features2d/features2d.hpp"

#include "opencv2/stitching/detail/autocalib.hpp"
#include "opencv2/stitching/detail/blenders.hpp"
#include "opencv2/stitching/detail/camera.hpp"
#include "opencv2/stitching/detail/exposure_compensate.hpp"
#include "opencv2/stitching/detail/matchers.hpp"
#include "opencv2/stitching/detail/motion_estimators.hpp"
#include "opencv2/stitching/detail/seam_finders.hpp"
#include "opencv2/stitching/detail/util.hpp"
#include "opencv2/stitching/detail/warpers.hpp"
#include "opencv2/stitching/warpers.hpp"
#include <QtCore/QDir>

using namespace std;
using namespace cv;
using namespace detail;

//ͼƬ�洢·��
const string path = "../temp/";
//����·��
vector<string> imgs_path;
//֡���
int frameInterval = 400;

//
Mat result;


void init() {

	QDir dir(QString::fromStdString(path));
	if (dir.exists())
	{
		//����ļ�����
		dir.setFilter(QDir::Files);
		for (int i = 0; i < dir.count(); i++)
			dir.remove(dir[i]);
	}
	else {
		//�����洢�ļ���
		dir.mkdir(QString::fromStdString(path));
	}

}

bool getImages(string videoPath) {

	Mat frame;
	size_t count = 0;
	stringstream ss;
	string path_buf;

	cv::VideoCapture capture(videoPath);
	if (!capture.isOpened())
	{
		cout << "��Ƶ�ļ��޷��򿪣���";
		return false;
	}

	imgs_path.clear();
	while (capture.read(frame))
	{
		if (frame.empty())
			break;
		
		if (count%frameInterval == 0)
		{
			ss.clear();
			ss << path << count << ".jpg";
			ss >> path_buf;
			imgs_path.emplace_back(path_buf);
			imwrite(path_buf, frame);
		}
		count++;
	}
	return true;
}

int main() {

	
	string videoPath = "../video/250���и߶�.MOV";
	//��ʼ������
	init();
	//����Ƶ�а�֡�����ȡͼƬ
	getImages(videoPath);

	vector<Mat> images;
	for (int i = 0; i < imgs_path.size();i++) {
		images.emplace_back(imread(imgs_path[i]));
	}
	

	int image_num = images.size();
	
	Ptr<FeaturesFinder> finder;
	//����ORB�㷨Ѱ��������
	finder = new OrbFeaturesFinder();
	vector<ImageFeatures> features(image_num);

	int64 tt = getTickCount();

	for (int i=0;i<images.size();i++)
	{
		Mat img = images[i];
 		(*finder)(img, features[i]);
 		features[i].img_idx = i;
 		cout << "Features in image #" << i + 1 << ": " << features[i].keypoints.size() << endl;

	}
	//�ͷ��ڴ�
	finder->collectGarbage();
	cout << "Finding features times:" << (getTickCount() - tt) / getTickFrequency() << "sec" << endl;
	 
	//������ƥ��
	vector<MatchesInfo> pair_matches;
	BestOf2NearestMatcher matcher(true,0.3f);
	matcher(features, pair_matches);
	matcher.collectGarbage();
	cout << pair_matches[0].H << endl;

	return 0;
}