#include "Main.h"
extern Point2f faceCenter;

void FaceDetection()
{

	bool detected = false;
	Mat captureFrame;
	Mat bottomFrame;
	Mat grayscaleFrame;
	Mat colorImg;
	Mat grayImage;
	char kbCmd = ' ';

	CascadeClassifier faceCascade;
	faceCascade.load("haarcascade_frontalface_alt.xml");
	//setup video capture device and link it to the first capture device
	VideoCapture captureDevice;
	captureDevice.open(0);

	//create a loop to capture and find faces
	cout << "Press C to capture image..." << endl;
	while (!detected)
	{
		captureDevice >> captureFrame;
		bottomFrame = captureFrame.clone();

		//convert captured image to gray scale and equalize
		cvtColor(captureFrame, grayscaleFrame, CV_RGB2GRAY);
		equalizeHist(grayscaleFrame, grayscaleFrame);

		vector<Rect> detectedFaces;
		faceCascade.detectMultiScale(grayscaleFrame, detectedFaces, 1.1, 3, CV_HAAR_FIND_BIGGEST_OBJECT | CV_HAAR_SCALE_IMAGE, Size(30, 30));

		//draw a rectangle for all found faces in the vector array on the original image
		for (int i = 0; i < detectedFaces.size(); i++)
		{
			int x = detectedFaces[i].x;
			int y = detectedFaces[i].y;
			int w = detectedFaces[i].width;
			int h = detectedFaces[i].height;
			
			int xBound = w/2;
			int yBound = h/2;
			faceCenter = Point( xBound + w/2, yBound + h/2 );

			Point pt1(x + w, y + h);
			Point pt2(x, y);
			
			// Make sure the boundary in image range
			if (x - xBound > 0 && y - yBound > 0 && x - xBound + w * 2 < 640 && y - yBound + h * 2 < 480)
			{
				grayImage = grayscaleFrame( Rect(x - xBound, y - yBound, w * 2, h * 2) );
				colorImg = bottomFrame( Rect(x - xBound, y - yBound, w * 2, h * 2) );
			}
			//Draw rectangle
			rectangle(captureFrame, pt1, pt2, cvScalar(0, 255, 0, 0), 1, 8, 0);
		}

		//print the output
		imshow("Face Detecting...", captureFrame);
		waitKey(33);
		if (_kbhit())
			kbCmd = _getche();
		if (detectedFaces.size() == 0){
			kbCmd == ' ';
		}
		else {
			if (kbCmd == 'c')
				detected = true;
		}
	}
	
	destroyWindow("Face Detecting...");
	faceCenter *= 400.0 / colorImg.cols;
	resize(colorImg, colorImg, Size(400, 400));
	
	//Create a window to present the detecting results
	namedWindow("Portrait Region", CV_WINDOW_AUTOSIZE);
	imshow("Portrait Region", colorImg); waitKey(0);
	destroyWindow("Portrait Region");

	imwrite("colorImg.jpg", colorImg);
}