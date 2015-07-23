#include "FuncDeclaration.h"

extern Mat R2;
extern Mat refineImage;
extern vector<vector<Point> > filteredContours;

Mat simulation;
vector < vector<Point2i > > blobs;
vector<Rect> boundRects;
int fillLines = 0;
bool turn = false;

void DrawSimulation(){
	SketchSimulation();
	FillSimulation();
}
void SketchSimulation(){
	sort(filteredContours.begin(), filteredContours.end(), CompareLength);
	simulation = Mat(R2.size(), CV_8UC3, Scalar(255, 255, 255));

	ofstream outputFile;
	for (int i = 0; i < filteredContours.size(); i++) {
		cout << "Length of " << i + 1 << " contour: " << arcLength(filteredContours[i], false) << endl;

		string fileName = outputFileName("drawPoints/sketch", i, ".txt");
		outputFile.open(fileName);

		for (int j = 0; j < filteredContours[i].size()-1; j++){
		//for (int j = 0; j < filteredContours[i].size() * 1 / 2; j++){
			outputFile << filteredContours[i][j] << endl;
			line(simulation, filteredContours[i][j], filteredContours[i][j + 1], Scalar(0, 0, 0), 2);
			//imshow("Drawing Simulation", simulation);
			//cvWaitKey(10);
		}
		outputFile.close();
	}
	cvWaitKey(0);
}
void FillSimulation(){

	ConnectedComponent(refineImage, blobs);

	//Find connected components and construct Bounding box
	vector <Mat> fillRegions;
	for (int i = 0; i < blobs.size(); i++) {
		//cout << "Number of points in region " << i + 1 << " : " << blobs[i].size() << endl;
		
		Mat fillRegion = Mat::zeros(R2.size(), CV_8U);

		for (int j = 0; j < blobs[i].size(); j++) {
			int x = blobs[i][j].x;
			int y = blobs[i][j].y;
			fillRegion.at<uchar>(y, x) = 255;
		}
		// Perform hole filling to remove unwanted holes
		//fillRegion = HoleFilling(fillRegion);
		fillRegion = ~fillRegion;
		fillRegions.push_back(fillRegion);

		string fileName = outputFileName("fillRegions/fill", i+1, ".jpg");
		//imwrite(fileName, fillRegion);
		boundRects.push_back(BoundingBox(fillRegion));
		rectangle(fillRegion, boundRects[i].tl(), boundRects[i].br(), 128, 2, 8, 0);
		imwrite(fileName, fillRegion);
	}

	
	//Filling regions
	for (int i = 0; i < blobs.size(); i++) {

		// Boundary initialization
		int ys = boundRects[i].tl().y;
		int ye = boundRects[i].br().y;
		int xs = boundRects[i].tl().x;
		int xe = boundRects[i].br().x;
		boundaryInitial(ys, ye, xs, xe, 5);
		
				
		float size = boundRects[i].height;
		if (boundRects[i].height < boundRects[i].width) size = boundRects[i].width;
		
		int rows = ye - ys;
		int gap = rows*0.01;
		if (gap < 3) gap = 3;

		ofstream outputFile;
		string fileName = outputFileName("drawPoints/fill", i, ".txt");
		outputFile.open(fileName);
		Point previousPoint;
		
		for (int j = ys; j <= ye; j = j + gap)
			FindDrawPoints(j, xs, ys, xe, fillRegions[i], simulation, outputFile, size, previousPoint);
		// Last Row
		for (int k = xs; k <= xe; k = k + gap)
			FindDrawPoints(ye, k, ys, xe, fillRegions[i], simulation, outputFile, size, previousPoint);
		outputFile.close();
	}

	cout << "\nTotal Number of fill lines: " << fillLines << endl<<endl;
	cvWaitKey(0);
}

void FindDrawPoints(int y, int x, int ys, int xe, Mat fill_region, Mat & fill, ofstream & outputFile, float size, Point & previousPoint){
	vector<Point> points;
	
	/**/
	//Initial starting points
	/*
	if ((int)fill_region.at<uchar>(y, x) < 128)
		points.push_back(Point(x, y));
	else{
		while (y > ys && x < xe){
			y = y - 1;
			x = x + 1;
			if ((int)fill_region.at<uchar>(y, x) < 128){
				points.push_back(Point(x, y));
				break;
			}
		}
	}
	

	int y = y - 1;
	int x = x + 1;
	bool cross = false;
	*/
	bool cross = true;
	if (y < ys) y = ys;
	if (x > xe) x = xe;

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
	if(points.size()%2!=0)  // add the boundary point
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
				line(fill, points[2 * i], points[2 * i + 1], Scalar(0, 0, 0), 3);
				previousPoint = points[2 * i + 1];
				outputFile << points[2 * i] << points[2 * i + 1] << endl;
				fillLines++;
				imshow("Drawing Simulation", fill);
				waitKey(10);
			}
	}
	else {
		for (int i = num-1; i >= 0; i--)
			if (norm(points[2 * i] - points[2 * i + 1]) > size*0.05)
			{
				//cout <<size<<" "<< norm(points[2 * i] - points[2 * i + 1]) << endl;
				line(fill, points[2 * i + 1], points[2 * i], Scalar(0, 0, 0), 3);
				previousPoint = points[2 * i];
				outputFile << points[2 * i + 1] << points[2 * i] << endl;
				fillLines++;
				imshow("Drawing Simulation", fill);
				waitKey(10);
			}
	}
	turn = !turn;
}
void boundaryInitial(int &ys, int &ye, int &xs, int &xe, int range){
	for (int i = 0; i < range; i++){
		if (ys > 0) ys--;
		if (ye < 400-range) ye++;
		else ye = 400 - range;
		if (xs > 0) xs--;
		if (xe < 400 - range) xe++;
		else xe = 400 - range;
	}
}

