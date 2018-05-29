#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <limits>
#include <GL/glut.h>
using namespace cv;
using namespace std;

//TODO: 28.05. fertig
//TODO: Berechnungen müssen gemacht werden (Pen and Paper)
//TODO: Mit den Daten muss OpenGL aktiviert werden, damit man diese in 3D Space legen kann.
//TODO: Für OpenGL muss Kamera angelegt werden, Texturen möglich sein und auch deren verschiebung (Kamera muss gegebenenfalls nicht verschiebbar sein)
//TODO: Kontrolschema muss hergestellt werden aus offensichtlichen Gründen.
//TODO: Tests machen
//TODO: Kommentare nicht vergessen!

//TODO: 29.05. fertig


uchar* thresholding3C(Mat img, int width, int height, uchar R, uchar G, uchar B, bool up) {
	//TODO: Kommentieren, was hier passiert: Thresholding mit 3 farben (RGB, ohne Alpha) mit nur einem Schwellenwert
	uchar* ret = new uchar [width*height];
	uchar* data = img.data;

	for (int i = 0; i < width*height; i++) {
		if (up) {
			if (data[i*3] >= R && data[i * 3 + 1] >= G && data[i * 3 + 2] >= B) {
				ret[i] = 1;
			}
			else {
				ret[i] = 0;
			}
		}
		else {
			if (data[i * 3] <= R && data[i * 3 + 1] <= G && data[i * 3 + 2] <= B) {
				ret[i] = 1;
			}
			else {
				ret[i] = 0;
			}
		}
	}

	return ret;
}

uchar* getSegmentFromImg3C(Mat image, int width, int height, uchar* segment, uchar segIndex) {
	//TODO: Kommentieren: Segmentierung des Bildes in ein neues Bild (altes Bild bleibt unverändert)
	uchar* imgData = image.data;
	uchar* ret = new uchar [width*height*3];

	for (int i = 0; i < width*height; i++) {
		if (segment[i] == segIndex) {
			ret[i * 3] = imgData[i * 3];
			ret[i * 3 + 1] = imgData[i * 3 + 1];
			ret[i * 3 + 2] = imgData[i * 3 + 2];
		}
		else {
			ret[i * 3] = 0;
			ret[i * 3 + 1] = 0;
			ret[i * 3 + 2] = 0;
		}
	}

	return ret;
}

Mat getSegmentMatFromImg3C(Mat image, int width, int height, uchar* segment, uchar segIndex) {
	//TODO: Kommentieren: Segmentierung eines Bildes, ohne Veränderung des Ursprungbildes, wobei ein optimale Maße für das Segment verwendet werden.

	//--------------------------------------------------------------------------------------------------------
	//-------------------------------------- Optimization Calculations start ---------------------------------
	//--------------------------------------------------------------------------------------------------------
	
	
	// - Cropping and getting Dimensions
	int minx = width, maxx = 0;
	int miny = height, maxy = 0;
	bool goon = true;
	// --- get miny
	for (int i = 0; i < width * height && goon; i++) {
		if (segment[i] == segIndex) {
			miny = (i / width);
			goon = false;
		}
	}
	goon = true;
	// --- get maxy
	for (int i = width * height; i >= 0 && goon; i--) {
		if (segment[i] == segIndex) {
			maxy = (i / width) + 1;
			goon = false;
		}
	}
	goon = true;
	// --- get minx
	for (int i = 0; i%width < width  && goon; i = (i + width) >= height * width ? (i % width + 1) : (i + width)) {
		if (segment[i] == segIndex) {
			minx = (i % width); 
			goon = false;
		}
	}
	goon = true;
	// --- get maxx
	for (int i = width * height - 1; i%width >= 0 && goon; i = (i - width) < 0 ? ((height-1)*width+(i % width - 1)) : (i - width)) {
		if (segment[i] == segIndex) {
			maxx = (i % width) + 1;
			goon = false;
		}
	}

	int inWidth = maxx - minx;
	int inHeight = maxy - miny;

	//--------------------------------------------------------------------------------------------------------
	//-------------------------------------- Optimization Calculations stop ----------------------------------
	//--------------------------------------------------------------------------------------------------------


	// - create Material with optimal size
	uchar* imgData = image.data;
	Mat ret = Mat(inHeight, inWidth, CV_8UC(4));
	uchar* retData = ret.data;

	// - fill Material Data
	for (int x = 0; x < inWidth; x++) {
		for (int y = 0; y < inHeight; y++) {
			if (segment[(minx + x + width * (miny + y))] == segIndex) {
				retData[(x + inWidth * y) * 4]		= imgData[(minx + x + width * (miny + y)) * 3];
				retData[(x + inWidth * y) * 4 + 1]	= imgData[(minx + x + width * (miny + y)) * 3 + 1];
				retData[(x + inWidth * y) * 4 + 2]	= imgData[(minx + x + width * (miny + y)) * 3 + 2];
				retData[(x + inWidth * y) * 4 + 3]	= 255;
			}
			else {
				retData[(x + inWidth * y) * 4]		= 0;
				retData[(x + inWidth * y) * 4 + 1]	= 0;
				retData[(x + inWidth * y) * 4 + 2]	= 0;
				retData[(x + inWidth * y) * 4 + 3]	= 0;
			}
		}
	}

	return ret;
}

