#include "FuncDeclaration.h"
#include "MeanShift.h"
extern Mat colorImg;
extern Mat colorSegment;

vector <Mat> fillRegions;
vector<Scalar> colorValue;

void ColorRefinement(Mat & src){
	int morph_elem = 0;
	int morph_size = 2;
	int open_operator = 2;
	int close_operator = 3;

	Mat element = getStructuringElement(morph_elem, Size(2 * morph_size + 1, 2 * morph_size + 1), Point(morph_size, morph_size));
	// Apply the specified morphology operation
	morphologyEx(src, src, close_operator, element);
	morphologyEx(src, src, open_operator, element);
	morphologyEx(src, src, open_operator, element);
}

void ColorSeparation(){
	cout << "Separating color regions ..." << endl;
	// Bilateral Filtering
	Mat bilateralFilteredImg;
	bilateralFilter(colorImg, bilateralFilteredImg, 20, 100, 50);
	colorSegment = Mat(colorImg.size(), CV_8UC3, Scalar(255, 255, 255));
	IplImage* img = cvCloneImage(&(IplImage)colorImg);

	// Mean shifting
	int **ilabels = new int *[img->height];
	for (int i = 0; i < img->height; i++) ilabels[i] = new int[img->width];
	int regionNum = MeanShift(img, ilabels);
	cout << "Segent region number: " << regionNum << endl;
	vector <int> regionPixNum(regionNum);
	
	// Initial
	for (int i = 0; i < regionNum; i++){
		colorValue.push_back(Scalar(0, 0, 0));
		regionPixNum[i] = 0;
		fillRegions.push_back(Mat(colorImg.size(), CV_8UC3, Scalar(255, 255, 255)));
	}

	// Compute average color
	for (int i = 0; i < bilateralFilteredImg.rows; i++)
		for (int j = 0; j < bilateralFilteredImg.cols; j++)
		{
			int label = ilabels[i][j];
			colorValue[label] += Scalar(bilateralFilteredImg.at<Vec3b>(i, j)[0], bilateralFilteredImg.at<Vec3b>(i, j)[1], bilateralFilteredImg.at<Vec3b>(i, j)[2]);
			fillRegions[label].at<Vec3b>(i, j) = Vec3b(0, 0, 0);
			regionPixNum[label]++;
		}

	int backgroundIndex = 0;
	int maxPoints = 0;
	for (int i = 0; i < regionNum; i++){
		// Find background
		if (regionPixNum[i]>maxPoints) {
			backgroundIndex = i;
			maxPoints = regionPixNum[i];
		}
		colorValue[i] /= regionPixNum[i];
		ColorRefinement(fillRegions[i]);
	}

	// Background Removal
	colorValue.erase(colorValue.begin() + backgroundIndex);
	fillRegions.erase(fillRegions.begin() + backgroundIndex);


	// Color Recovery
	for (int i = 0; i < fillRegions.size(); i++){
		Mat grayImg;
		cvtColor(fillRegions[i], grayImg, CV_RGB2GRAY);
		Mat binaryImg = grayImg < 128;
		Mat nonZeroCoordinates;
		findNonZero(binaryImg, nonZeroCoordinates);
		for (int j = 0; j < nonZeroCoordinates.total(); j++) {
			int x = nonZeroCoordinates.at<Point>(j).x;
			int y = nonZeroCoordinates.at<Point>(j).y;
			fillRegions[i].at<Vec3b>(y, x) = Vec3b(colorValue[i][0], colorValue[i][1], colorValue[i][2]);
			colorSegment.at<Vec3b>(y, x) = Vec3b(colorValue[i][0], colorValue[i][1], colorValue[i][2]);
		}
		string fileName = outputFileName("fillRegions/fill", i + 1, ".jpg");
		imwrite(fileName, fillRegions[i]);
	}
	
	//imshow("", colorSegment); waitKey(0);
	//imwrite("fillRegions/ColorSegment.jpg", colorSegment);
}