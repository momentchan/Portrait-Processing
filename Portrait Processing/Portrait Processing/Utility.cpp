#include "FuncDeclaration.h"

extern int low_threshold;
extern int ratio;
extern int kernel_size;

bool CompareLength(vector<Point> contour1, vector<Point> contour2) {
	double i = arcLength(contour1, false);
	double j = arcLength(contour2, false);
	return (i > j);
}
bool CompareArea(vector<Point> area1, vector<Point> area2) {
	double i = area1.size();
	double j = area2.size();
	return (i > j);
}
bool CompareBox(Rect area1, Rect area2) {
	double i = area1.area();
	double j = area2.area();
	return (i > j);
}
int HistogramCalulation(Mat image, Mat & histImage, float alpha){
	// allcoate memory for no of pixels for each intensity value
	int histogram[256];

	// initialize all intensity values to 0
	for (int i = 0; i < 255; i++)
		histogram[i] = 0;

	// calculate the no of pixels for each intensity values
	for (int y = 0; y < image.rows; y++)
	for (int x = 0; x < image.cols; x++)
		histogram[(int)image.at<uchar>(y, x)]++;

	// draw the histograms
	int hist_w = 512; int hist_h = 400;
	int bin_w = cvRound((double)hist_w / 256);

	histImage = Mat(hist_h, hist_w, CV_8UC1, Scalar(255, 255, 255));

	// find the maximum intensity element from histogram
	int max_num = histogram[0];
	int max = 0;
	int total_num = image.size().area();
	int threshold = total_num*alpha;
	int accumulate = 0;
	int T;
	bool find = false;

	for (int i = 1; i < 256; i++){
		if (!find){
			if (accumulate < threshold){
				accumulate += histogram[i];
			}
			else {
				find = true;
				T = i - 1;
				accumulate -= histogram[i - 1];
			}
		}
		if (max_num < histogram[i]){
			max_num = histogram[i];
			max = i;
		}
	}
	//cout << threshold << endl << T << endl << accumulate << endl << endl;

	// normalize the histogram between 0 and histImage.rows
	for (int i = 0; i < 255; i++){
		histogram[i] = ((double)histogram[i] / max_num)*histImage.rows;
	}

	// draw the intensity line for histogram
	for (int i = 0; i < 255; i++)
		line(histImage, Point(bin_w*(i), hist_h), Point(bin_w*(i), hist_h - histogram[i]), Scalar(0, 0, 0), 1, 8, 0);
	imwrite("b2_histogram.jpg", histImage);

	cvWaitKey();
	return T;
}
Rect BoundingBox(Mat holefilled){
	Mat fill_edges;
	vector<Vec4i> hierarchy;
	vector<vector<Point> > contours;
	vector<Rect> boundRect;

	/// Detect edges using canny
	Canny(holefilled, fill_edges, low_threshold, low_threshold*ratio, kernel_size);
	findContours(fill_edges, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

	vector<vector<Point> > contours_poly(contours.size());
	for (int i = 0; i < contours.size(); i++)
	{
		approxPolyDP(Mat(contours[i]), contours_poly[i], 3, true);
		boundRect.push_back(boundingRect(Mat(contours_poly[i])));
	}

	/// Draw polygonal contour + bonding rects + circles
	Mat drawing = Mat::zeros(fill_edges.size(), CV_8UC3);
	boundRect.erase(unique(boundRect.begin(), boundRect.end()), boundRect.end());
	sort(boundRect.begin(), boundRect.end(), CompareBox);

	// Use two-dimension array to store the distance
	//cout << "Number of box befor filtering: " << boundRect.size() << endl;
	int size = boundRect.size();
	int** point_dis = new int*[size];
	for (int i = 0; i < size; ++i)
		point_dis[i] = new int[size];
	for (int i = 0; i < size; i++)
	for (int j = 0; j < size; j++)
		point_dis[i][j] = norm(boundRect[i].br() - boundRect[j].br()) + norm(boundRect[i].tl() - boundRect[j].tl());

	vector <int> deleteBoxIndex;
	for (int i = 0; i < size; i++)
	for (int j = i + 1; j < size; j++)
	if (point_dis[i][j] < 5)
		deleteBoxIndex.push_back(i);

	for (int i = deleteBoxIndex.size() - 1; i >= 0; i--)
		boundRect.erase(boundRect.begin() + deleteBoxIndex[i]);
	RNG rng(12345);
	for (int i = 0; i< boundRect.size(); i++)
	{
		Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255));
		//drawContours(drawing, contours_poly, i, color, 1, 8, vector<Vec4i>(), 0, Point());
		//cout << boundRect[i].tl() << " " << boundRect[i].br() << " " << boundRect[i].area() << endl;
		rectangle(drawing, boundRect[i].tl(), boundRect[i].br(), color, 2, 8, 0);
	}
	//cout << "Number of box after filtering: " << boundRect.size() << endl<<endl;

	/// Show in a window
	//imshow("Contours", drawing);
	//cvWaitKey(0);
	return boundRect[0];
}
Mat HoleFilling(Mat fill_region)
{
	//cvtColor(fill_region, fill_region, CV_BGR2GRAY);
	Mat image_thresh;
	threshold(fill_region, image_thresh, 125, 255, THRESH_BINARY);

	// Loop through the border pixels and if they're black, floodFill from there
	Mat mask;
	image_thresh.copyTo(mask);
	for (int i = 0; i < mask.cols; i++) {
		if (mask.at<char>(0, i) == 0)
			floodFill(mask, Point(i, 0), 255, 0, 10, 10);
		if (mask.at<char>(mask.rows - 1, i) == 0)
			floodFill(mask, Point(i, mask.rows - 1), 255, 0, 10, 10);
	}
	for (int i = 0; i < mask.rows; i++) {
		if (mask.at<char>(i, 0) == 0)
			floodFill(mask, Point(0, i), 255, 0, 10, 10);
		if (mask.at<char>(i, mask.cols - 1) == 0)
			floodFill(mask, Point(mask.cols - 1, i), 255, 0, 10, 10);
	}
	Mat holefilled;
	// Compare mask with original.
	image_thresh.copyTo(holefilled);
	for (int row = 0; row < mask.rows; ++row) {
		for (int col = 0; col < mask.cols; ++col) {
			if (mask.at<char>(row, col) == 0) {
				holefilled.at<char>(row, col) = 255;
			}
		}
	}
	bitwise_not(holefilled, holefilled);
	//imshow("Final image", holefilled);
	return holefilled;
	//imwrite("final.jpg", newImage);
	//waitKey(0);
}
void ConnectedComponent(const Mat img, vector<vector<Point2i>>& blobs){
	Mat binary;
	Mat labelImage;
	threshold(~img, binary, 0.0, 1.0, THRESH_BINARY);
	FindBlobs(binary, blobs, labelImage);
	imwrite("label.jpg", labelImage);
	sort(blobs.begin(), blobs.end(), CompareLength);
}
void FindBlobs(const Mat &binary, vector < vector<Point2i> > &blobs, Mat &labelImage)
{
	blobs.clear();

	// Fill the labelImage with the blobs
	// 0  - background
	// 1  - unlabelled foreground
	// 2+ - labelled foreground

	binary.convertTo(labelImage, CV_32SC1);

	int label_count = 2; // starts at 2 because 0,1 are used already

	for (int y = 0; y < labelImage.rows; y++) {
		int *row = (int*)labelImage.ptr(y);
		for (int x = 0; x < labelImage.cols; x++) {
			if (row[x] != 1) {
				continue;
			}

			Rect rect;
			floodFill(labelImage, Point(x, y), label_count, &rect, 0, 0, 4);

			vector <Point2i> blob;

			for (int i = rect.y; i < (rect.y + rect.height); i++) {
				int *row2 = (int*)labelImage.ptr(i);
				for (int j = rect.x; j < (rect.x + rect.width); j++) {
					if (row2[j] != label_count) {
						continue;
					}
					blob.push_back(Point2i(j, i));
				}
			}

			blobs.push_back(blob);

			label_count++;
		}
	}
}
string int2str(int &i) {
	string s;
	stringstream ss(s);
	ss << i;
	return ss.str();
}
string outputFileName(string file_name, int index, string type){
	file_name.append(int2str(index));
	file_name.append(type);
	return file_name;
}

