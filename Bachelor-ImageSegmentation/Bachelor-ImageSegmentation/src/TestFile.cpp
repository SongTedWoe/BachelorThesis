#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/core/opengl.hpp>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <limits>
#include <chrono>
#include <math.h>
using namespace cv;
using namespace std;
using namespace chrono;


const char* vShaderFile = "src/VertexShader.glsl";
const char* fShaderFile = "src/FragmentShader.glsl";
const char* vTexShader = "src/TextureVertexShader.glsl";
const char* fTexShader = "src/TextureFragmentShader.glsl";
const char* vTexMatShader = "sry/TextureMatrixVertexShader.glsl";
const char* fTexMatShader = "sry/TextureMatrixFragmentShader.glsl";


uchar* thresholding3C(Mat img, int width, int height, uchar R, uchar G, uchar B, bool up, int segIndex1, int segIndex2) {
	//TODO: Kommentieren, was hier passiert: Thresholding mit 3 farben (RGB, ohne Alpha) mit nur einem Schwellenwert
	uchar* ret = new uchar[width*height];
	uchar* data = img.data;

	for (int i = 0; i < width*height; i++) {
		if (up) {
			if (data[i * 3] >= R && data[i * 3 + 1] >= G && data[i * 3 + 2] >= B) {
				ret[i] = segIndex2;
			}
			else {
				ret[i] = segIndex1;
			}
		}
		else {
			if (data[i * 3] <= R && data[i * 3 + 1] <= G && data[i * 3 + 2] <= B) {
				ret[i] = segIndex2;
			}
			else {
				ret[i] = segIndex1;
			}
		}
	}

	return ret;
}
uchar* thresholding3C(uchar* data, int width, int height, uchar R, uchar G, uchar B, bool up, int segIndex1, int segIndex2) {
	//TODO: Kommentieren, was hier passiert: Thresholding mit 3 farben (RGB, ohne Alpha) mit nur einem Schwellenwert
	uchar* ret = new uchar[width*height];

	for (int i = 0; i < width*height; i++) {
		if (up) {
			if (data[i * 3] >= R && data[i * 3 + 1] >= G && data[i * 3 + 2] >= B) {
				ret[i] = segIndex2;
			}
			else {
				ret[i] = segIndex1;
			}
		}
		else {
			if (data[i * 3] <= R && data[i * 3 + 1] <= G && data[i * 3 + 2] <= B) {
				ret[i] = segIndex2;
			}
			else {
				ret[i] = segIndex1;
			}
		}
	}

	return ret;
}

