#include "Main.h"
#include "FuncDeclaration.h"

//Variables for face detection
Mat colorImg;
Mat grayImg;
Mat colorSegment;
Point2f faceCenter;
vector<vector<Point> > filteredContours;

Mat fillAreas;
Mat portrait;

//Variables for showing images
bool contourShow = 1;
void Translation();

int main(int argc, const char** argv)
{
	//Tracking and detecting face
	//FaceDetection();
	//system("cls");
	
	colorImg = imread("colorImg.jpg");
	cvtColor(colorImg, grayImg, CV_RGB2GRAY);
	faceCenter = Point2f(200, 200);
	ContourExtraction();
	ColorSeparation();
	DrawSimulation();
	
	////////Translation
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
