#include "FuncDeclaration.h"
#include "MeanShift.h"
extern Mat colorImg;
extern Mat colorSegment;

vector <Mat> separateRegions;
vector <Mat> fillRegions;
vector<Scalar> colorValue;
vector<Scalar> colorPalette;
vector<vector<int>> colorIndexes;

bool colorDefine = false;


void ColorRefinement(Mat & src){
	int morph_elem = 0;
	int morph_size = 2;
	int open_operator = 2;
	int close_operator = 3;

	Mat element = getStructuringElement(morph_elem, Size(2 * morph_size + 1, 2 * morph_size + 1), Point(morph_size, morph_size));
	// Apply the specified morphology operation
	morphologyEx(src, src, close_operator, element);
	morphologyEx(src, src, open_operator, element);
	morphologyEx(src, src, open_operator, element);
}
vector<string> split(string str, char delimiter) {
	vector<string> internal;
	stringstream ss(str); // Turn the string into a stream.
	string tok;
	while (getline(ss, tok, delimiter)) {
		internal.push_back(tok);
	}
	return internal;
}
void ColorRead(){
	ifstream file("fillRegions/colorDefine.txt");
	string str;
	while (getline(file, str))
	{
		vector<string> sep = split(str, ' ');
		float r = stof(sep[0]);
		float g = stof(sep[1]);
		float b = stof(sep[2]);
		colorPalette.push_back(Scalar(r, g, b));
		vector<int> initial(0);
		colorIndexes.push_back(initial);
	}
}
void BGRtoYUV(Scalar & bgr){
	float b = float(bgr[0]);
	float g = float(bgr[1]);
	float r = float(bgr[2]);

	float y = 0.114*b + 0.587*g + 0.299*r;
	float u = 0.436*b - 0.28886*g - 0.14713*r;
	float v = -0.10001*b - 0.51499*g + 0.615*r;
	bgr = Scalar(y, u, v);
}
void BGRtoXYZ(Scalar & bgr){
	float b = float(bgr[0]/ 255.0);
	float g = float(bgr[1] / 255.0);
	float r = float(bgr[2] / 255.0);

	float X = 0.180423*b + 0.357580*g + 0.412453*r;
	float Y = 0.072169*b + 0.715160*g + 0.212671*r;
	float Z = 0.950227*b + 0.119193*g + 0.019334*r;
	bgr = Scalar(X, Y, Z);
}
float f(float t){
	if (t > 0.008856)
		t = pow(t, 0.33);
	else
		t = 7.787*t + 16 / 116;
	return t;
}
void XYZtoLab(Scalar & xyz){
	float x = float(xyz[0]);
	float y = float(xyz[1]);
	float z = float(xyz[2]);
	x /= 0.950456;
	z /= 1.088754;
	float L;
	if (y > 0.008856)
		L = 116 * pow(y, 0.33) - 16;
	else
		L = 903.3*y;

	float a = 500 * (f(x) - f(y));
	float b = 200 * (f(x) - f(y));
	xyz = Scalar(L,a,b);
}


void ColorRegistration(Scalar & color, int regionIndex){
	BGRtoYUV(color);
	//BGRtoXYZ(color); XYZtoLab(color);
	float minDistance = INFINITY;
	int colorIndex = 0;
	
	for (int i = 0; i < colorPalette.size(); i++){
		Scalar colorMatch = colorPalette[i];
		BGRtoYUV(colorMatch);
		//BGRtoXYZ(colorMatch); XYZtoLab(colorMatch);
		float distance = norm(color, colorMatch);
		if (distance < minDistance){
			minDistance = distance;
			colorIndex = i;
			//cout << minDistance << " " << i << endl;
		}
	}
	color = colorPalette[colorIndex];
	colorIndexes[colorIndex].push_back(regionIndex);
}