uchar *getViewableSegment(uchar *segment, int width, int height) {
	//Spread the gray levels to humanly distinguishable values

	// - get max and min of gray levels
	int min = 256, max = -1;
	// --- get min
	bool goon = true;
	for (int i = 0; i < width * height && goon; i++) {
		if (segment[i] < min) {
			min = segment[i];
			if (min == 0) goon = false;
		}
	}
	// --- get max
	goon = true;
	for (int i = 0; i < width * height && goon; i++) {
		if (segment[i] > max) {
			max = segment[i];
			if (max == 255) goon = false;
		}
	}

	// - use min and max to setup look-up-table
	int *lookUpTable = new int[255];
	if (max != min) {
		for (int i = min; i <= max; i++) {
			lookUpTable[i] = i - min + ((255) / (max - min));
		}
	}
	else {
		lookUpTable[max] = max;
	}

	// - use look-up-table to change values
	uchar *ret = new uchar[width*height];
	for (int i = 0; i < width*height; i++) {
		ret[i] = lookUpTable[segment[i]];
	}

	return ret;
}

Mat getViewableSegment(Mat segment) {
	//Spread the gray levels to humanly distinguishable values

	// - get Material data
	Mat *ret = new Mat(segment.rows, segment.cols, CV_8UC(1));
	uchar *data = ret->data;
	int width = segment.cols;
	int height = segment.rows;
	for (int i = 0; i < width * height; i++) {
		data[i] = segment.data[i];
	}

	// - get max and min of gray levels
	int min = 256, max = -1;
	// --- get min
	bool goon = true;
	for (int i = 0; i < width * height && goon; i++) {
		if (data[i] < min) {
			min = data[i];
			if (min == 0) goon = false;
		}
	}
	// --- get max
	goon = true;
	for (int i = 0; i < width * height && goon; i++) {
		if (data[i] > max) {
			max = data[i];
			if (max == 255) goon = false;
		}
	}

	// - use min and max to setup look-up-table
	int *lookUpTable = new int[255];
	if (max != min) {
		for (int i = min; i <= max; i++) {
			lookUpTable[i] = i - min + ((255) / (max - min));
		}
	}
	else {
		lookUpTable[max] = max;
	}

	// - use look-up-table to change values
	for (int i = 0; i < width*height; i++) {
		data[i] = lookUpTable[data[i]];
	}

	return *ret;
}


