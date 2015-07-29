#include "FuncDeclaration.h"
#include "MeanShift.h"
extern Mat R2color;
extern Mat colorSegment;

void ColorSeparation(){

	// Bilateral Filtering
	
	//bilateralFilter(R2color, colorSegment, 20, 100, 50);
	colorSegment = R2color;
	IplImage* img = cvCloneImage(&(IplImage)colorSegment);

	// Mean shift
	int **ilabels = new int *[img->height];
	for (int i = 0; i < img->height; i++) ilabels[i] = new int[img->width];
	int regionCount = MeanShift(img, ilabels);
	cout << "regionCount " << regionCount << endl;
	vector<Scalar> color(regionCount);
	vector<int> regionPixNum(regionCount);

	// Initial
	for (int i = 0; i < regionCount; i++){
		color[i] = Scalar(0, 0, 0);
		regionPixNum[i] = 0;
	}

	// Color recovery
	for (int i = 0; i < colorSegment.rows; i++)
		for (int j = 0; j < colorSegment.cols; j++)
		{
			int cl = ilabels[i][j];
			color[cl] += Scalar(colorSegment.at<Vec3b>(i, j)[0], colorSegment.at<Vec3b>(i, j)[1], colorSegment.at<Vec3b>(i, j)[2]);
			regionPixNum[cl]++;
		}
	for (int i = 0; i < regionCount; i++){
		//cout << regionPixNum[i] << " ";
		//cout << color[i] << " ";
		color[i] /= regionPixNum[i];
	}
	for (int i = 0; i < colorSegment.rows; i++)
		for (int j = 0; j < colorSegment.cols; j++)
		{
			int cl = ilabels[i][j];
			colorSegment.at<Vec3b>(i, j) = Vec3b(color[cl][0], color[cl][1], color[cl][2]);
		}

	imshow("", colorSegment); 
	waitKey(0);

	cvNamedWindow("MeanShift", CV_WINDOW_AUTOSIZE);
	cvShowImage("MeanShift", img);

	cvWaitKey();
	cvDestroyWindow("MeanShift");
	cvReleaseImage(&img);




	//unsigned long AAtime = 0;
	//AAtime = getTickCount();

	//R1 = imread("R1.jpg", CV_RGB2GRAY);
	//R2 = imread("R2.jpg", CV_RGB2GRAY);
	//Mat R2color = imread("R2_color.jpg");
	//resize(R2color, R2color, Size(400, 400));
	//
	//bilateralFilter(R2color, R2bilateral, 20, 100, 50);

	//pyrMeanShiftFiltering(R2bilateral, cartoon, 50, 30, 3 );
	//ProccTimePrint(AAtime, "cpu");


	//imshow("origin", R2color);
	//imshow("bilateral", R2bilateral); 
	//imshow("cartoon Filter cpu", cartoon);
	//waitKey(0);
}