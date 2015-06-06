#include "Main.h"
#include "FuncDeclaration.h"

//Variables for face detection
Mat R1;
Mat R2;
Point2f center;
vector<vector<Point> > filteredContours;

Mat binaryImage;
Mat refineImage;

Mat fillAreas;
Mat sketchContours;
Mat portrait;

//Variables for showing images
bool contourShow = 1;
bool binaryShow = 1;
bool refineShow = 1;
bool separateShow = 1;

int main(int argc, const char** argv)
{
	//Tracking and detecting face
	//FaceDetection();
	//system("cls");
	
	R1 = imread("R1.jpg", CV_RGB2GRAY);
	R2 = imread("R2.jpg", CV_RGB2GRAY);
	cvtColor(R1, R1, CV_RGB2GRAY);
	cvtColor(R2, R2, CV_RGB2GRAY);
	center = Point2f(200, 150);
	
	ContourExtraction();
	Binarization();
	Refinement();
	Separation();
	DrawSimulation();
}



