#include "FuncDeclaration.h"
#include "smootour.h"
void erode_and_dilate(Mat mat, bool erode = true, bool dilate = true) {
	if (erode) {
		int erosion_type = MORPH_RECT;
		int erosion_size = 3;

		Mat erode_element = getStructuringElement(erosion_type,
			Size(2 * erosion_size + 1, 2 * erosion_size + 1),
			Point(erosion_size, erosion_size));
		cv::erode(mat, mat, erode_element);
	}

	if (dilate) {
		int dilation_type = MORPH_RECT;
		int dilation_size = 5;

		Mat dilate_element = getStructuringElement(dilation_type,
			Size(2 * dilation_size + 1, 2 * dilation_size + 1),
			Point(dilation_size, dilation_size));
		cv::dilate(mat, mat, dilate_element);
	}
}

Mat get_frame(VideoCapture cap) {
	const int SCALE_DOWN = 2;
	Mat frame;
	cap >> frame;
	Mat scaled_frame(frame.rows / SCALE_DOWN, frame.cols / SCALE_DOWN, CV_8UC4);
	resize(frame, scaled_frame, Size(frame.cols / SCALE_DOWN, frame.rows / SCALE_DOWN), 1.0 / SCALE_DOWN, 1.0 / SCALE_DOWN);

	return scaled_frame;
}


int ContourSmooth(){
	cout << "smootour test running...!\n";

	//image processing
	VideoCapture cap(0);
	if (!cap.isOpened()) {
		cout << "failed to connect to camera \n";
		return -1;
	}
	Mat frame;
	frame = get_frame(cap); //get first frame for sizing.
	cout << "frame size: " << frame.cols << "," << frame.rows << "\n";

	Mat thresholded(frame.rows, frame.cols, CV_8UC1);

	//window display initialization
	namedWindow("smootour-test", CV_WINDOW_NORMAL);
	Mat windowFrame = Mat::zeros(frame.rows * 2, frame.cols * 2, CV_8UC3);
	Rect frameROI(0, 0, frame.cols, frame.rows);

	Mat thresholdedFrame(frame.rows, frame.cols, CV_8UC3);
	Rect tresholdedROI(frame.cols, 0, frame.cols, frame.rows);

	Mat implicitFrame(frame.rows, frame.cols, CV_8UC3);
	Rect implicitROI(0, frame.rows, frame.cols, frame.rows);

	//initialize with correct row and column size
	//TODO: could also initialize with the first binary image.
	Smootour frame_smootour(frame.rows, frame.cols);

	bool running = true;
	while (running) {
		//display live webcam view in upper corner of image.
		frame = get_frame(cap);

		//threshold
		// for testing, we're just using a match to white.
		Scalar match_colour(255, 255, 255);
		const float RANGE_FACTOR = 0.2;
		Scalar low_match(match_colour[0] * (1 - RANGE_FACTOR), match_colour[1] * (1 - RANGE_FACTOR), match_colour[2] * (1 - RANGE_FACTOR));
		Scalar high_match(match_colour[0] * (1 + RANGE_FACTOR), match_colour[1] * (1 + RANGE_FACTOR), match_colour[2] * (1 + RANGE_FACTOR));
		inRange(frame, low_match, high_match, thresholded);
		erode_and_dilate(thresholded);
		cvtColor(thresholded, thresholdedFrame, CV_GRAY2RGB);

		//plain contours
		vector<vector<Point> > contours;
		Mat thresholded_contours = thresholded.clone();
		findContours(thresholded_contours, contours, CV_RETR_TREE, CV_CHAIN_APPROX_NONE);

		//smooth contours
		vector<vector<Point> > smooth_contours;
		frame_smootour.update(thresholded);
		smooth_contours = frame_smootour.get_contours();

		//implicit surface image
		Mat implicit_image = 255 * frame_smootour.get_implicit_image();
		cvtColor(implicit_image, implicitFrame, CV_GRAY2RGB);

		//draw contours on frames
		Scalar contour_colour(255, 0, 0, 255);
		drawContours(frame, contours, -1, contour_colour, 1);
		drawContours(thresholdedFrame, contours, -1, contour_colour, 2);

		Scalar smooth_contour_colour(0, 0, 255, 255);
		drawContours(frame, smooth_contours, -1, smooth_contour_colour, 1);
		drawContours(implicitFrame, smooth_contours, -1, smooth_contour_colour, 2);


		//display all
		frame.copyTo(windowFrame(frameROI));

		thresholdedFrame.copyTo(windowFrame(tresholdedROI));
		implicitFrame.copyTo(windowFrame(implicitROI));

		imshow("smootour-test", windowFrame);

		//input
		int key = waitKey(30);
		if (key != -1) {
			switch (key) {
			case 27: //ESC
				running = false;
				break;

			default:
				cout << "key " << key << "\n";
				break;
			};
		}
	}

	return 0;
}