#include "FuncDeclaration.h"

extern Mat refineImage;
extern Mat sketchContours;
extern Mat fillAreas;
extern Mat portrait;
extern bool separateShow;


void Separation(){
	cout << "Image Separating..." << endl << endl;
	Mat temp = ~(refineImage + sketchContours);
	portrait = refineImage & sketchContours;
	fillAreas = ~(~refineImage - temp);

	imwrite("fillAreas.jpg", fillAreas);
	imwrite("portrait.jpg", portrait);
	if (separateShow){
		imshow("Skeching contour", sketchContours);
		imshow("Filling area", fillAreas);
		imshow("Final portriat", portrait);
		waitKey(0);
	}
	destroyAllWindows();
}