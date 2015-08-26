#include "FuncDeclaration.h"
extern Mat grayImg;
extern Point2f faceCenter;
float alpha = 0.15;
Mat shadowImg;

int HistogramCalulation(Mat image){
	// allcoate memory for no of pixels for each intensity value
	int histogram[256];

	// initialize all intensity values to 0
	for (int i = 0; i < 255; i++)
		histogram[i] = 0;

	// calculate the no of pixels for each intensity values
	for (int y = 0; y < image.rows; y++)
	for (int x = 0; x < image.cols; x++)
		histogram[(int)image.at<uchar>(y, x)]++;

	// draw the histograms
	int hist_w = 512; int hist_h = 400;
	int bin_w = cvRound((double)hist_w / 256);

	Mat histImage(hist_h, hist_w, CV_8UC1, Scalar(255, 255, 255));

	// find the maximum intensity element from histogram
	int max_num = histogram[0];
	int max = 0;
	int total_num = image.size().area();
	int threshold = total_num*alpha;
	int accumulate = 0;
	int T;
	bool find = false;

	for (int i = 1; i < 256; i++){
		if (!find){
			if (accumulate < threshold){
				accumulate += histogram[i];
			}
			else {
				find = true;
				T = i - 1;
				accumulate -= histogram[i - 1];
			}
		}
		if (max_num < histogram[i]){
			max_num = histogram[i];
			max = i;
		}
	}
	//cout << threshold << endl << T << endl << accumulate << endl << endl;

	// normalize the histogram between 0 and histImage.rows
	for (int i = 0; i < 255; i++){
		histogram[i] = ((double)histogram[i] / max_num)*histImage.rows;
	}

	// draw the intensity line for histogram
	for (int i = 0; i < 255; i++)
		line(histImage, Point(bin_w*(i), hist_h), Point(bin_w*(i), hist_h - histogram[i]), Scalar(0, 0, 0), 1, 8, 0);

	// display histogram
	//imshow("Intensity Histogram", histImage);
	cvWaitKey();
	return T;
}


void ShadowGeneration(){
	int w = grayImg.cols;
	int T = HistogramCalulation(grayImg(Rect(faceCenter.x - w / 4, faceCenter.y - w / 4, w / 2, w / 2)));
	cout << "Shadow generation..." << endl;
	cout << "Threshold for binarization: " << T << endl << endl;
	Mat binaryImg;
	threshold(grayImg, binaryImg, T, 255, CV_THRESH_BINARY);
	shadowImg = Mat::zeros(grayImg.size(), CV_8U);
	for (int i = 0; i < binaryImg.cols;i++)
		for (int j = 0; j < binaryImg.rows; j++){
			if ((int)binaryImg.at<uchar>(i, j))
				shadowImg.at<uchar>(i, j) = 255;
		}
	imshow("Shadow Image", shadowImg);	waitKey(0);
	destroyAllWindows();
}