void testOpenCV() {
	//This function tests out OpenCV functionality and the functions for image Segmentation
	String filename = "data/Logo.png";
	String windowName1 = "Display window",
		windowName2 = "THM Logo - Original",
		windowName3 = "THM Logo - Segment",
		windowName4 = "THM Logo - Segment with Material",
		windowName5 = "THM Logo - Segmented (index = 0)",
		windowName6 = "THM Logo - Segmented (index = 1)",
		windowName7 = "THM Logo - Segmented (index = 1, optimized)";
		
	Mat image;
	image = imread(filename, IMREAD_UNCHANGED); // Read the file
	if (image.empty()) // Check for invalid input
	{
		cout << "Could not open or find the image" << endl;
		return;
	}
	// - display image
	namedWindow(windowName1, WINDOW_AUTOSIZE); // Create a window for display.
	imshow(windowName2, image); // Show our image inside it.

										  
	// - calculate segmentation
	uchar* segment = thresholding3C(image, image.cols, image.rows, 200, 255, 200, 0);
	uchar* showSegment = getViewableSegment(segment, image.cols, image.rows);

	// - display segmentation (bitmap)
	Mat segmentMat = Mat(image.rows, image.cols, CV_8UC(1), showSegment);
	imshow(windowName3, segmentMat);

	Mat segmentMat1 = Mat(image.rows, image.cols, CV_8UC(1), segment);
	Mat showSegmentMat1 = getViewableSegment(segmentMat1);
	imshow(windowName4, showSegmentMat1);

	// - seperate image in segmentation image
	// --- here two
	// --- two steps: - create Array with segmentations
	//				  - create Material filled with array to show with imshow()
	uchar* segmentImg1 = getSegmentFromImg3C(image, image.cols, image.rows, segment, 0);
	uchar* segmentImg2 = getSegmentFromImg3C(image, image.cols, image.rows, segment, 1);

	// - display segmentated images
	Mat segmentImg1Mat = Mat(image.rows, image.cols, CV_8UC(3), segmentImg1);
	imshow(windowName5, segmentImg1Mat);

	Mat segmentImg2Mat = Mat(image.rows, image.cols, CV_8UC(3), segmentImg2);
	imshow(windowName6, segmentImg2Mat);

	// - display segmentated image 	
	// --- in one step put into Material
	Mat segmentImg3Mat = getSegmentMatFromImg3C(image, image.cols, image.rows, segment, 1);
	imshow(windowName7, segmentImg3Mat);

	// - write image (segmentated image) into .png file
	vector<int> compression_params;
	compression_params.push_back(IMWRITE_PNG_COMPRESSION);
	compression_params.push_back(9);

	bool result = false;
	try
	{
		result = imwrite("alpha.png", segmentImg3Mat, compression_params);
	}
	catch (const cv::Exception& ex)
	{
		fprintf(stderr, "Exception converting image to PNG format: %s\n", ex.what());
	}
	if (result)
		printf("Saved PNG file with alpha data.\n");
	else
		printf("ERROR: Can't save PNG file.\n");

	waitKey();
	// - delete windows
	destroyWindow(windowName1);
	destroyWindow(windowName2);
	destroyWindow(windowName3);
	destroyWindow(windowName4);
	destroyWindow(windowName5);
	destroyWindow(windowName6);
	destroyWindow(windowName7);
}

void drawTest(void *params) {
	// - some OpenGL
	glClearColor(1.f, 1.f, 1.f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}

void testOpenGL() {

	// - create OpenGL-Context (OpenCV can do so)
	namedWindow("OpenGL window", WINDOW_OPENGL);
	setOpenGlContext("OpenGL window");
	setOpenGlDrawCallback("OpenGL window", drawTest);
	updateWindow("OpenGL window");

	waitKey();
	destroyWindow("OpenGL window");
}




int main(int argc, char** argv)
{
	char c;
	bool repeat = true;
	while (repeat) {
		cout << "To test OpenCV				enter c" << endl;
		cout << "To test OpenGL				enter g" << endl;
		cout << "To get buildInformation	enter i" << endl;
		cout << "To end the program			enter q" << endl;
		cin >> c;
		//emptys std-Input-Buffer in case of longer input, that just one character is read
		cin.ignore(numeric_limits<streamsize>::max(), '\n');

		switch (c) {
		case 'c':
			testOpenCV();
			break;
		case 'g':
			testOpenGL();
			break;
		case 'i':
			cout << getBuildInformation() << endl;
			break;
		case 'q':
			repeat = false;
			break;
		}
	}
	return 0;
}
