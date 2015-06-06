#include "FuncDeclaration.h"

extern Mat binaryImage;
extern Mat refineImage;
extern bool refineShow;

int morph_elem = 0; //Rec
int morph_size = 2;
int max_morph_size = 10;
int open_operator = 2;
int close_operator = 3;


void Refinement(){
	cout << "Image Refining..." << endl << endl;
	// Create window
	if (refineShow){
		namedWindow("Refine Image", CV_WINDOW_AUTOSIZE);
		createTrackbar("Morph size:", "Refine Image", &morph_size, max_morph_size, RefineThreshold);
	}
	RefineThreshold(0, 0);
	waitKey(0);
	destroyAllWindows();
}
void RefineThreshold(int, void*){
	Mat element = getStructuringElement(morph_elem, Size(2 * morph_size + 1, 2 * morph_size + 1), Point(morph_size, morph_size));
	binaryImage.copyTo(refineImage);

	// Apply the specified morphology operation
	morphologyEx(refineImage, refineImage, close_operator, element);
	imwrite("refineImage_c1.jpg", refineImage);
	morphologyEx(refineImage, refineImage, open_operator, element);
	imwrite("refineImage_c1o1.jpg", refineImage);
	morphologyEx(refineImage, refineImage, open_operator, element);
	imwrite("refineImage_c1o2.jpg", refineImage);
	
	if (refineShow)
		imshow("Refine Image", refineImage);
}