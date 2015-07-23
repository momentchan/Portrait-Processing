#include "FuncDeclaration.h"

extern Mat binaryImage;
extern Mat R1;
extern Mat R2;
extern bool binaryShow;
float alpha = 0.23;

void Binarization(){
	cout << "Image binarizing..." << endl;

	Mat histImage;
	int T = HistogramCalulation(R1, histImage, alpha);
	cout << "Threshold for binarization: " << T << endl << endl;
	threshold(R2, binaryImage, T, 255, CV_THRESH_BINARY);

	if (binaryShow){
		imshow("Intensity Histogram", histImage);
		imshow("Binarization", binaryImage);
	}
	imwrite("binaryImage.jpg", binaryImage);
	waitKey(0);
	destroyAllWindows();
}
