#include "Main.h"
#include "FuncDeclaration.h"
#include "MeanShift.h"
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
Mat R2bilateral;
Mat cartoon;

//Variables for showing images
bool contourShow = 1;
bool binaryShow = 1;
bool refineShow = 1;	
bool separateShow = 1;
void Translation();

int main(int argc, const char** argv)
{
	Mat R2color = imread("R2_color.jpg");
	resize(R2color, R2color, Size(400, 400));
	bilateralFilter(R2color, R2bilateral, 20, 100, 50);
	IplImage* img = cvCloneImage(&(IplImage)R2bilateral);
	//IplImage *img = cvLoadImage("R2.jpg");

	// Mean shift
	int **ilabels = new int *[img->height];
	for (int i = 0; i<img->height; i++) ilabels[i] = new int[img->width];
	int regionCount = MeanShift(img, ilabels);
	cout << "regionCount " << regionCount << endl;
	vector<Scalar> color(regionCount);
	vector<int> regionPixNum(regionCount);
	

	for (int i = 0; i < regionCount; i++){
		color[i] = Scalar(0, 0, 0);
		regionPixNum[i] = 0;
	}
	
	// Color recovery
	for (int i = 0; i<R2bilateral.rows; i++)
	for (int j = 0; j<R2bilateral.cols; j++)
	{
		int cl = ilabels[i][j];
		color[cl] += Scalar(R2bilateral.at<Vec3b>(i, j)[0], R2bilateral.at<Vec3b>(i, j)[1], R2bilateral.at<Vec3b>(i, j)[2]);
		regionPixNum[cl]++;
	}
	for (int i = 0; i < regionCount; i++){
		//cout << regionPixNum[i] << " ";
		//cout << color[i] << " ";
		color[i] /= regionPixNum[i];
	}
	for (int i = 0; i<R2bilateral.rows; i++)
	for (int j = 0; j<R2bilateral.cols; j++)
	{
		int cl = ilabels[i][j];
		R2bilateral.at<Vec3b>(i, j) = Vec3b(color[cl][0], color[cl][1], color[cl][2]);
	}

	imshow("", R2bilateral); waitKey(0);

	cvNamedWindow("MeanShift", CV_WINDOW_AUTOSIZE);
	cvShowImage("MeanShift", img);

	cvWaitKey();

	cvDestroyWindow("MeanShift");

	cvReleaseImage(&img);

	////Tracking and detecting face
	////FaceDetection();
	////system("cls");

	//unsigned long AAtime = 0;
	//AAtime = getTickCount();

	//R1 = imread("R1.jpg", CV_RGB2GRAY);
	//R2 = imread("R2.jpg", CV_RGB2GRAY);
	//Mat R2color = imread("R2_color.jpg");
	//resize(R2color, R2color, Size(400, 400));
	//
	//bilateralFilter(R2color, R2bilateral, 20, 100, 50);

	//pyrMeanShiftFiltering(R2bilateral, cartoon, 50, 30, 3 );
	//ProccTimePrint(AAtime, "cpu");


	//imshow("origin", R2color);
	//imshow("bilateral", R2bilateral); 
	//imshow("cartoon Filter cpu", cartoon);
	//waitKey(0);
	//
	////cvtColor(R1, R1, CV_RGB2GRAY);
	//cvtColor(R2, R2, CV_RGB2GRAY);
	//center = Point2f(200, 200);
	//
	//ContourExtraction();

	///////*
	/////*Binarization();
	////Refinement();
	////Separation();*/
	//DrawSimulation();
	//////*/
	//////Translation();

	//
}


void Translation(){
	int sketchNumbers = 0;
	float paperSize = 20; //cm
	float imageSize = 400;
	float centerX = 5;


	vector<vector<Point3f>> sketch_pos; // (20, y , z) cm

	//count how many files in drawPoints directory
	WIN32_FIND_DATA fd;
	HANDLE h = FindFirstFile(TEXT("drawPoints/sketch*.txt"), &fd);
	if (h != INVALID_HANDLE_VALUE) {
		do {
			sketchNumbers++;
		} while (FindNextFile(h, &fd));
		FindClose(h);
	}

	for (int i = 0; i<sketchNumbers; i++){
		string file_num = int2str(i);
		string file_name = "drawPoints/sketch";
		file_name.append(file_num);
		file_name.append(".txt");
		ifstream file(file_name);
		string str;
		vector <Point3f> sub_sketch_pos;
		while (getline(file, str))
		{
			size_t pos = str.find(",");
			float y = (stoi(str.substr(1, pos - 1)) - 200)*(float)(paperSize / imageSize)/100 + 0.4;
			float z = (stoi(str.substr(pos + 2, str.length() - pos - 3)) - 200)*float(paperSize / imageSize)/100;
			sub_sketch_pos.push_back(Point3f(0.2,y, z));
		}
		sketch_pos.push_back(sub_sketch_pos);
	}
	ofstream outputFile;
	for (int i = 0; i < sketch_pos.size(); i++) {
		string fileName = outputFileName("translatePoints/sketch", i, ".txt");
		outputFile.open(fileName);

		for (int j = 0; j < sketch_pos[i].size() - 1; j++){
			outputFile << sketch_pos[i][j] << endl;
		}
		outputFile.close();
	}
}
