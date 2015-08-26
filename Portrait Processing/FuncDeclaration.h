#include "Main.h"

// Face Detection
void FaceDetection();

// Contour Extraction
void ContourExtraction();
void CannyThreshold(int, void*);
float MWCalculation(Moments, Point2f);
void CoherentLine(Mat, Mat &, double);
void thinning(const Mat&, Mat&);
void BranchPointRemoval(Mat &);


// Binarization
void ShadowGeneration();

//Simulation
void DrawSimulation();
void SketchSimulation();
void FillSimulation();
void ShadowSimulation();
void FindDrawPoints(int, int, int, int, Mat, Mat&, ofstream&, float, Vec3b, Point&,int);
void boundaryInitial(int&, int&, int&, int&, int);

// Utilities
bool CompareLength(vector<Point>, vector<Point>);
bool CompareBox(Rect, Rect);
void FindBlobs(const Mat &, vector < vector<Point2i>> &, Mat &);
void ConnectedComponent(const Mat, vector<vector<Point2i>>&);
Rect BoundingBox(Mat);
Mat HoleFilling(Mat);
string int2str(int &i);
string outputFileName(string, int,string);
int ConnectedComponentNumber(const Mat, const Mat);
Mat FindLargestRegion(const Mat img);
void imageRefinement(Mat &);


int ContourSmooth();
void ColorSeparation();
void ColorDefinement();