uchar* getSegmentFromImg3C(Mat image, int width, int height, uchar* segment, uchar segIndex) {
	//TODO: Kommentieren: Segmentierung des Bildes in ein neues Bild (altes Bild bleibt unverändert)
	uchar* imgData = image.data;
	uchar* ret = new uchar[width*height * 3];

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
uchar* getSegmentFromImg3C(uchar* imgData, int width, int height, uchar* segment, uchar segIndex) {
	//TODO: Kommentieren: Segmentierung des Bildes in ein neues Bild (altes Bild bleibt unverändert)
	uchar* ret = new uchar[width*height * 3];

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
	for (int i = 0; i%width < width && goon; i = (i + width) >= height * width ? (i % width + 1) : (i + width)) {
		if (segment[i] == segIndex) {
			minx = (i % width);
			goon = false;
		}
	}
	goon = true;
	// --- get maxx
	for (int i = width * height - 1; i%width >= 0 && goon; i = (i - width) < 0 ? ((height - 1)*width + (i % width - 1)) : (i - width)) {
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
				retData[(x + inWidth * y) * 4] = imgData[(minx + x + width * (miny + y)) * 3];
				retData[(x + inWidth * y) * 4 + 1] = imgData[(minx + x + width * (miny + y)) * 3 + 1];
				retData[(x + inWidth * y) * 4 + 2] = imgData[(minx + x + width * (miny + y)) * 3 + 2];
				retData[(x + inWidth * y) * 4 + 3] = 255;
			}
			else {
				retData[(x + inWidth * y) * 4] = 0;
				retData[(x + inWidth * y) * 4 + 1] = 0;
				retData[(x + inWidth * y) * 4 + 2] = 0;
				retData[(x + inWidth * y) * 4 + 3] = 0;
			}
		}
	}

	return ret;
}
uchar* getSegmentMatFromImg3C(uchar* imgData, int width, int height, uchar* segment, uchar segIndex) {
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
	for (int i = 0; i%width < width && goon; i = (i + width) >= height * width ? (i % width + 1) : (i + width)) {
		if (segment[i] == segIndex) {
			minx = (i % width);
			goon = false;
		}
	}
	goon = true;
	// --- get maxx
	for (int i = width * height - 1; i%width >= 0 && goon; i = (i - width) < 0 ? ((height - 1)*width + (i % width - 1)) : (i - width)) {
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
	uchar* ret = new uchar(inHeight*inWidth * 4);

	// - fill Material Data
	for (int x = 0; x < inWidth; x++) {
		for (int y = 0; y < inHeight; y++) {
			if (segment[(minx + x + width * (miny + y))] == segIndex) {
				ret[(x + inWidth * y) * 4] = imgData[(minx + x + width * (miny + y)) * 3];
				ret[(x + inWidth * y) * 4 + 1] = imgData[(minx + x + width * (miny + y)) * 3 + 1];
				ret[(x + inWidth * y) * 4 + 2] = imgData[(minx + x + width * (miny + y)) * 3 + 2];
				ret[(x + inWidth * y) * 4 + 3] = 255;
			}
			else {
				ret[(x + inWidth * y) * 4] = 0;
				ret[(x + inWidth * y) * 4 + 1] = 0;
				ret[(x + inWidth * y) * 4 + 2] = 0;
				ret[(x + inWidth * y) * 4 + 3] = 0;
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

string *getShaderFromFile(const char *filePath) {
	ifstream input;
	string *vertexcode = NULL;
	input.exceptions(ifstream::failbit | ifstream::badbit);
	try {
		input.open(filePath);
		stringstream shaderstream;
		shaderstream << input.rdbuf();
		input.close();
		vertexcode = new string(shaderstream.str());
	}
	catch (ifstream::failure e) {
		cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << endl;
	}
	return vertexcode;
}




//------------------------------------------------------------------------------------------------//
//------------------------------------ Testing out Framworks -------------------------------------//
//------------------------------------------------------------------------------------------------//
GLfloat vertices[] = {
	-0.5f,-0.5f, 0.0f,
	0.5f, -0.5f, 0.0f,
	0.0f, 0.5f, 0.0f,
	-0.5f,-0.5f,0.0f,
	-0.5f,0.5f, 0.0f,
	0.5f, 0.5f, 0.0f,
	0.5f,-0.5f, 0.0f
};
GLfloat vertices2[] = {
	//position				color					texture coordinates
	-1.0f, -1.0f, 0.0f,		1.0f, 1.0f, 1.0f,		0.0f, 1.0f,//top-left
	-1.0f, 1.0f, 0.0f,		1.0f, 1.0f, 1.0f,		0.0f, 0.0f,//bottom-left
	1.0f, 1.0f, 0.0f,		1.0f, 1.0f, 1.0f,		1.0f, 0.0f,//bottom-right
	1.0f, -1.0f, 0.0f,		1.0f, 1.0f, 1.0f,		1.0f, 1.0f//top-right
};
GLuint indices2[] = {
	0, 2, 1,
	0, 3, 2
};
bool run = true;
milliseconds startTime, currentTime;

int shaderProgram, shaderProgram2;
unsigned int VBO, VAO, VBO2, VAO2, EBO2, texture2;
void testOpenCV() {
	//This function tests out OpenCV functionality and the functions for image Segmentation
	string filename("data/Logo.png");
	string windowName1("Display window"),
		windowName2("THM Logo - Original"),
		windowName3("THM Logo - Segment"),
		windowName4("THM Logo - Segment with Material"),
		windowName5("THM Logo - Segmented (index = 0)"),
		windowName6("THM Logo - Segmented (index = 1)"),
		windowName7("THM Logo - Segmented (index = 1, optimized)");

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
	uchar* segment = thresholding3C(image, image.cols, image.rows, 200, 255, 200, 0, 0, 1);
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

	//Delete unneeded Heap-Memory
	delete segment;
	delete showSegment;
	delete segmentImg1;
	delete segmentImg2;
	~image;
	~segmentMat;
	~segmentMat1;
	~showSegmentMat1;
	~segmentImg1Mat;
	~segmentImg2Mat;
	~segmentImg3Mat;
	segment = NULL;
	showSegment = NULL;
	segmentImg1 = NULL;
	segmentImg2 = NULL;

	// - delete windows
	destroyWindow(windowName1);
	destroyWindow(windowName2);
	destroyWindow(windowName3);
	destroyWindow(windowName4);
	destroyWindow(windowName5);
	destroyWindow(windowName6);
	destroyWindow(windowName7);
}



void setupTest() {
	//Get ShaderCode
	string *vShaderCode = getShaderFromFile(vShaderFile);
	const char* vShaderCodeChar = (*vShaderCode).c_str();
	string *orangeFragmentShaderCode = getShaderFromFile(fShaderFile);
	const char* orangeFragmentShaderCodeChar = (*orangeFragmentShaderCode).c_str();

	//Compile VertexShader
	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vShaderCodeChar, NULL);
	glCompileShader(vertexShader);

	int success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << endl;
	}

	//Compile FragmentShader
	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &orangeFragmentShaderCodeChar, NULL);
	glCompileShader(fragmentShader);

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << endl;
	}

	//Link Shaders
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << endl;
	}
	//Clean Up Shaders
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	delete(vShaderCode);
	delete(orangeFragmentShaderCode);

	//Set up VertexBufferObject and VertexArrayObject
	glGenBuffers(1, &VBO);
	glGenVertexArrays(1, &VAO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	//Set up Scene
	startTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
}
void setdownTest() {
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteProgram(shaderProgram);
}
void moveTriangle() {
	currentTime = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
	double dif = (double)(currentTime.count() - startTime.count()) / 1000;
	float sindif = sin((float)dif) * 0.5f;
	float cosdif = cos((float)dif) * 0.5f;
	vertices[6] = 0.0f + sindif;
	vertices[7] = 0.5f + cosdif;
	vertices[13] = 0.5f + cosdif;
	vertices[16] = 0.5f + cosdif;
	cout << "time = " << dif << endl;
}
void drawTestGL(void *params) {
	// - some OpenGL
	moveTriangle();
	glClearColor(1.f, 1.f, 1.f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(shaderProgram);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawArrays(GL_TRIANGLES, 0, 3);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDrawArrays(GL_LINE_LOOP, 3, 4);
	glBindVertexArray(0);
}
void testOpenGL() {

	// - create OpenGL-Context (OpenCV can do so)
	namedWindow("OpenGL window", WINDOW_OPENGL);
	setWindowProperty("OpenGL window", WINDOW_AUTOSIZE, WINDOW_AUTOSIZE);
	setOpenGlContext("OpenGL window");
	setOpenGlDrawCallback("OpenGL window", drawTestGL);

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		cout << "Failed to initialize glew" << endl;
		return;
	}
	setupTest();

	while (waitKey(16) < 0) {
		updateWindow("OpenGL window");
	}

	setdownTest();
	setOpenGlDrawCallback("OpenGL window", 0, 0);
	destroyWindow("OpenGL window");
}
void setupTest2() {
	//get Shader Chars
	string *vShaderCode = getShaderFromFile(vTexShader);
	cout << *vShaderCode << endl;
	const char* vShaderChar = vShaderCode->c_str();
	string *fShaderCode = getShaderFromFile(fTexShader);
	cout << *fShaderCode << endl;
	const char* fShaderChar = fShaderCode->c_str();

	//Compile Shaders
	// - VertexShader:
	// -- Create Shader
	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	// -- Bind Source
	glShaderSource(vertexShader, 1, &vShaderChar, NULL);
	// -- Compile Shader
	glCompileShader(vertexShader);
	// -- check succesfull Compilation
	int success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << endl;
	}

	// - FragmentShader
	// -- Create shader
	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	// -- Bind Source
	glShaderSource(fragmentShader, 1, &fShaderChar, NULL);
	// -- Compile Shader
	glCompileShader(fragmentShader);
	// -- check succesfull Compilation
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << endl;
	}

	//Link Shaders
	// - create ShaderProgram
	shaderProgram2 = glCreateProgram();
	// - Attach all Shaders to ShaderProgram
	glAttachShader(shaderProgram2, vertexShader);
	glAttachShader(shaderProgram2, fragmentShader);
	// - Link ShaderProgram
	glLinkProgram(shaderProgram2);
	// - check successfull Linking
	glGetProgramiv(shaderProgram2, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << endl;
	}

	//Clean up Shaders
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	delete(vShaderCode);
	delete(fShaderCode);

	//Set up VertexBufferObject, ElementBufferObject and VertexArrayObject
	// - Generate VBO, EBO and VAO
	glGenBuffers(1, &VBO2);
	glGenBuffers(1, &EBO2);
	glGenVertexArrays(1, &VAO2);
	// - Bind VAO
	glBindVertexArray(VAO2);
	// - Configure VBO
	// -- Bind VBO
	glBindBuffer(GL_ARRAY_BUFFER, VBO2);
	// -- set VBO Data
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2), vertices2, GL_STATIC_DRAW);
	// - Configure EBO
	// -- Bind EBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO2);
	// -- set EBO Data
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices2), indices2, GL_STATIC_DRAW);
	// - Set VAO-Parameters
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid *)(6 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	//Set up Scene
	// - generate Texture
	glGenTextures(1, &texture2);
	// - Bind Texture
	glBindTexture(GL_TEXTURE_2D, texture2);
	// - Set Texture options on bound Texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	// - get and set texture
	string filename("data/Logo.png");
	Mat image = imread(filename, IMREAD_UNCHANGED);
	if (image.empty()) // Check for invalid input
	{
		cout << "Could not open or find the image" << endl;
		return;
	}
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.cols, image.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, image.data);
	glGenerateMipmap(GL_TEXTURE_2D);
	image.release();
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}
void setdownTest2() {
	glDeleteVertexArrays(1, &VAO2);
	glDeleteBuffers(1, &VBO2);
	glDeleteBuffers(1, &EBO2);
	glDeleteProgram(shaderProgram2);
	glDeleteTextures(1, &texture2);
}
void drawTestGL2(void *params) {
	// - some OpenGL
	glClearColor(1.f, 1.f, 1.f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(shaderProgram2);
	glActiveTexture(texture2);
	glBindTexture(GL_TEXTURE_2D, texture2);
	glBindVertexArray(VAO2);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}
void testOpenGL2() {

	// - create OpenGL-Context (OpenCV can do so)
	namedWindow("OpenGL window", WINDOW_OPENGL);
	setWindowProperty("OpenGL window", WINDOW_AUTOSIZE, WINDOW_AUTOSIZE);
	setOpenGlContext("OpenGL window");
	setOpenGlDrawCallback("OpenGL window", drawTestGL2);

	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		cout << "Failed to initialize glew" << endl;
		return;
	}
	setupTest2();

	while (waitKey(16) < 0) {
		updateWindow("OpenGL window");
	}

	setdownTest2();
	setOpenGlDrawCallback("OpenGL window", 0, 0);
	destroyWindow("OpenGL window");
}
void testWaitKey() {
	namedWindow("No Reason Window", WINDOW_AUTOSIZE);
	bool run = true;
	int Key = 0;

	while (run) {
		Key = waitKey();
		cout << "You pressed the Key with the Keycode: " << Key << endl;
		cout << "To end the Test press ESC" << endl;
		if (Key == 27) run = false;
	}

	destroyWindow("No Reason Window");
}

