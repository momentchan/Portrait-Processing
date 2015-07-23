#include "Main.h"

// Face Detection
void FaceDetection();

// Contour Extraction
void ContourExtraction();
void CannyThreshold(int, void*);
float MWCalculation(Moments, Point2f);
void CoherentLine(Mat,Mat&);


// Binarization
int HistogramCalulation(Mat image, Mat &, float);
void Binarization();
void Refinement();
void RefineThreshold(int, void*);

// Separation
void Separation();

//Simulation
void DrawSimulation();
void SketchSimulation();
void FillSimulation();
void FindDrawPoints(int, int, int, int, Mat, Mat&, ofstream&, float, Point&);
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

