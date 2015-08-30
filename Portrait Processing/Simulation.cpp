#include "FuncDeclaration.h"

extern Mat colorImg;
extern Mat shadowImg;
extern Mat colorSegment;

extern vector <Scalar> colorValue;
extern vector<Scalar> colorPaletteRGB;
extern vector <Mat> fillRegions;
extern vector<vector<Point> > filteredContours;
extern vector<vector<int>> colorIndexes;


Mat humanPortrait;

int fillLines = 0;
bool turn = false;

void DrawSimulation(){
	humanPortrait = Mat(colorImg.size(), CV_8UC3, Scalar(255, 255, 255));
	FillSimulation();
	SketchSimulation();
	ShadowSimulation();
	imwrite("SimulationResult.jpg", humanPortrait);
}
void SketchSimulation(){
	RNG rng(12345);
	sort(filteredContours.begin(), filteredContours.end(), CompareLength);
	ofstream outputFile;
	for (int i = 0; i < filteredContours.size(); i++) {
		cout << "Length of " << i + 1 << " contour: " << arcLength(filteredContours[i], false) << endl;

		string fileName = outputFileName("drawPoints/sketch", i, ".txt");
		outputFile.open(fileName);
		Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
		for (int j = 0; j < filteredContours[i].size()-1; j++){
		//for (int j = 0; j < filteredContours[i].size() * 1 / 2; j++){
			
			outputFile << filteredContours[i][j].x << " " << filteredContours[i][j].y << endl;
			line(humanPortrait, filteredContours[i][j], filteredContours[i][j + 1], Scalar(0, 0, 0), 2);
			//line(humanPortrait, filteredContours[i][j], filteredContours[i][j + 1], color, 2);
			//circle(humanPortrait, filteredContours[i][j], 3, Scalar(0, 0, 0), 0, CV_AA);
			imshow("Drawing Simulation", humanPortrait);
			waitKey(10);
		}
		//waitKey(0);
		outputFile.close();
	}
	waitKey(0);
}
void FillSimulation(){
	int fillRegionsNum = 0;
	//Filling regions
	for (int i = 0; i < fillRegions.size(); i++) {
		if (colorIndexes[i].size()>0){

			// Boundary initialization
			int ys = 1;
			int ye = colorImg.rows - 1;
			int xs = 1;
			int xe = colorImg.cols - 1;
			Mat fillRgionBlack;
			cvtColor(fillRegions[i], fillRgionBlack, CV_RGB2GRAY);
			fillRgionBlack = fillRgionBlack > 245;

			float size = 0;
			//float size = boundRects[i].height;
			//if (boundRects[i].height < boundRects[i].width) size = boundRects[i].width;

			//int rows = ye - ys;
			//int gap = rows*0.01;
			//if (gap < 3) gap = 3;
			int gap = 5;
			int lineWidth = 3;
			ofstream outputFile;
			string fileName = outputFileName("drawPoints/fill", fillRegionsNum, ".txt");
			outputFile.open(fileName);
			Point previousPoint;
			Vec3b fillColor = Vec3b(colorPaletteRGB[i][0], colorPaletteRGB[i][1], colorPaletteRGB[i][2]);

			// Write indexing of color
			outputFile << i << endl;

			//outputFile << colorIndexes[i] << endl;
			// Find draw points
			for (int j = ys; j <= ye; j = j + gap)
				FindDrawPoints(j, xs, ys, xe, fillRgionBlack, humanPortrait, outputFile, size, fillColor, previousPoint, lineWidth);
			// Last Row
			for (int k = xs; k <= xe; k = k + gap)
				FindDrawPoints(ye, k, ys, xe, fillRgionBlack, humanPortrait, outputFile, size, fillColor, previousPoint, lineWidth);
			outputFile.close();


			// For oid covering
			gap = 7;
			fillColor = Vec3b(colorPaletteRGB[i][0], colorPaletteRGB[i][1], colorPaletteRGB[i][2]);//Vec3b(255, 255, 255);//
			fileName = outputFileName("drawPoints/oil", fillRegionsNum, ".txt");
			outputFile.open(fileName);

			// Write indexing of color
			outputFile << i << endl;

			//outputFile << colorIndexes[i] << endl;
			// Find draw points
			for (int j = ys; j <= ye; j = j + gap)
				FindDrawPoints(j, xs, ys, xe, fillRgionBlack, humanPortrait, outputFile, size, fillColor, previousPoint, 0);
			// Last Row
			for (int k = xs; k <= xe; k = k + gap)
				FindDrawPoints(ye, k, ys, xe, fillRgionBlack, humanPortrait, outputFile, size, fillColor, previousPoint, 0);
			outputFile.close();


			fillRegionsNum++;
		}
	}
	cout << "\nTotal Number of fill lines: " << fillLines << endl << endl;
	waitKey(0);
}