void error_callback(int error, const char* description);

static void kCallBack(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
}

void testGLFW() {
	cout << "start the test for GLFW" << endl;
	cout << glfwGetVersionString() << endl;
	glfwSetErrorCallback(error_callback);
	if (glfwInit()) {

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		GLFWwindow* window = glfwCreateWindow(640, 480, "OpenGL", NULL, NULL);
		if (!window) {
			cout << "Failed to create Window" << endl;
			glfwTerminate();
			exit(EXIT_FAILURE);
		}

		glfwSetKeyCallback(window, kCallBack);

		glfwMakeContextCurrent(window);

		glewExperimental = GL_TRUE;
		if (glewInit() != GLEW_OK) {
			cout << "Failed to initialize glew" << endl;
			return;
		}


		int width, height;
		float red, speed = 5;
		glfwSwapInterval(1);

		cout << "starting!" << endl;
		while (!glfwWindowShouldClose(window)) {


			glfwGetFramebufferSize(window, &width, &height);
			glViewport(0, 0, width, height);
			red = (float)((1 + std::sin(glfwGetTime()*speed)) / 2);
			cout << "Color: (" << red << ", 0, 0)" << endl;
			glClearColor(red, 0, 0, 1);
			glClear(GL_COLOR_BUFFER_BIT);

			glfwSwapBuffers(window);
			glfwPollEvents();
		}

		glfwDestroyWindow(window);

		glfwTerminate();
	}
	else {
		exit(EXIT_FAILURE);
	}
	cout << "end the test for GLFW" << endl;
}
//------------------------------------------------------------------------------------------------//
//------------------------------------ Testing out Framworks - FINISH-----------------------------//
//------------------------------------------------------------------------------------------------//

int main(int argc, char** argv)
{
	char c;
	bool repeat = true;
	while (repeat) {
		cout << "To test OpenCV			enter c" << endl;
		cout << "To test OpenGL			enter g" << endl;
		cout << "To test OpenGL with textures	enter G" << endl;
		cout << "To test GLFW			enter f" << endl;
		cout << "To get buildInformation		enter i" << endl;
		cout << "To end the program		enter q" << endl;
		cout << "To test waitKey 		enter w" << endl;
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
		case 'G':
			testOpenGL2();
			break;
		case 'f':
			testGLFW();
			break;
		case 'i':
			cout << getBuildInformation() << endl;
			break;
		case 'q':
			repeat = false;
			break;
		case 'w':
			testWaitKey();
			break;
		case 't':
			testClasses();
			break;
		}
	}
	return 0;
}