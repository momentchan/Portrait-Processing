#include "FuncDeclaration.h"

extern Mat colorImg;
extern Mat colorSegment;
extern vector <Scalar> colorValue;
extern vector <Mat> fillRegions;
extern vector<vector<Point> > filteredContours;

Mat humanPortrait;
vector < vector<Point2i > > blobs;

int fillLines = 0;
bool turn = false;

void DrawSimulation(){
	humanPortrait = Mat(colorImg.size(), CV_8UC3, Scalar(255, 255, 255));
	FillSimulation();
	SketchSimulation();
}
void SketchSimulation(){
	sort(filteredContours.begin(), filteredContours.end(), CompareLength);
	ofstream outputFile;
	for (int i = 0; i < filteredContours.size(); i++) {
		cout << "Length of " << i + 1 << " contour: " << arcLength(filteredContours[i], false) << endl;

		string fileName = outputFileName("drawPoints/sketch", i, ".txt");
		outputFile.open(fileName);

		for (int j = 0; j < filteredContours[i].size()-1; j++){
		//for (int j = 0; j < filteredContours[i].size() * 1 / 2; j++){
			outputFile << filteredContours[i][j].x << " " << filteredContours[i][j].y << endl;
			line(humanPortrait, filteredContours[i][j], filteredContours[i][j + 1], Scalar(0, 0, 0), 2);
			//circle(humanPortrait, filteredContours[i][j], 1, Scalar(0, 0, 0), 0, CV_AA);
			imshow("Drawing Simulation", humanPortrait);
			waitKey(10);
		}
		//waitKey(0);
		outputFile.close();
	}
	waitKey(0);
}
void FillSimulation(){
	//Filling regions
	for (int i = 0; i < fillRegions.size(); i++) {
		// Boundary initialization
		int ys = 1;
		int ye = colorImg.rows-1;
		int xs = 1;
		int xe = colorImg.cols-1;
		Mat fillRgionBlack;
		cvtColor(fillRegions[i], fillRgionBlack, CV_RGB2GRAY);
		fillRgionBlack = fillRgionBlack>245;
				
		float size = 0;
		//float size = boundRects[i].height;
		//if (boundRects[i].height < boundRects[i].width) size = boundRects[i].width;
		
		//int rows = ye - ys;
		//int gap = rows*0.01;
		//if (gap < 3) gap = 3;
		int gap = 5;

		ofstream outputFile;
		string fileName = outputFileName("drawPoints/fill", i, ".txt");
		outputFile.open(fileName);
		Point previousPoint;
		Vec3b fillColor = Vec3b(colorValue[i][0], colorValue[i][1], colorValue[i][2]);
		
		// Find draw points
		for (int j = ys; j <= ye; j = j + gap)
			FindDrawPoints(j, xs, ys, xe, fillRgionBlack, humanPortrait, outputFile, size, fillColor, previousPoint);
		// Last Row
		for (int k = xs; k <= xe; k = k + gap)
			FindDrawPoints(ye, k, ys, xe, fillRgionBlack, humanPortrait, outputFile, size, fillColor, previousPoint);
		outputFile.close();
	}
	cout << "\nTotal Number of fill lines: " << fillLines << endl<<endl;
	waitKey(0);
}

void FindDrawPoints(int y, int x, int ys, int xe, Mat fill_region, Mat & fill, ofstream & outputFile, float size, Vec3b fillColor, Point & previousPoint){
	vector<Point> points;
	bool cross = true;

	//Find draw points
	while (y > ys && x < xe){		// Not touch boundary
		{
			if ((int)fill_region.at<uchar>(y, x) > 128 && !cross){
				points.push_back(Point(x - 1, y + 1));
				cross = true;
			}
			else if ((int)fill_region.at<uchar>(y, x) < 128 && cross){
				points.push_back(Point(x, y));
				cross = false;
			}
		}
		y = y - 1;
		x = x + 1;
	}
	if (points.size() % 2 != 0)  // add the boundary point
		points.push_back(Point(x, y));

	//Draw points
	//if (points.size() == 1)cout << points[0].y<<" "<<points[0].x << endl<<endl;
	int num = points.size() / 2;
	if (num > 0)
	if (!turn){
		for (int i = 0; i < num; i++)
			//if (norm(points[2 * i] - points[2 * i + 1])>size*0.05)
		{
			//cout <<size<<" "<< norm(points[2 * i] - points[2 * i + 1]) << endl;
			
			line(fill, points[2 * i], points[2 * i + 1], fillColor, 3);
			previousPoint = points[2 * i + 1];
			outputFile << points[2 * i] << points[2 * i + 1] << endl;
			fillLines++;
			imshow("Drawing Simulation", fill);
			waitKey(10);
		}
	}
	else {
		for (int i = num - 1; i >= 0; i--)
		if (norm(points[2 * i] - points[2 * i + 1]) > size*0.05)
		{
			//cout <<size<<" "<< norm(points[2 * i] - points[2 * i + 1]) << endl;
			line(fill, points[2 * i + 1], points[2 * i], fillColor, 3);
			previousPoint = points[2 * i];
			outputFile << points[2 * i + 1] << points[2 * i] << endl;
			fillLines++;
			imshow("Drawing Simulation", fill);
			waitKey(10);
		}
	}
	turn = !turn;
}

