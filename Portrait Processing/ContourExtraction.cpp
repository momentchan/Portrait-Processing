#include "FuncDeclaration.h"
#include "imatrix.h"
#include "ETF.h"
#include "fdog.h"
#include "myvec.h"

extern Mat R2;
extern Mat sketchContours;
extern bool contourShow;
extern vector<vector<Point> > filteredContours;
extern Point2f center;
int low_threshold = 3;//92; //canny threshold
int max_threshold = 10;//150;
int ratio = 2;
int kernel_size = 3;
//float Ts = 0.1;
int Ts = 1;

float MWCalculation(Moments mu, Point2f mc){
	return mu.m00 / norm(Mat(mc), Mat(center));
}
void ContourExtraction(){
	cout << "Extraction contour..." << endl;
	if (contourShow){
		namedWindow("Original Contours", CV_WINDOW_AUTOSIZE);
		createTrackbar("Contour Threshold:", "Original Contours", &low_threshold, max_threshold, CannyThreshold);
		createTrackbar("Criterion Threshold: ", "Original Contours", &Ts, 10, CannyThreshold);
	}
	CannyThreshold(0, 0);
	waitKey(0);
	destroyAllWindows();
}
void  Bspline(vector<Point> &pointSet){
	vector <Point> spline;
	cout << pointSet.size() << " ";

	// pick up controls points


	for (int i = 3; i < pointSet.size(); i=i+5)
	{
		Point p1 = pointSet[i-3];
		Point p2 = pointSet[i-2];
		Point p3 = pointSet[i-1];
		Point p4 = pointSet[i];

		int divisions = sqrt(pow(p3.x - p2.x, 2) + pow(p3.y - p2.y, 2));
		//cout << divisions << endl;
		double a[5];
		double b[5];
		a[0] = (-p1.x + 3 * p2.x - 3 * p3.x + p4.x) / 6.0;
		a[1] = (3 * p1.x - 6 * p2.x + 3 * p3.x) / 6.0;
		a[2] = (-3 * p1.x + 3 * p3.x) / 6.0;
		a[3] = (p1.x + 4 * p2.x + p3.x) / 6.0;
		b[0] = (-p1.y + 3 * p2.y - 3 * p3.y + p4.y) / 6.0;
		b[1] = (3 * p1.y - 6 * p2.y + 3 * p3.y) / 6.0;
		b[2] = (-3 * p1.y + 3 * p3.y) / 6.0;
		b[3] = (p1.y + 4 * p2.y + p3.y) / 6.0;

		spline.push_back(Point(a[3], b[3]));
		for (i = 1; i <= divisions - 1; i++)
		{
			float  t = float(i) / float(divisions);
			//cout<<t<<endl;
			spline.push_back(Point((a[2] + t * (a[1] + t * a[0]))*t + a[3], (b[2] + t * (b[1] + t * b[0]))*t + b[3]));
		}
		
	}
	pointSet.clear();
	pointSet = spline;
	cout << pointSet.size() << endl;
}

void CoherentLine(Mat src, Mat & dst, double thres){

	// We assume that you have loaded your input image into an imatrix named "img"
	//Mat R2 = imread("R2.jpg", 0);
	int rows = src.rows;
	int cols = src.cols;
	imatrix img(rows, cols);

	// copy from dst (unsigned char) to img (int)
	for (int y = 0; y < rows; y++)
	for (int x = 0; x < cols; x++)
		img[y][x] = src.at<uchar>(y, x);
	int image_x = img.getRow();
	int image_y = img.getCol();
	//////////////////////////////////////////////////


	//////////////////////////////////////////////////
	ETF e;
	e.init(image_x, image_y);
	e.set(img); // get gradients from input image

	//e.set2(img); // get gradients from gradient map

	e.Smooth(4, 2);
	//////////////////////////////////////////////////////

	///////////////////////////////////////////////////
	double tao = 0.99;
	//double thres = 0.7;
	GetFDoG(img, e, 1.0, 3.0, tao);
	GrayThresholding(img, thres);

	/////////////////////////////////////////////
	// copy result from img (int) to dst (unsigned char)
	for (int y = 0; y < rows; y++) {
		for (int x = 0; x < cols; x++) {
			dst.at<uchar>(y, x) = (uchar)img[y][x];
		}
	}
	//imshow(" ", dst); cvWaitKey(0);


	/* Compare cannny
	Mat canny;
	Canny(src, canny, 90, 180, 3);
	canny = ~canny;
	imshow("", canny); cvWaitKey(0);
	*/
}


