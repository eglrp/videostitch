#include<iostream>
#include <math.h>
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
#include <QDir>
#include <QString>

using namespace std;
using namespace cv;
using namespace detail;

//��ƵĿ¼
string videoPath = "../video/250���и߶�.MOV";
//ͼƬ�洢·��
const string path = "../temp/";
//����·��
vector<String> imgs_path;
//ѡȡ֡���������ƥ������----������ٶ��йأ��ٶ�Խ�죬����Խ��֡���ԽС��
int matchKeyPointNum_Min = 450;
//֡���
int frameInterval;

//��ʼ����׼������Ŀ¼
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

//��ȡ����ͼƬ��������ƥ����
int findMatchPoints(Mat src_1,Mat src_2) {

	Ptr<FeaturesFinder> finder;
	//����ORB�㷨Ѱ��������
	finder = new OrbFeaturesFinder();
	vector<ImageFeatures> features(2);
	
	Mat grayP1, grayP2;
	vector<Mat> images_gray;
	//�ҶȻ�
	cvtColor(src_1, grayP1, CV_BGR2GRAY);
	images_gray.emplace_back(grayP1);
	cvtColor(src_2, grayP2, CV_BGR2GRAY);
	images_gray.emplace_back(grayP2);
	
	//Ѱ��������
	for (int i = 0; i < images_gray.size(); i++)
	{
		Mat img = images_gray[i];
		(*finder)(img, features[i]);
		features[i].img_idx = i;
		cout << "Features in image #" << i + 1 << ": " << features[i].keypoints.size() << endl;
	}
	//�ͷ��ڴ�
	finder->collectGarbage();
	//������ƥ��
	vector<MatchesInfo> pair_matches;
	BestOf2NearestMatcher matcher(false);
	matcher(features, pair_matches);
	matcher.collectGarbage();
	cout << "ƥ������" << pair_matches[1].num_inliers << endl;
	return pair_matches[1].num_inliers;
}

//�Զ���ȡ���ʵ�֡���
void getFrameInterval() {

	Mat frame;
	size_t count = 0;
	
	cv::VideoCapture capture(videoPath);
	if (!capture.isOpened())
	{
		cout << "��Ƶ�ļ��޷��򿪣���";
		return ;
	}
	Mat src_1,src_2;

	while (capture.read(frame))
	{
		
		if (frame.empty())
			break;

		if (src_1.empty())
		{
			frame.copyTo(src_1);
			
		}
		else
		{
			if (count % 50 == 0) {
				frame.copyTo(src_2);
				
				int num = findMatchPoints(src_1, src_2);
				if (num <= matchKeyPointNum_Min)
				{
					frameInterval = count;
					cout << "���ʵ�֡�����" << count << endl;
					return;
				}
			}
		}
		count++;
	}


}

//������Ƶ��ȡͼ��
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
			cout << path_buf<<endl;
			imgs_path.emplace_back(path_buf);
			imwrite(path_buf, frame);
		}
		count++;
	}
	return true;
}

//ͼ��ƴ�ӣ���src_1ƴ�ӵ�src_2��
Mat stitchImage(Mat src_1, Mat src_2) {

	for (int i = 0; i < src_1.rows; i++) {
		uchar* row = src_1.ptr(i);
		uchar* row_dst = src_2.ptr(i);
		for (int j = 0; j < src_1.cols; j++) {

			if (row_dst[j * 3] == 0 && row_dst[j * 3 + 1] == 0 && row_dst[j * 3 + 2] == 0) {
				row_dst[j * 3] = row[j * 3];
				row_dst[j * 3 + 1] = row[j * 3 + 1];
				row_dst[j * 3 + 2] = row[j * 3 + 2];

			}
			else
			{
				row_dst[j * 3] = row_dst[j * 3];
				row_dst[j * 3 + 1] = row_dst[j * 3 + 1];
				row_dst[j * 3 + 2] = row_dst[j * 3 + 2];
			}
		}
	}
	return src_2;
}