void ColorSeparation(){
	ColorRead();

	cout << "Separating color regions ..." << endl;
	// Bilateral Filtering
	Mat bilateralFilteredImg;
	bilateralFilter(colorImg, bilateralFilteredImg, 20, 100, 50);
	if(colorDefine) bilateralFilteredImg = colorImg;
	imwrite("Bilateral Image.jpg", bilateralFilteredImg);
	colorSegment = Mat(colorImg.size(), CV_8UC3, Scalar(255, 255, 255));
	IplImage* img = cvCloneImage(&(IplImage)colorImg);

	// Mean shifting
	int **ilabels = new int *[img->height];
	for (int i = 0; i < img->height; i++) ilabels[i] = new int[img->width];
	int regionNum = MeanShift(img, ilabels);
	cout << "Segent region number: " << regionNum << endl;
	vector <int> regionPixNum(regionNum);
	
	// Initial
	for (int i = 0; i < regionNum; i++){
		colorValue.push_back(Scalar(0, 0, 0));
		regionPixNum[i] = 0;
		separateRegions.push_back(Mat(colorImg.size(), CV_8UC3, Scalar(255, 255, 255)));
	}

	// Compute average color
	for (int i = 0; i < bilateralFilteredImg.rows; i++)
		for (int j = 0; j < bilateralFilteredImg.cols; j++)
		{
			int label = ilabels[i][j];
			colorValue[label] += Scalar(bilateralFilteredImg.at<Vec3b>(i, j)[0], bilateralFilteredImg.at<Vec3b>(i, j)[1], bilateralFilteredImg.at<Vec3b>(i, j)[2]);
			separateRegions[label].at<Vec3b>(i, j) = Vec3b(0, 0, 0);
			regionPixNum[label]++;
		}

	int backgroundIndex = 0;
	int maxPoints = 0;
	for (int i = 0; i < regionNum; i++){
		// Find background
		if (regionPixNum[i]>maxPoints) {
			backgroundIndex = i;
			maxPoints = regionPixNum[i];
		}
		colorValue[i] /= regionPixNum[i];
		//ColorRefinement(separateRegions[i]);
	}
	

	// Background Removal
	colorValue.erase(colorValue.begin() + backgroundIndex);
	//colorIndexes.erase(colorIndexes.begin() + backgroundIndex);
	separateRegions.erase(separateRegions.begin() + backgroundIndex);

	for (int i = 0; i < separateRegions.size(); i++){
		ColorRegistration(colorValue[i], i);
	}
	for (int i = 0; i < colorIndexes.size(); i++){
		fillRegions.push_back(Mat(colorImg.size(), CV_8UC3, Scalar(255, 255, 255)));
		/*for (int j = 0; j < colorIndexes[i].size(); j++){
			cout << colorIndexes[i][j] << " ";
		}
		cout << endl;*/
	}

	// Color Recovery
	ofstream outputFile;
	if (colorDefine){
		outputFile.open("fillRegions/colorDefine.txt");
	}

	for (int i = 0; i < colorIndexes.size(); i++){
		for (int j = 0; j < colorIndexes[i].size(); j++){
			Mat grayImg;
			cvtColor(separateRegions[colorIndexes[i][j]], grayImg, CV_RGB2GRAY);
			Mat binaryImg = grayImg < 128;
			Mat nonZeroCoordinates;
			findNonZero(binaryImg, nonZeroCoordinates);
			for (int k = 0; k < nonZeroCoordinates.total(); k++) {
				int x = nonZeroCoordinates.at<Point>(k).x;
				int y = nonZeroCoordinates.at<Point>(k).y;
				fillRegions[i].at<Vec3b>(y, x) = Vec3b(colorPalette[i][0], colorPalette[i][1], colorPalette[i][2]);
				//separateRegions[i].at<Vec3b>(y, x) = Vec3b(colorValue[i][0], colorValue[i][1], colorValue[i][2]);
				colorSegment.at<Vec3b>(y, x) = Vec3b(colorPalette[i][0], colorPalette[i][1], colorPalette[i][2]);
			}
			//cout << colorValue[i] << endl;
			if (colorDefine){
				outputFile << colorValue[i][0] << " " << colorValue[i][1] << " " << colorValue[i][2] << endl;
			}
		}
		if (colorIndexes[i].size() > 0){
			string fileName = outputFileName("fillRegions/fill", i + 1, ".jpg");
			imwrite(fileName, fillRegions[i]);
		}
	}
	if (colorDefine){
		outputFile.close();
	}
	
	imshow("", colorSegment); waitKey(0);
	imwrite("fillRegions/ColorSegment.jpg", colorSegment);
}