void thinningIteration(cv::Mat& img, int iter)
{
	CV_Assert(img.channels() == 1);
	CV_Assert(img.depth() != sizeof(uchar));
	CV_Assert(img.rows > 3 && img.cols > 3);

	cv::Mat marker = cv::Mat::zeros(img.size(), CV_8UC1);

	int nRows = img.rows;
	int nCols = img.cols;

	if (img.isContinuous()) {
		nCols *= nRows;
		nRows = 1;
	}

	int x, y;
	uchar *pAbove;
	uchar *pCurr;
	uchar *pBelow;
	uchar *nw, *no, *ne;    // north (pAbove)
	uchar *we, *me, *ea;
	uchar *sw, *so, *se;    // south (pBelow)

	uchar *pDst;

	// initialize row pointers
	pAbove = NULL;
	pCurr = img.ptr<uchar>(0);
	pBelow = img.ptr<uchar>(1);

	for (y = 1; y < img.rows - 1; ++y) {
		// shift the rows up by one
		pAbove = pCurr;
		pCurr = pBelow;
		pBelow = img.ptr<uchar>(y + 1);

		pDst = marker.ptr<uchar>(y);

		// initialize col pointers
		no = &(pAbove[0]);
		ne = &(pAbove[1]);
		me = &(pCurr[0]);
		ea = &(pCurr[1]);
		so = &(pBelow[0]);
		se = &(pBelow[1]);

		for (x = 1; x < img.cols - 1; ++x) {
			// shift col pointers left by one (scan left to right)
			nw = no;
			no = ne;
			ne = &(pAbove[x + 1]);
			we = me;
			me = ea;
			ea = &(pCurr[x + 1]);
			sw = so;
			so = se;
			se = &(pBelow[x + 1]);

			int A = (*no == 0 && *ne == 1) + (*ne == 0 && *ea == 1) +
				(*ea == 0 && *se == 1) + (*se == 0 && *so == 1) +
				(*so == 0 && *sw == 1) + (*sw == 0 && *we == 1) +
				(*we == 0 && *nw == 1) + (*nw == 0 && *no == 1);
			int B = *no + *ne + *ea + *se + *so + *sw + *we + *nw;
			int m1 = iter == 0 ? (*no * *ea * *so) : (*no * *ea * *we);
			int m2 = iter == 0 ? (*ea * *so * *we) : (*no * *so * *we);

			if (A == 1 && (B >= 2 && B <= 6) && m1 == 0 && m2 == 0)
				pDst[x] = 1;
		}
	}

	img &= ~marker;
}

/**
* Function for thinning the given binary image
*
* Parameters:
* 		src  The source image, binary with range = [0,255]
* 		dst  The destination image
*/
void thinning(const cv::Mat& src, cv::Mat& dst)
{
	dst = src.clone();
	dst /= 255;         // convert to binary image

	cv::Mat prev = cv::Mat::zeros(dst.size(), CV_8UC1);
	cv::Mat diff;
	
	//int i = 0;
	do {
		thinningIteration(dst, 0);
		thinningIteration(dst, 1);
		cv::absdiff(dst, prev, diff);
		dst.copyTo(prev);
		/*string s = "thinning";
		s.append(int2str(i));
		s.append(".jpg");
		imwrite(s, ~(255*dst));
		i++;*/
	} while (cv::countNonZero(diff) > 0);

	dst *= 255;
}

