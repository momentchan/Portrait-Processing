#include "Main.h"
extern Point2f center;

void FaceDetection(){

	bool detectFace = false;
	Mat capture_frame;
	Mat grayscale_frame;
	Mat R1;
	Mat R2;
	char kbCmd = ' ';

	CascadeClassifier face_cascade;
	face_cascade.load("haarcascade_frontalface_alt.xml");

	//setup video capture device and link it to the first capture device
	VideoCapture captureDevice;
	captureDevice.open(0);

	//create a loop to capture and find faces
	Mat R1_color, R2_color;
	cout << "Press C to capture image..." << endl;

	while (!detectFace)
	{
		captureDevice >> capture_frame;

		//convert captured image to gray scale and equalize
		cvtColor(capture_frame, grayscale_frame, CV_RGB2GRAY);
		equalizeHist(grayscale_frame, grayscale_frame);

		vector<Rect> faces;
		face_cascade.detectMultiScale(grayscale_frame, faces, 1.1, 3, CV_HAAR_FIND_BIGGEST_OBJECT | CV_HAAR_SCALE_IMAGE, Size(30, 30));

		//draw a rectangle for all found faces in the vector array on the original image
		for (int i = 0; i < faces.size(); i++)
		{
			int width = faces[i].width;
			int height = faces[i].height;
			int x = faces[i].x;
			int y = faces[i].y;

			int xBound = width / 2;
			int yBound = height / 4;
			center = Point( width/2 + xBound, height/2 + yBound);

			Point pt1(x + width, y + height);
			Point pt2(x, y);
			
			R1 = grayscale_frame(Rect(x, y, width, height));
			R1_color = capture_frame(Rect(x, y, width, height));
			
			if (x - xBound > 0 && y - yBound > 0 && x - xBound + width * 2 < 640 && y - yBound + height * 2 < 480)
			{
				R2 = grayscale_frame( Rect(x - xBound, y - yBound, width * 2, height * 2) );
				R2_color = capture_frame( Rect(x - xBound, y - yBound, width * 2, height * 2) );
			}
			//Draw rectangle
			rectangle(capture_frame, pt1, pt2, cvScalar(0, 255, 0, 0), 1, 8, 0);
		}
		//print the output
		imshow("Face Detecting...", capture_frame);
		cvWaitKey(33);
		if (_kbhit())
			kbCmd = _getche();
		if (faces.size() == 0){
			kbCmd == ' ';
		}
		else {
			if (kbCmd == 'c')
				detectFace = true;
		}
			
		
	}
	destroyWindow("Face Detecting...");
	center *= 200.0/R1.cols;
	resize(R1, R1, Size(200, 200));
	resize(R1_color, R1_color, Size(200, 200));
	resize(R2, R2, Size(400, 400));
	resize(R2_color, R2_color, Size(400, 400));
	
	//Create a window to present the detecting results
	namedWindow("Face Region", CV_WINDOW_AUTOSIZE);
	imshow("Face Region", R1_color);
	namedWindow("Portrait Region", CV_WINDOW_AUTOSIZE);
	imshow("Portrait Region", R2_color);
	imwrite("R1.jpg", R1);
	imwrite("R2.jpg", R2);
	imwrite("R1_color.jpg", R1_color);
	imwrite("R2_color.jpg", R2_color);
	cvWaitKey(1000);
	destroyWindow("Face Region");
	destroyWindow("Portrait Region");
}