void ShadowSimulation(){
	fillLines = 0;
	imageRefinement(shadowImg);
	vector<vector<Point2i>> blobs;
	ConnectedComponent(shadowImg, blobs);
	vector<Mat> shadowRegions;
	cout << blobs.size() << endl;

	for (int i = 0; i < blobs.size(); i++){
		if (blobs[i].size() < 50)
			break;
		
		Mat shadowRegion = Mat(shadowImg.size(), CV_8UC1);
		shadowRegion.setTo(255);
		//cout << blobs[i].size() << endl;
		for (int j = 0; j < blobs[i].size(); j++){
			int x = blobs[i][j].x;
			int y = blobs[i][j].y;
			shadowRegion.at<uchar>(y, x) = 0;
		}
		shadowRegions.push_back(shadowRegion);
		//imshow("", shadowRegion); waitKey(0);
	}

	for (int i = 0; i < shadowRegions.size(); i++){  // i=1 skip hair
		// Boundary initialization
		int ys = 1;
		int ye = colorImg.rows - 1;
		int xs = 1;
		int xe = colorImg.cols - 1;
	
		float size = 0;
		int gap = 3;
		int lineWidth = 1;
		ofstream outputFile;
		string fileName = outputFileName("drawPoints/shadow", i, ".txt");
		outputFile.open(fileName);
		Point previousPoint;
		Vec3b fillColor = Vec3b(40, 40, 40);// 52.5873, 87.5418, 111.205); // color39
		
		// Find draw points
		for (int j = ys; j <= ye; j = j + gap)
			FindDrawPoints(j, xs, ys, xe, shadowRegions[i], humanPortrait, outputFile, size, fillColor, previousPoint, lineWidth);
		// Last Row
		for (int k = xs; k <= xe; k = k + gap)
			FindDrawPoints(ye, k, ys, xe, shadowRegions[i], humanPortrait, outputFile, size, fillColor, previousPoint, lineWidth);
		outputFile.close();

	}
	cout << "\nTotal Number of fill lines: " << fillLines << endl << endl;
	waitKey(0);
}



void FindDrawPoints(int y, int x, int ys, int xe, Mat fill_region, Mat & fill, ofstream & outputFile, float size, Vec3b fillColor, Point & previousPoint, int lineWidth){
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
			
			line(fill, points[2 * i], points[2 * i + 1], fillColor, lineWidth);
			previousPoint = points[2 * i + 1];
			outputFile << points[2 * i].x << " " << points[2 * i].y << " " << points[2 * i + 1].x << " " << points[2 * i + 1].y << endl;
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
			line(fill, points[2 * i + 1], points[2 * i], fillColor, lineWidth);
			previousPoint = points[2 * i];
			outputFile << points[2 * i + 1].x << " " << points[2 * i + 1].y << " " << points[2 * i].x << " " << points[2 * i].y << endl;
			fillLines++;
			imshow("Drawing Simulation", fill);
			waitKey(10);
		}
	}
	turn = !turn;
}