void BranchPointRemoval(Mat &src){
	int removePoint = 0;
	for (int i = 1; i < src.rows-1; i++)
		for (int j = 1; j < src.cols-1; j++){
			if ((int)src.at<uchar>(i, j)==255){
				int count = 0;
				for (int k = -1; k <= 1; k++){
					for (int l = -1; l <= 1; l++){
						if ((int)src.at<uchar>(i + k, j + l) == 255)
							count++;
					}
				}
				if (count>4){
					src.at<uchar>(i, j) = (uchar)0;
					removePoint += count;
				}
			}
		}
		cout << "removePoint: " << removePoint << endl;
}
void CannyThreshold(int, void*){
	

	vector<vector<Point> > detectedContours;
	vector<Vec4i> hierarchy;
	Mat Edges = Mat::zeros(R2.size(), CV_8UC1);

	// Detect edges using canny
	//Mat Blur;
	//blur(R2, Blur, Size(3, 3)); // Reduce noise with a kernel 3x3
	//Canny(Blur, Edges, low_threshold, low_threshold*ratio, kernel_size);

	// Detect edges using Coherent Line
	CoherentLine(R2, Edges, low_threshold*0.1);

	// Find contours
	Mat EdgesBinary;
	threshold(Edges, EdgesBinary, 200, 255, CV_THRESH_BINARY_INV);
	//imshow("", Edges);
	//imshow("EdgesBinary", EdgesBinary);

	thinning(EdgesBinary, EdgesBinary);
	//imshow("EdgesBinary", EdgesBinary); waitKey(0);
	BranchPointRemoval(EdgesBinary);


	findContours(EdgesBinary, detectedContours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE, Point(0, 0));
	



	for (size_t k = 0; k < detectedContours.size(); k++){
		//cout << detectedContours[k].size() << " ";
		approxPolyDP(Mat(detectedContours[k]), detectedContours[k], 2, false);
		//cout << detectedContours[k].size() << endl;
		if (detectedContours[k].size()>4){
			//cout<<k<<endl;
		//	Bspline(detectedContours[k]);
		}
	}
	
	/*vector<vector<Point> > smoothContours(detectedContours.size());
	for (size_t k = 0; k < detectedContours.size(); k++){
		approxPolyDP(Mat(detectedContours[k]), smoothContours[k], 2, false);
		cout << detectedContours[k].size() << " " << smoothContours[k].size() << endl;
	}*/
	


	

	// Get the moments
	vector<Moments> mu(detectedContours.size());
	for (int i = 0; i < detectedContours.size(); i++)
		mu[i] = moments(detectedContours[i], false);

	//  Get the mass centers:
	filteredContours.clear();
	vector<Point2f> mc(detectedContours.size());
	vector<float> MomentWeights(detectedContours.size());

	for (int i = 0; i < detectedContours.size(); i++) {
		mc[i] = Point2f(mu[i].m10 / mu[i].m00, mu[i].m01 / mu[i].m00);
		MomentWeights[i] = MWCalculation(mu[i], mc[i]);
		if (MomentWeights[i]> Ts*0.1)
			filteredContours.push_back(detectedContours[i]);
	}

	// Draw contours
	RNG rng(12345);
	Mat drawOrigin = Mat(R2.size(), CV_8UC3, Scalar(255, 255, 255));
	Mat drawFiltered = Mat(R2.size(), CV_8UC3, Scalar(255, 255, 255));
	sketchContours = Mat(R2.size(), CV_8UC1, Scalar(255, 255, 255));

	for (int i = 0; i< detectedContours.size(); i++) {
		Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
		drawContours(drawOrigin, detectedContours, i, color, 1, 8, hierarchy, 2, Point());
		if (MomentWeights[i]> Ts*0.1){
			//Color
			drawContours(drawFiltered, detectedContours, i, color, 1, 8, hierarchy, 2, Point());
			//Black
			drawContours(sketchContours, detectedContours, i, Scalar(0, 0, 0), 2, 8, hierarchy, 2, Point());
		}
	}




	cout << "Number of Edges before filtering : " << detectedContours.size() << endl;
	cout << "Number of Edges after filtering : " << filteredContours.size() << endl << endl;

	/// Show in a window
	imwrite("ContourOrigin.jpg", drawOrigin);
	imwrite("ContourFiltered.jpg", drawFiltered);
	imwrite("SketchContours.jpg", sketchContours);
	imwrite("ExtractEdges.jpg", Edges);

	if (contourShow){
		imshow("Original Contours", drawOrigin);
		imshow("Filtered Contours", drawFiltered);
		imshow("Final Contours", sketchContours);
		imshow("Extracted Edges", Edges);
	}
}