int main() {

	//findMatchPoints(imread("D:/VS2015WorkSpace/OpenCV/VideoStitch/temp/600.jpg"), imread("D:/VS2015WorkSpace/OpenCV/VideoStitch/temp/3000.jpg"));

	cout << "-------��ʼ��ȡ��Ƶ����-----------"<<endl<<endl;
	int64 tt = getTickCount();
	//��ʼ������
	init();
	//�Զ���ȡ���ʵ�֡���
	getFrameInterval();
	//����Ƶ�а�֡�����ȡͼƬ
	getImages(videoPath);
	cout << "\n-------������ȡ��Ƶ����-----------" << endl;
	cout << "get pictures times:" << (getTickCount() - tt) / getTickFrequency() << " sec" << endl;
	//ԭͼ��
	vector<Mat> images;
	//����ͼ���С
	vector<Mat> images_resize;
	//�Ҷ�ͼ��
	vector<Mat> images_gray;
	//ƴ��ͼ��Ĵ�С
	const int images_num = imgs_path.size();

	tt = getTickCount();
	//����ͼƬλ��
#pragma omp parallel for
	for (int i = 0; i < imgs_path.size(); i++)
	{
		images.emplace_back(imread(imgs_path[i]));

		Mat src_p = images[i];
		Mat imag(2500, 2500, CV_8UC3, Scalar::all(0));
		Mat temp(imag, Rect(800, 800, src_p.cols, src_p.rows));
		src_p.copyTo(temp);
		images_resize.emplace_back(imag);
		cvtColor(imag, imag, CV_BGR2GRAY);
		images_gray.emplace_back(imag);

	}
	cout << "init times:" << (getTickCount() - tt) / getTickFrequency() << " sec" << endl;

	tt = getTickCount();
	//����ͼ��
	Mat dst;
	//ȡ��һ��ͼ��
	Mat src = images_resize[0];
	//ORB
	Ptr<ORB> orb = ORB::create();
#pragma omp parallel for
	for (int i = 1; i < images_num; i++)
	{
		//ͼ���������
		std::vector<KeyPoint> keypoints_src, keypoints_;
		//��������ͼ��������ӣ�������Mat����
		Mat descriptors_src, descriptors_;
		
		//��һ�������Oriented FAST�ǵ�λ��.
		orb->detect(src, keypoints_src);
		orb->detect(images_resize[i], keypoints_);
		
		//�ڶ��������ݽǵ�λ�ü���BRIEF������
		orb->compute(src, keypoints_src, descriptors_src);
		orb->compute(images_resize[i], keypoints_, descriptors_);

		//ƥ����Ϣ����
		vector<DMatch> matches;
		BFMatcher matcher(NORM_HAMMING);
		//������ƥ��
		matcher.match(descriptors_src, descriptors_, matches);
		//��ȡƥ���
		vector<Point2f> imagePoints1, imagePoints2;
		for (int j = 0; j < matches.size(); j++)
		{
			imagePoints1.push_back(keypoints_src[matches[j].queryIdx].pt);
			imagePoints2.push_back(keypoints_[matches[j].trainIdx].pt);

		}

		//����ƥ���õ�����ͼ��ı任����
		Mat H;
		H = findHomography(imagePoints1, imagePoints2, CV_RANSAC);
		//��ͼ�����͸�ӱ任, ���Ǳ���
		Mat outImg;
		warpPerspective(src, outImg, H, Size(2000, 2000));
		//ƴ��
		dst=stitchImage(outImg, images_resize[i]);
		
		//�������
		keypoints_src.clear();
		keypoints_.clear();
		imagePoints1.clear();
		imagePoints2.clear();
		src.setTo(0);
		//�����ƴ�ӽ����Ϊ�´�ƴ�ӵ���ʼ����
		dst.copyTo(src);
	}
	cout << "stitch picture times:" << (getTickCount() - tt) / getTickFrequency() << " sec" << endl;
	//����ת��
	//transpose(dst, dst);
	//��ת
	//flip(dst,dst,1);
	
	imwrite("res.jpg", dst);
	system("res.jpg");

}