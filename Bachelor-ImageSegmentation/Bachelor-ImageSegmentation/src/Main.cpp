//#define GLEW_STATIC

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

/*
This program is the product of my bachelor thesis with the topic:
	Integration of 3D Effects on 2D objects

This program will in the end render the THM Logo and allow the user to select a area to segment in,
pick a color to segment by and finally let the segmentation be done, so that it can be either seen
as a simple closing in of the segment. This will be seen as a normal rendered scene or a stereoscopic 
version of that scene.
The segmentation algorithm used is thresholding. The rendering framework used is OpenGL and other 
frameworks that support its production. OpenCV is used for reading the image and manipulating it.

Let's hope it will be a nice program to use and a good bachelor thesis in the end 
*/


const char* vShaderFile = "src/VertexShader.glsl";
const char* fShaderFile = "src/FragmentShader.glsl";
const char* vTexShader = "src/TextureVertexShader.glsl";
const char* fTexShader = "src/TextureFragmentShader.glsl";
const char* vTexMatShader = "sry/TextureMatrixVertexShader.glsl";
const char* fTexMatShader = "sry/TextureMatrixFragmentShader.glsl";


GLFWwindow* window;

#pragma region keyManagement
//-----------------------------------------
// This Areaa is for the management of keyboardinputs with GLFW
//-----------------------------------------
int __keySituation[GLFW_KEY_LAST+1];
int __oldSituation[GLFW_KEY_LAST+1];
bool checkKeys[GLFW_KEY_LAST + 1]
{ 0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,1,0,0,0,0,0,0,1,
0,0,0,0,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,0,1,
0,1,0,0,0,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,
1,1,1,1,0,0,1,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,1,1,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,
0,0,0,0,0,0,0,0,0,0,
1,1,1,1,1,0,0,0,0,0,
1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,0,0,0,0,0,
1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,0,0,0,
1,1,1,1,1,1,1,1,1 };

//always use this method before using any of the other methods

void fillKey(){
	for (int i = 0; i <= GLFW_KEY_LAST; i++) {
		if (checkKeys[i]) {
			__oldSituation[i] = __keySituation[i];
			__keySituation[i] = glfwGetKey(window, i);
		}
	}
}

bool isKeyDown(int key) {
	return __keySituation[key] == GLFW_PRESS;
}

bool isKeyUp(int key) {
	return __keySituation[key] == GLFW_RELEASE;
}

bool isKeyPressed(int key) {
	return __keySituation[key] == GLFW_PRESS && __oldSituation[key] == GLFW_RELEASE;
}

bool isKeyRelease(int key) {
	return __keySituation[key] == GLFW_RELEASE && __oldSituation[key] == GLFW_PRESS;
}

//-----------------------------------------
// Ende Keyboardinputmanagement
//-----------------------------------------
#pragma endregion


//Thresholding Segmentation of a Mat with a uchar* of size width*height as return value, which represents the binary segment
//width and height of the picture
//RGB and up describe the threshold
//up = true --> [R;255]
//up = false --> [0;R]
//segIndex1 is set, if RGB of picture within range
//segIndex2 is set, if RGB of picture outside of range
uchar* thresholding3C(Mat img, int width, int height, uchar R, uchar G, uchar B, bool up, int segIndex1, int segIndex2) {
	uchar* ret = new uchar [width*height];
	uchar* data = img.data;

	for (int i = 0; i < width*height; i++) {
		if (up) {
			if (data[i*3] >= R && data[i * 3 + 1] >= G && data[i * 3 + 2] >= B) {
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
//see thrsholding3C
//Input is uchar*
uchar* thresholding3C(uchar* data, int width, int height, uchar R, uchar G, uchar B, bool up, int segIndex1, int segIndex2) {
	uchar* ret = new uchar[width*height];

	for (int i = 0; i < width*height; i++) {
		if (up) {
			if (data[i * 3] >= B && data[i * 3 + 1] >= G && data[i * 3 + 2] >= R) {
				ret[i] = segIndex2;
			}
			else {
				ret[i] = segIndex1;
			}
		}
		else {
			if (data[i * 3] <= B && data[i * 3 + 1] <= G && data[i * 3 + 2] <= R) {
				ret[i] = segIndex2;
			}
			else {
				ret[i] = segIndex1;
			}
		}
	}

	return ret;
}

//seperates segment from picture with binary segment and segIndex
//returns uchar* of size 3*width*height
//width and height of picture
uchar* getSegmentFromImg3C(Mat image, int width, int height, uchar* segment, uchar segIndex) {
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
//see getSegmentFromImg3C
//Input is uchar*
uchar* getSegmentFromImg3C(uchar* imgData, int width, int height, uchar* segment, uchar segIndex) {
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

//see getSegmentMatFromImg3C
//Return is OpenCV::Mat with 4 channels and optimized size
Mat getSegmentMatFromImg3C(Mat image, int width, int height, uchar* segment, uchar segIndex) {
	//--------------------------------------------------------------------------------------------------------
	//-------------------------------------- Optimization Calculations start ---------------------------------
	//--------------------------------------------------------------------------------------------------------
	
	
	// - Cropping and getting Dimensions
	int minx = 0, maxx = width;
	int miny = 0, maxy = height;
	bool goon = true;
	// --- get miny

	for (int y = 0; y < height && goon; y++) {
		for (int x = 0; x < width && goon; x++) {
			if (segment[x + y * width] == segIndex) {
				miny = y;
				goon = false;
			}
		}
	}
	goon = true;
	// --- get maxy
	for (int y = height; y >= 0 && goon; y--) {
		for (int x = 0; x < width && goon; x++) {
			if (segment[x + y * width] == segIndex) {
				miny = y;
				goon = false;
			}
		}
	}
	goon = true;
	// --- get minx
	for (int x = 0; x < width && goon; x++) {
		for (int y = 0; y < height && goon; y++) {
			if (segment[x + y * width] == segIndex) {
				minx = x;
				goon = false;
			}
		}
	}
	goon = true;
	// --- get maxx
	for (int x = width; x >= 0 && goon; x--) {
		for (int y = 0; y < height && goon; y++) {
			if (segment[x + y * width] == segIndex) {
				minx = x;
				goon = false;
			}
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
//see getSegmentMatFromImg3C
//Input is uchar* and return is uchar* with optimized size
uchar* getSegmentMatFromImg3C(uchar* imgData, int width, int height, uchar* segment, uchar segIndex, int *outWidth, int *outHeight, int* startx, int* starty) {
	//--------------------------------------------------------------------------------------------------------
	//-------------------------------------- Optimization Calculations start ---------------------------------
	//--------------------------------------------------------------------------------------------------------


	// - Cropping and getting Dimensions
	int minx = 0, maxx = width;
	int miny = 0, maxy = height;
	bool goon = true;
	// --- get miny

	for (int y = 0; y < height && goon; y++) {
		for (int x = 0; x < width && goon; x++) {
			if (segment[x + y * width] == segIndex) {
				miny = y;
				goon = false;
			}
		}
	}
	goon = true;
	// --- get maxy
	for (int y = height; y >= 0 && goon; y--) {
		for (int x = 0; x < width && goon; x++) {
			if (segment[x + y * width] == segIndex) {
				maxy = y;
				goon = false;
			}
		}
	}
	goon = true;
	// --- get minx
	for (int x = 0; x < width && goon; x++) {
		for (int y = 0; y < height && goon; y++) {
			if (segment[x + y * width] == segIndex) {
				minx = x;
				goon = false;
			}
		}
	}
	goon = true;
	// --- get maxx
	for (int x = width; x >= 0 && goon; x--) {
		for (int y = 0; y < height && goon; y++) {
			if (segment[x + y * width] == segIndex) {
				maxx = x;
				goon = false;
			}
		}
	}

	int inWidth = maxx - minx;
	int inHeight = maxy - miny;

	//--------------------------------------------------------------------------------------------------------
	//-------------------------------------- Optimization Calculations stop ----------------------------------
	//--------------------------------------------------------------------------------------------------------


	// - create Material with optimal size
	*outWidth = inWidth;
	*outHeight = inHeight;
	*startx = minx;
	*starty = miny;
	uchar* ret = new uchar[inHeight*inWidth * 4];

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

//return a OpenGL viewable binary segment
//works with uchar*
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
//return a OpenGL viewable binary segment
//works with OpenCV::Mat
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

//Reads shader from filePath and returns string
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

//This class is used for 3-dimensional vector calculation
//Late Note: should have used GLM::vec3 all the time instead of making this class
class Vector3 {
public:
	float x;
	float y;
	float z;

	static float dot(Vector3 vec1, Vector3 vec2) {
		return vec1.x * vec2.x + vec1.y * vec2.y + vec1.z * vec2.z;
	}
	static Vector3 cross(Vector3 vec1, Vector3 vec2) {
		Vector3 vec;
		vec.x = vec1.y * vec2.z - vec1.z * vec2.y;
		vec.y = vec1.z * vec2.x - vec1.x * vec2.z;
		vec.z = vec1.x * vec2.y - vec1.y * vec2.x;
		return vec;
	}
	static Vector3 add(Vector3 vec1, Vector3 vec2) {
		Vector3 vec;
		vec.x = vec1.x + vec2.x;
		vec.y = vec1.y + vec2.y;
		vec.z = vec1.z + vec2.z;
		return vec;
	}
	static Vector3 sub(Vector3 vec1, Vector3 vec2) {
		Vector3 vec;
		vec.x = vec1.x - vec2.x;
		vec.y = vec1.y - vec2.y;
		vec.z = vec1.z - vec2.z;
		return vec;
	}
	static Vector3 mul(Vector3 vec1, float a) {
		Vector3 vec;
		vec.x = vec1.x * a;
		vec.y = vec1.y * a;
		vec.z = vec1.z * a;
		return vec;
	}
	static Vector3 div(Vector3 vec1, float a) {
		Vector3 vec;
		vec.x = vec1.x / a;
		vec.y = vec1.y / a;
		vec.z = vec1.z / a;
		return vec;
	}

	void add(Vector3 vec1) {
		this->x = vec1.x + this->x;
		this->y = vec1.y + this->y;
		this->z = vec1.z + this->z;
	}
	void sub(Vector3 vec1) {
		this->x = this->x - vec1.x;
		this->y = this->y - vec1.y;
		this->z = this->z - vec1.z;
	}
	void mul(float a) {
		this->x = this->x * a;
		this->y = this->y * a;
		this->z = this->z * a;
	}
	void div(float a) {
		this->x = this->x / a;
		this->y = this->y / a;
		this->z = this->z / a;
	}
	float length() {
		return (float)(sqrt(x*x + y * y + z * z));
	}
	Vector3 direction() {
		Vector3 vec = *this;
		float length = this->length();
		if (length != 0) {
			vec.x = this->x / length;
			vec.y = this->y / length;
			vec.z = this->z / length;
		}
		return vec;
	}
	void set(float x, float y, float z) { 
		this->x = x; this->y = y; this->z = z; 
	};
	glm::vec3 GLM() {
		return glm::vec3(this->x, this->y, this->z);
	}


	Vector3(): x(0), y(0), z(0) {};
	Vector3(float x, float y, float z): x(x), y(y), z(z){};
	Vector3(glm::vec3 v) : x(v.x), y(v.y), z(v.z) {};
};
//This class contains all vectors needed to describe position, orientation and scale of a object
class Transform {
private:
	Vector3 position;
	Vector3 up;
	Vector3 front;
	//There will not be a setter for right, as this will always be calculated when up or right are set
	Vector3 right;
	Vector3 scale;

public:
	void setPosition(Vector3 position) {
		this->position.x = position.x;
		this->position.y = position.y;
		this->position.z = position.z;
	};
	void setPosition(float x, float y, float z) {
		this->position.x = x;
		this->position.y = y;
		this->position.z = z;
	};
	void setUp(Vector3 up) {
		this->up.x = up.x;
		this->up.y = up.y;
		this->up.z = up.z;
		this->right = Vector3::cross(up, front);
	};
	void setUp(float x, float y, float z) {
		up.x = x;
		up.y = y;
		up.z = z;
		this->right = Vector3::cross(up, front);
	}
	void setFront(Vector3 front) {
		this->front.x = front.x;
		this->front.y = front.y;
		this->front.z = front.z;
		this->right = Vector3::cross(up, front);
	};
	void setFront(float x, float y, float z) {
		front.x = x;
		front.y = y;
		front.z = z;
		this->right = Vector3::cross(up, front);
	}
	void setScale(Vector3 scale) {
		this->scale.x = scale.x;
		this->scale.y = scale.y;
		this->scale.z = scale.z;
	};
	void setScale(float x, float y, float z) {
		scale.x = x;
		scale.y = y;
		scale.z = z;
		
	};
	

	Vector3 getPosition() {	return position; };
	Vector3 getUp() { return up; };
	Vector3 getFront() { return front; };
	Vector3 getRight() { return right; };
	Vector3 getScale() { return scale; };

	Transform(Vector3 position, Vector3 up, Vector3 front, Vector3 scale) {
		this->position = position;
		this->up = up;
		this->setFront(front);
		this->scale = scale;
		
	}
	Transform(Vector3 position, Vector3 up, Vector3 front, Vector3 right, Vector3 scale) : position(position), up(up), front(front), right(right), scale(scale) {}
	Transform() : Transform(Vector3(0, 0, 0), Vector3(0, 1, 0), Vector3(0, 0, 1), Vector3(1, 0, 0), Vector3(1, 1, 1)) {};
};
//This class contains all needed to render a textured Quad
class TexturedQuad {
private:
	Transform transform;
	unsigned int VAO;
	unsigned int VBO;
	unsigned int EBO;
	unsigned int texture;
	uchar* data;
	int texWidth;
	int texHeight;
	int shader;
	bool visible = false;

	void initQuad(uchar* texture, int width, int height, GLint dstFormat, GLint srcFormat) {
		//This function Initializes everything necessary to draw a Textured Quad
		glGenBuffers(1, &(this->VBO));
		glGenBuffers(1, &(this->EBO));
		glGenVertexArrays(1, &(this->VAO));
		// - Bind VAO
		glBindVertexArray(this->VAO);
		// - Configure VBO
		// -- Bind VBO
		glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
		// -- set VBO Data
		glBufferData(GL_ARRAY_BUFFER, sizeof(this->vertices), this->vertices, GL_STATIC_DRAW);
		// - Configure EBO
		// -- Bind EBO
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
		// -- set EBO Data
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(this->indices), this->indices, GL_STATIC_DRAW);
		// - Set VAO-Parameters
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid *)(6 * sizeof(GLfloat)));
		glEnableVertexAttribArray(2);

		glGenTextures(1, &(this->texture));
		glBindTexture(GL_TEXTURE_2D, this->texture); 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, dstFormat, width, height, 0, srcFormat, GL_UNSIGNED_BYTE, texture);
		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, 0);



		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		//adjust scale of the Quad
		float maxScale = std::max(this->transform.getScale().x, this->transform.getScale().y);
		Vector3 s = this->transform.getScale();
		if (width > height) {
			s.x = maxScale;
			s.y = maxScale * height / width;
		}else {
			s.x = maxScale * width / height;
			s.y = maxScale;
		}
		this->transform.setScale(s);
		this->texWidth = width;
		this->texHeight = height;
	};

	
	
public:
	GLfloat vertices[32] = {
		//	local coords			color			texcoords
		//	x	y	z				R	G	B		x	y	
		-1, 1,	0,				1,	1,	1,		0,	0,	//top		left
		1,	1,	0,				1,	1,	1,		1,	0,	//top		right
		1,	-1,	0,				1,	1,	1,		1,	1,	//bottom	right
		-1,	-1,	0,				1,	1,	1,		0,	1	//bottom	left
	};

	GLuint indices[6] = {
		0,1,2,
		0,2,3
	};

	Transform getTransform() { return this->transform; };
	void setTransform(Transform transform) { this->transform = transform; };
	void setVisible(bool visible) { this->visible = visible; };
	bool getVisible() { return this->visible; }
	int getTexWidth() { return this->texWidth; }
	int getTexHeight() { return this->texHeight; }
	uchar* getData() { return this->data; }
	int getShaderProgram() { return this->shader; }

	void draw(glm::mat4 view, glm::mat4 projection) {
		if (visible) {
			glUseProgram(this->shader);
			Vector3 p = this->getTransform().getPosition();
			Vector3 s = this->getTransform().getScale();
			Vector3 f = this->getTransform().getFront().direction();
			Vector3 u = this->getTransform().getUp().direction();
			Vector3 r = this->getTransform().getRight().direction();
			glm::mat4 rot;
			rot[0] = glm::vec4(r.x, r.y, r.z, 0);
			rot[1] = glm::vec4(u.x, u.y, u.z, 0);
			rot[2] = glm::vec4(f.x, f.y, f.z, 0);
			rot[3] = glm::vec4(0, 0, 0, 1);
			glm::mat4 sca;
			sca[0] = glm::vec4(s.x, 0, 0, 0);
			sca[1] = glm::vec4(0, s.y, 0, 0);
			sca[2] = glm::vec4(0, 0, s.z, 0);
			sca[3] = glm::vec4(0, 0, 0, 1);
			glm::mat4 trans;
			trans[0] = glm::vec4(1, 0, 0, 0);
			trans[1] = glm::vec4(0, 1, 0, 0);
			trans[2] = glm::vec4(0, 0, 1, 0);
			trans[3] = glm::vec4(p.x, p.y, p.z, 1);
			glm::mat4 model = trans * rot * sca;
			int modelLoc = glGetUniformLocation(this->shader, "model");
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
			int viewLoc = glGetUniformLocation(this->shader, "view");
			glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
			int projectionLoc = glGetUniformLocation(this->shader, "projection");
			glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
			
			glActiveTexture(this->texture);
			glBindTexture(GL_TEXTURE_2D, this->texture);

			glBindVertexArray(this->VAO);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
			glBindVertexArray(0);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	};
	TexturedQuad() {}
	TexturedQuad(uchar* texture, int width, int height, GLint dstFormat, GLint srcFormat, int shaderProgram) {
		this->data = new uchar[3 * width * height * sizeof(uchar)];
		memcpy(this->data, texture, 3 * width * height * sizeof(uchar));
		this->visible = true;
		this->transform = Transform();
		this->shader = shaderProgram;
		initQuad(texture, width, height, dstFormat, srcFormat);
	};
	TexturedQuad(Mat texture, GLint dstFormat, GLint srcFormat, int shaderProgram) {
		this->data = new uchar[3 * texture.cols * texture.rows * sizeof(uchar)];
		memcpy(this->data, texture.datastart, 3 * texture.cols * texture.rows * sizeof(uchar));
		this->visible = true;
		this->transform = Transform();
		this->shader = shaderProgram;
		initQuad(texture.data, texture.cols, texture.rows, dstFormat, srcFormat);
	};
	TexturedQuad(uchar* texture, int width, int height, GLint dstFormat, GLint srcFormat, int shaderProgram, Transform transform) {
		this->data = new uchar[3 * width * height * sizeof(uchar)];
		memcpy(this->data, texture, 3 * width * height * sizeof(uchar));
		this->visible = true;
		this->transform = transform;
		this->shader = shaderProgram;
		initQuad(texture, width, height, dstFormat, srcFormat);
	};
	TexturedQuad(Mat texture, GLint dstFormat, GLint srcFormat, int shaderProgram, Transform transform) {
		this->data = new uchar[3 * texture.cols * texture.rows * sizeof(uchar)];
		memcpy(this->data, texture.datastart, 3 * texture.cols * texture.rows * sizeof(uchar));
		this->visible = true;
		this->transform = transform;
		this->shader = shaderProgram;
		initQuad(texture.data, texture.cols, texture.rows, dstFormat, srcFormat);
	};
	TexturedQuad(uchar* texture, int width, int height, GLint dstFormat, GLint srcFormat, int shaderProgram, Transform transform, bool visible) {
		this->data = new uchar[3 * width * height * sizeof(uchar)];
		memcpy(this->data, texture, 3 * width * height * sizeof(uchar));
		this->visible = visible;
		this->transform = transform;
		this->shader = shaderProgram;
		initQuad(texture, width, height, dstFormat, srcFormat);
	};
	TexturedQuad(Mat texture, GLint dstFormat, GLint srcFormat, int shaderProgram, Transform transform, bool visible) {
		this->data = new uchar[3 * texture.cols * texture.rows * sizeof(uchar)];
		memcpy(this->data, texture.datastart, 3 * texture.cols * texture.rows * sizeof(uchar));
		this->visible = visible;
		this->transform = transform;
		this->shader = shaderProgram;
		initQuad(texture.data, texture.cols, texture.rows, dstFormat, srcFormat);
	};
};
//This class contains all necessary information to make mono and stereoscopic rendering possible
class Camera {
private :
	Vector3 position;
	Vector3 look;
	Vector3 up;
	float near;
	float far;
	float width;
	float height;
	float aspect;
	float fov;
	bool stereo;
	float eyeSep;
	float focus;


public:

	void setPosition(Vector3 position) { this->position = position; };
	void setLook(Vector3 look) { this->look = look; };
	void setUp(Vector3 up) { this->up = up; };
	void setNear(float near) { this->near = near; };
	void setFar(float far) { this->far = far; };
	void setDims(float width, float height) {
		this->width = width;
		this->height = height;
		this->aspect = width / height;
	}
	void setfov(float fov) { this->fov = fov; };
	void setStereo(float eyeSep, float focus) {
		this->stereo = true;
		this->eyeSep = eyeSep;
		this->focus = focus;
	}
	void turnOnStereo() {
		this->stereo = true;
	}
	void turnOffStereo() {
		this->stereo = false;
	}
	void setFocus(float focus) { this->focus = focus; };
	void setEyeSep(float eyeSep) { this->eyeSep = eyeSep; };

	Vector3 getPosition() { return this->position; };
	Vector3 getLook() { return this->look; };
	Vector3 getLookDir() {
		Vector3 lookDir(this->look.x - this->position.x, this->look.y - this->position.y, this->look.z - this->position.z);
		lookDir = lookDir.direction();
		return lookDir;}
	Vector3 getUp() { return this->up; };
	Vector3 getRight() {
		Vector3 lookDir(this->look.x - this->position.x, this->look.y - this->position.y, this->look.z - this->position.z);
		Vector3 right;
		right = Vector3::cross(lookDir, this->up).direction();
		return right;
	}
	float getNear() { return this->near; }
	float getFar() { return this->far; }
	float getWidth() { return this->width; }
	float getHeight() { return this->height; }
	float getAspect() { return this->aspect; }
	float getFov() { return this->fov; }
	float getFovRad() { return this->fov * (float)acos(-1) / 180; }
	bool isStereo() { return this->stereo; }
	float getEyeSep() { return this->eyeSep; }
	float getFocus() { return this->focus; }


	glm::vec3 getPositionGLM() { return glm::vec3(this->position.x, this->position.y, this->position.z); }
	glm::vec3 getLookGLM() { return glm::vec3(this->look.x, this->look.y, this->look.z); }
	glm::vec3 getUpGLM() { return glm::vec3(this->up.x, this->up.y, this->up.z); }
	glm::vec3 getRightGLM() {
		glm::vec3 lookDir(this->look.x - this->position.x, this->look.y - this->position.y, this->look.z - this->position.z);
		glm::vec3 right;
		right = glm::cross(lookDir, this->getUpGLM());
		glm::normalize(right);
		return right;
	}

	Camera() {
		this->position = Vector3(0,0,10);
		this->look = Vector3(0, 0, 0);
		this->up = Vector3(0, 1, 0);
		this->near = 1;
		this->far = 10000;
		this->width = 800;
		this->height = 600;
		this->fov = 40;
		this->stereo = false;
		this->eyeSep = 30;
		this->focus = 1;
	}
	Camera(Vector3 pos, Vector3 look, Vector3 up, float near, float far, float width, float height, float fov) {
		this->position = pos;
		this->look = look;
		this->up = up;
		this->near = near;
		this->far = far;
		this->width = width;
		this->height = height;
		this->aspect = width / height;
		this->fov = fov;
		this->stereo = false;
	}
	Camera(Vector3 pos, Vector3 look, Vector3 up, float near, float far, float width, float height, float fov, float eyeSep, float focus) {
		this->position = pos;
		this->look = look;
		this->up = up;
		this->near = near;
		this->far = far;
		this->width = width;
		this->height = height;
		this->aspect = width / height;
		this->fov = fov;
		this->stereo = true;
		this->eyeSep = eyeSep;
		this->focus = focus;
	}

};



#pragma region UnitTests
//--------------------------------------
// This area contains UnitTests to test the classes created for this programm
//--------------------------------------
bool checkVector(Vector3 v, float x, float y, float z, string test) {
	if (v.x == x && v.y == y && v.z == z) {
		cout << "SUCCESS!! " << test << " did work" << endl;
		return true;
	}
	else {
		cout << "ERROR!!! " << test << " did not work" << endl;
		return false;
	}
}
bool checkFloat(float a, float b, string test) {
	if (a == b) {
		cout << "SUCCESS!! " << test << " did work" << endl;
		return true;
	}
	else {
		cout << "ERROR!!! " << test << " did not work" << endl;
		return false;
	}
}
bool checkFloatRange(float a, float b, float margin, string test) {
	if (abs(a - b) < abs(margin)) {
		cout << "SUCCESS!! " << test << " did work" << endl;
		return true;
	}
	else {
		cout << "ERROR!!! " << test << " did not work" << endl;
		return false;
	}
}
bool checkBool(bool b, string test) {
	if (b) {
		cout << "SUCCESS!! " << test << " did work" << endl;
		return true;
	}
	else {
		cout << "ERROR!!! " << test << " did not work" << endl;
		return false;
	}
}
bool checkVector(Vector3 v, float x, float y, float z) {
	if (v.x == x && v.y == y && v.z == z) {
		return true;
	}
	else {
		return false;
	}
}
bool checkFloat(float a, float b) {
	if (a == b) {
		return true;
	}
	else {
		return false;
	}
}
bool checkFloatRange(float a, float b, float margin) {
	if (abs(a - b) < abs(margin)) {
		return true;
	}
	else {
		return false;
	}
}
bool checkBool(bool b) {
	if (b) {
		return true;
	}
	else {
		return false;
	}
}
//This method is just to test wether the classes work as intended
void testClasses() {
	cout << endl << "Tests start" << endl;
	checkVector(Vector3(), 0, 0, 0, "Check Vector");
	cout << endl << "Testing Vectors" << endl;
	Vector3 testvec;
	checkVector(testvec, 0, 0, 0, "Standard Initialization");
	Vector3 testvec2(1, 2, 3); 
	checkVector(testvec2, 1, 2, 3, "Three Argument Initialization");
	testvec.set(1, 1, 1);
	checkVector(testvec, 1, 1, 1, "Vector Setting");
	float resf = testvec.length(); //sqrt(3) ~ 1.73205
	checkFloatRange(resf, 1.73205f, 0.00001f, "Length");
	resf = Vector3::dot(testvec, testvec); //3
	checkFloatRange(resf, 3.f, 0.00001f, "Dot Product");
	testvec2.set(-1, 1, 1);
	Vector3 res = Vector3::cross(testvec, testvec2);
	checkVector(res, 0, -2, 2, "Cross Product");
	res = Vector3::add(testvec, testvec2);
	checkVector(res, 0, 2, 2, "static Addition");
	res = Vector3::sub(testvec, testvec2);
	checkVector(res, 2, 0, 0, "static Subtraction");
	res = Vector3::mul(testvec, 2);
	checkVector(res, 2, 2, 2, "static Multiplication");
	res = Vector3::div(testvec, 2);
	checkVector(res, 0.5, 0.5, 0.5, "static Division");
	testvec.add(testvec);
	checkVector(testvec, 2, 2, 2, "Member Addition");
	testvec2.set(1, 1, 1);
	testvec.sub(testvec2);
	checkVector(testvec2, 1, 1, 1, "Member Subraction");
	testvec.mul(2);
	checkVector(testvec, 2, 2, 2, "Member Multiplication");
	testvec.div(2);
	checkVector(testvec, 1, 1, 1, "Member Division");
	testvec.set(5, 0, 0);
	res = testvec.direction();
	checkVector(res, 1, 0, 0, "Direction");
	cout << "Vector Test finished" << endl << endl;

	cout << "Testing Transform" << endl;
	Transform trans1(Vector3(0,0,0), Vector3(0,1,0), Vector3(0,0,1), Vector3(1,0,0), Vector3(1,1,1));
	bool resb = true;
	resb = resb && checkVector(trans1.getPosition(), 0, 0, 0, "GetPosition");
	resb = resb && checkVector(trans1.getUp(), 0, 1, 0, "GetUp");
	resb = resb && checkVector(trans1.getFront(), 0, 0, 1, "GetFront");
	resb = resb && checkVector(trans1.getRight(), 1, 0, 0, "GetRight");
	resb = resb && checkVector(trans1.getScale(), 1, 1, 1, "GetScale");
	checkBool(resb, "Getters");
	checkBool(resb, "Quintuple Initialization");
	trans1.setPosition(Vector3(1, 2, 3));
	checkVector(trans1.getPosition(), 1, 2, 3, "Set Position with Vector");
	trans1.setPosition(3,2,1);
	checkVector(trans1.getPosition(), 3,2,1, "Set Position with Floats");
	trans1.setUp(Vector3(0, 2, 0));
	checkBool(checkVector(trans1.getUp(), 0, 2, 0) && checkVector(trans1.getRight(), 2, 0, 0), "Set Up with Vector");
	trans1.setUp(0,4,0);
	checkBool(checkVector(trans1.getUp(), 0, 4, 0) && checkVector(trans1.getRight(), 4, 0, 0), "Set Up with Float");
	trans1.setFront(Vector3(2, 0, 0));
	checkBool(checkVector(trans1.getFront(), 2, 0, 0) && checkVector(trans1.getRight(), 0, 0, -8), "Set Front with Vector");
	trans1.setFront(1, 0, 0);
	checkBool(checkVector(trans1.getFront(), 1, 0, 0) && checkVector(trans1.getRight(), 0, 0, -4), "Set Front with Float");
	trans1.setScale(Vector3(1, 2, 3));
	checkVector(trans1.getScale(), 1, 2, 3, "Set Scale with Vector");
	trans1.setScale(3, 2, 1);
	checkVector(trans1.getScale(), 3, 2, 1, "Set Scale with Floats");

	trans1 = Transform(Vector3(1, 2, 3), Vector3(1, 0, 0), Vector3(0, 0, 1), Vector3(2, 1, 3));
	checkBool(checkVector(trans1.getPosition(), 1,2,3) &&
		checkVector(trans1.getUp(), 1,0,0) &&
		checkVector(trans1.getFront(), 0,0,1) &&
		checkVector(trans1.getRight(), 0,-1,0) &&
		checkVector(trans1.getScale(), 2, 1,3),
		"Quadruple Initialization");

	trans1 = Transform();
	checkBool(checkVector(trans1.getPosition(), 0, 0, 0) &&
		checkVector(trans1.getUp(), 0, 1, 0) &&
		checkVector(trans1.getFront(), 0, 0, 1) &&
		checkVector(trans1.getRight(), 1, 0, 0) &&
		checkVector(trans1.getScale(), 1, 1, 1),
		"Standard Initialization");
	cout << "Transform Test finished" << endl;
}

#pragma endregion


void start();
void draw();
void update();


void drawSelection(glm::mat4 view, glm::mat4 projection);
void setupSelection();


void printInfo();

milliseconds curTime, lastFrameTime;

int sp;
TexturedQuad texQuads[3];
Camera cam;
float sfocus = 5;
float sfov = 47.f;
float seyeSep = 0.05;
string windowName = "Integration von 3D-Effekten auf 2D Bilder";
const char* vsf = "src/TextureMatrixVertexShader.glsl";
const char* fsf = "src/TextureMatrixFragmentShader.glsl";
const char* simplevsf = "src/VertexShader.glsl";
const char* simplefsf = "src/FragmentShader.glsl";
bool debugMode = false;
bool pickSquare = false;
bool pickColor = false;
bool pickedSquare = false;
bool pickedColor = false;
bool waitRelease = false;
bool seperate = false;
bool seperated = false;
Vector3 targetPos;
double pickcoords[4];
double texcoords[4];
double pickcolor[3]{ 255, 255, 255 };
bool thresholdDir = false; // false = <; true = >=
double mouseX;
double mouseY;
GLFWcursor* standCursor;
GLFWcursor* pickCursor;

unsigned int sVAO;
unsigned int sVBO;
unsigned int sEBO;
int sshader;
float selectcoords[12]{0.0,0.0,0.01, 0.0,0.0,0.01, 0.0,0.0,0.01, 0.0,0.0,0.01};
int selectindex[6]{ 0,1,2,0,2,3 };

void error_callback(int error, const char* description) {
	fprintf(stderr, "Error: %s\n", description);
}

static void kCallBack(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
}


static void sCallBack(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
	cam.setDims(width, height);
}

//Initialize the scene
void start() {
	//Setup GLFW environments and windows
	glfwSetErrorCallback(error_callback);
	if (!glfwInit()) {
		cout << "Failed to initialize GLFW" << endl;
		return;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	int width = 600;
	int height = 400;
	window = glfwCreateWindow(width, height, "OpenGL", NULL, NULL);
	if (!window) {
		cout << "Failed to create Window" << endl;
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwSetKeyCallback(window, kCallBack);
	glfwSetWindowSizeCallback(window, sCallBack);

	glfwMakeContextCurrent(window);

	//Set Camera (it is up here for fov and focus uses further in this function)
	cam = Camera(Vector3(0, 0, 5), Vector3(0, 0, 0), Vector3(0, 1, 0), 1.0f, 100.f, (float)width, (float)height, sfov, seyeSep, sfocus);
	cam.turnOffStereo();


	//Start GLEW
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		cout << "Failed to initialize glew" << endl;
		return;
	}

	//Setup OpenGL and Buffer Objects
	// - Get ShaderCode
	string *vShaderCode = getShaderFromFile(vsf);
	const char* vShaderCodeChar = (*vShaderCode).c_str();
	string *fShaderCode = getShaderFromFile(fsf);
	const char* fShaderCodeChar = (*fShaderCode).c_str();
	// - Shader Compilieren
	// -- Vertex Shader
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
	// -- Fragment Shader
	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fShaderCodeChar, NULL);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << endl;
	}
	// - Link Shaders
	// -- create Program, attach, link
	sp = glCreateProgram();
	glAttachShader(sp, vertexShader);
	glAttachShader(sp, fragmentShader);
	glLinkProgram(sp);
	glGetProgramiv(sp, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(sp, 512, NULL, infoLog);
		cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << endl;
	}

	// - Clean Up Shaders
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	delete(vShaderCode);
	delete(fShaderCode);

	
	// - get and set texture
	string filename("data/Logo.png");
	Mat image = imread(filename, IMREAD_UNCHANGED);
	if (image.empty()) // Check for invalid input
	{
		cout << "Could not open or find the image" << endl;
		return;
	}
	float scale = tan(cam.getFovRad() / 2) * cam.getFocus();
	Transform trans(Vector3(0, 0, 0), Vector3(0, 1, 0), Vector3(0, 0, 1), Vector3(scale, scale, 1));
	texQuads[0] = TexturedQuad(image, GL_RGB, GL_BGR, sp, trans, true);
	texcoords[0] = 0;
	texcoords[1] = 0;
	texcoords[2] = texQuads[0].getTexWidth();
	texcoords[3] = texQuads[0].getTexHeight();

	image.release();

	
	//Set Cursors
	standCursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
	pickCursor = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
	glfwSetCursor(window, standCursor);

	//Enable Depth test
	glEnable(GL_DEPTH_TEST);
	glfwSwapInterval(1);
	//Main-Loop

	printInfo();
	while (!glfwWindowShouldClose(window)) {
		update();
		draw();
	}
	//Setdown OpenGL
	//Setdown GLFW and windows
	glfwDestroyWindow(window);

	glfwTerminate();
}

//Draw things
void draw() {
	//calculate View and Projection Matrix depending on Camera
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	//initial clear of buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if(cam.isStereo()){
		//stereoscopic camera
		//precalculate later used variables
		float L, R, B, T;
		float dispo = cam.getEyeSep() * 0.5f * cam.getNear() / cam.getFocus();
		glm::vec3 camMove = -cam.getRight().direction().GLM();
		camMove = camMove * cam.getEyeSep() * 0.5f;
		
		//Left eye
		T = tan(cam.getFovRad() / 2) * cam.getNear();
		B = -T;
		L = (B * cam.getAspect()) + dispo;
		R = (T * cam.getAspect()) + dispo;
		
		//red colormask
		glColorMask(1, 0, 0, 1);

		glm::mat4 view = glm::lookAt(cam.getPositionGLM() + camMove, cam.getLook().GLM() + camMove, cam.getUpGLM());
		glm::mat4 projection = glm::frustum(L, R, B, T, cam.getNear(), cam.getFar());

		texQuads[0].draw(view, projection);
		texQuads[1].draw(view, projection);
		texQuads[2].draw(view, projection);
		drawSelection(view, projection);

		//Right eye
		glClear(GL_DEPTH_BUFFER_BIT);

		T = tan(cam.getFovRad() / 2) * cam.getNear();
		B = -T;
		L = (B * cam.getAspect()) - dispo;
		R = (T * cam.getAspect()) - dispo;
		
		//cyan colormask
		glColorMask(0, 1, 1, 1);

		view = glm::lookAt(cam.getPosition().GLM() - camMove, cam.getLook().GLM() - camMove, cam.getUpGLM());
		projection = glm::frustum(L, R, B, T, cam.getNear(), cam.getFar());

		texQuads[0].draw(view, projection);
		texQuads[1].draw(view, projection);
		texQuads[2].draw(view, projection);
		drawSelection(view, projection);

		//reset colormask
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	}else {
		//monoscopic camera
		glm::mat4 view = glm::lookAt(cam.getPositionGLM(), cam.getPosition().GLM() + cam.getLookDir().GLM(), cam.getUpGLM());
		glm::mat4 projection = glm::perspective(cam.getFovRad(), cam.getAspect(), cam.getNear(), cam.getFar());

		texQuads[0].draw(view, projection);
		texQuads[1].draw(view, projection);
		texQuads[2].draw(view, projection);
		drawSelection(view, projection);
	}

	glfwSwapBuffers(window);
}

//manage Input
void update() {
	glfwPollEvents();
	fillKey();


	if (!debugMode) {
		if (pickSquare) {
			if (waitRelease) {
				//calculations for selection square
				glfwGetCursorPos(window, &mouseX, &mouseY);
				pickcoords[2] = mouseX;
				pickcoords[3] = mouseY;
				float distance = cam.getPosition().z - texQuads[0].getTransform().getPosition().z;
				float vertBorder = tan(cam.getFovRad() / 2) * distance;
				float horiBorder = vertBorder * cam.getWidth() / cam.getHeight();
				float windowW = cam.getWidth();
				float windowH = cam.getHeight();
				float windowX2 = (2 * pickcoords[2] - windowW) / (windowW);
				float windowY2 = (2 * pickcoords[3] - windowH) / (-windowH);
				float x2 = windowX2 * horiBorder;
				float y2 = windowY2 * vertBorder;
				selectcoords[3] = x2;
				selectcoords[6] = x2;
				selectcoords[7] = y2;
				selectcoords[10] = y2;

				float maxTexW = texQuads[0].getTexWidth();
				float maxTexH = texQuads[0].getTexHeight();
				float maxQuadW = texQuads[0].getTransform().getScale().x;
				float maxQuadH = texQuads[0].getTransform().getScale().y;
				x2 = (x2 + maxQuadW) / (2 * maxQuadW);
				y2 = (y2 - maxQuadH) / (-2 * maxQuadH);
				x2 = x2 * maxTexW;
				y2 = y2 * maxTexH;
				texcoords[2] = max(min(x2, maxTexW), 0.0f);
				texcoords[3] = max(min(y2, maxTexH), 0.0f);

				if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
					pickSquare = false;
					pickedSquare = true;
					waitRelease = false;

					//order texcoords
					float temp = min(texcoords[0], texcoords[2]);
					texcoords[2] = max(texcoords[0], texcoords[2]);
					texcoords[0] = temp;
					temp = min(texcoords[1], texcoords[3]);
					texcoords[3] = max(texcoords[1], texcoords[3]);
					texcoords[1] = temp;
					glfwSetCursor(window, standCursor);
				}
				printInfo();
			}
			else {
				//calculation for selection square
				glfwGetCursorPos(window, &mouseX, &mouseY);
				pickcoords[0] = mouseX;
				pickcoords[1] = mouseY;
				float distance = cam.getPosition().z - texQuads[0].getTransform().getPosition().z;
				float vertBorder = tan(cam.getFovRad() / 2) * distance;
				float horiBorder = vertBorder * cam.getWidth() / cam.getHeight();
				float windowW = cam.getWidth();
				float windowH = cam.getHeight();
				float windowX1 = (2 * pickcoords[0] - windowW) / (windowW);
				float windowY1 = (2 * pickcoords[1] - windowH) / (-windowH);
				float x1 = windowX1 * horiBorder;
				float y1 = windowY1 * vertBorder;
				selectcoords[0] = x1;
				selectcoords[1] = y1;
				selectcoords[4] = y1;
				selectcoords[9] = x1;

				float maxTexW = texQuads[0].getTexWidth();
				float maxTexH = texQuads[0].getTexHeight();
				float maxQuadW = texQuads[0].getTransform().getScale().x;
				float maxQuadH = texQuads[0].getTransform().getScale().y;
				x1 = (x1 + maxQuadW) / (2 * maxQuadW);
				y1 = (y1 - maxQuadH) / (-2 * maxQuadH);
				x1 = x1 * maxTexW;
				y1 = y1 * maxTexH;
				texcoords[0] = max(min(x1, maxTexW), 0.0f);
				texcoords[1] = max(min(y1, maxTexH), 0.0f);
				if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
					pickSquare = true;
					pickedSquare = false;
					waitRelease = true;
				}
				printInfo();
			}
		}
		else if (pickColor) {
			if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
				//calculation from mouse to texture coordinates 
				pickColor = false;
				pickedColor = true;
				glfwGetCursorPos(window, &mouseX, &mouseY);
				float distance = cam.getPosition().z - texQuads[0].getTransform().getPosition().z;
				float vertBorder = tan(cam.getFovRad() / 2) * distance;
				float horiBorder = vertBorder * cam.getWidth() / cam.getHeight();
				float windowW = cam.getWidth();
				float windowH = cam.getHeight();
				float windowX1 = (2 * mouseX - windowW) / (windowW);
				float windowY1 = (2 * mouseY - windowH) / (-windowH);
				float x1 = windowX1 * horiBorder;
				float y1 = windowY1 * vertBorder;

				float maxTexW = texQuads[0].getTexWidth();
				float maxTexH = texQuads[0].getTexHeight();
				float maxQuadW = texQuads[0].getTransform().getScale().x;
				float maxQuadH = texQuads[0].getTransform().getScale().y;
				x1 = (x1 + maxQuadW) / (2 * maxQuadW);
				y1 = (y1 - maxQuadH) / (-2 * maxQuadH);
				x1 = x1 * maxTexW;
				y1 = y1 * maxTexH;
				uchar* data = texQuads[0].getData();
				int texcoord = 3*((int)max(min(x1, maxTexW), 0.0f) + (int)max(min(y1, maxTexH), 0.0f) * maxTexW);
				pickcolor[2] = data[texcoord + 0];
				pickcolor[1] = data[texcoord + 1];
				pickcolor[0] = data[texcoord + 2];
				glfwSetCursor(window, standCursor);
				printInfo();
			}
		}
		else { //no picking (standard mode)
			if (isKeyDown(GLFW_KEY_LEFT_CONTROL) &&
				//debug mode
				isKeyPressed(GLFW_KEY_D)) {
				debugMode = true;
				printInfo();
			}
			if (isKeyPressed(GLFW_KEY_1)) {
				//pick a area start
				pickSquare = true;
				seperated = false;
				seperate = false;
				glfwSetCursor(window, pickCursor);
				setupSelection();
				texQuads[0].setVisible(true);
				texQuads[1].setVisible(false);
				texQuads[2].setVisible(false);
				printInfo();
			}
			if (isKeyPressed(GLFW_KEY_2)) {
				//pick a color start
				pickColor = true;
				seperated = false;
				seperate = false;
				glfwSetCursor(window, pickCursor);
				texQuads[0].setVisible(true);
				texQuads[1].setVisible(false);
				texQuads[2].setVisible(false);
				printInfo();
			}
			if (isKeyPressed(GLFW_KEY_KP_ADD)) {
				//set range to [c;255]
				thresholdDir = true;
				printInfo();
			}
			if (isKeyPressed(GLFW_KEY_KP_SUBTRACT)) {
				//set range to [0;C]
				thresholdDir = false;
				printInfo();
			}
			if (isKeyPressed(GLFW_KEY_S)) {
				//toggle stereo
				if (cam.isStereo()) cam.turnOffStereo();
				else cam.turnOnStereo();

				printInfo();
			}
			if ((isKeyPressed(GLFW_KEY_ENTER) || isKeyPressed(GLFW_KEY_KP_ENTER)) 
				&& !(seperate || seperated)) {
				//start with the segmentation
				int width = (int)texcoords[2] - (int)texcoords[0];
				int height = (int)texcoords[3] - (int)texcoords[1];
				int oWidth = texQuads[0].getTexWidth();
				int oHeight = texQuads[0].getTexHeight();
				int bounds[4]{ (int)texcoords[0], (int)texcoords[1], (int)texcoords[2], (int)texcoords[3] };
				uchar* tex = texQuads[0].getData();
				uchar* area;
				uchar* binSeg1;
				uchar* binSeg2;
				uchar* seg1;
				uchar* seg2;
				int texWidth;
				int texHeight;
				int startx;
				int starty;
				float quadWidth = texQuads[0].getTransform().getScale().x;
				float quadHeight = texQuads[0].getTransform().getScale().y;
				int segIndex1 = 0;
				int segIndex2 = 1;
				//create Area to work in
				area = new uchar[width * height * 3];
				for (int y = 0; y < height; y++) {
					for (int x = 0; x < width; x++) {
						area[(x + y * width) * 3] = tex[(bounds[0] + x + (bounds[1] + y)*oWidth) * 3];
						area[(x + y * width) * 3 + 1] = tex[(bounds[0] + x + (bounds[1] + y)*oWidth) * 3 + 1];
						area[(x + y * width) * 3 + 2] = tex[(bounds[0] + x + (bounds[1] + y)*oWidth) * 3 + 2];
					}
				}
				//Threshold
				binSeg1 = thresholding3C(area, width, height, pickcolor[0], pickcolor[1], pickcolor[2], thresholdDir, segIndex1, segIndex2);

				binSeg2 = new uchar[oWidth*oHeight];
				for (int y = 0; y < oHeight; y++) {
					for (int x = 0; x < oWidth; x++) {
						binSeg2[x + y*oWidth] = segIndex1;
					}
				}
				for (int y = 0; y < height; y++) {
					for (int x = 0; x < width; x++) {
						binSeg2[bounds[0] + x + (bounds[1] + y) * oWidth] = binSeg1[x + y * width];
					}
				}

				seg1 = getSegmentMatFromImg3C(area, width, height, binSeg1, segIndex2, &texWidth, &texHeight, &startx, &starty);

				float newX = (((bounds[0] + startx + texWidth / 2) / (float)oWidth) - 0.5f) * 2*quadWidth;
				float newY = (((bounds[1] + starty + texHeight / 2) / (float)oHeight) - 0.5f) * -2*quadHeight;
				float newSX = (texWidth / (float)oWidth) * quadWidth;
				float newSY = (texHeight / (float)oHeight) * quadHeight;

				Vector3 pos(newX, newY, texQuads[0].getTransform().getPosition().z);
				Vector3 up = texQuads[0].getTransform().getUp();
				Vector3 front = texQuads[0].getTransform().getFront();
				Vector3 scale(newSX, newSY, 1);
				Transform newTrans(pos, up, front, scale);
				texQuads[2] = TexturedQuad(seg1, texWidth, texHeight, GL_RGBA, GL_BGRA, texQuads[0].getShaderProgram(), newTrans);
				//Segment Direction
				Vector3 dir = cam.getPosition().GLM() - pos.GLM();

				//Vector3 dir = Vector3(0, 0, 1);

				targetPos = texQuads[2].getTransform().getPosition().GLM() + dir.direction().GLM() * 2.5f;

				seg2 = getSegmentMatFromImg3C(tex, oWidth, oHeight, binSeg2, segIndex1, &texWidth, &texHeight, &startx, &starty);

				newTrans = texQuads[0].getTransform();
				texQuads[1] = TexturedQuad(seg2, texWidth, texHeight, GL_RGBA, GL_BGRA, texQuads[0].getShaderProgram(), newTrans);

				//texQuads[0] -> invisible; texQuads[1,2] -> visible
				//texQuads[1] -> stationary Segment
				//texQuads[2] -> moving Segment
				texQuads[0].setVisible(false);
				texQuads[1].setVisible(true);
				texQuads[2].setVisible(true);
				seperate = true;
				pickedSquare = false;
				pickedColor = false;

				float sep = newSX * 2;
				if (cam.getEyeSep() != seyeSep) {
					cam.setEyeSep(seyeSep);
				}
				if (cam.getEyeSep() > sep) {
					cam.setEyeSep(sep);
				}

				printInfo();
			}

			if (seperate) {
				Vector3 currPos = texQuads[2].getTransform().getPosition();
				if (abs(targetPos.x - currPos.x) <= 0.01 &&
					abs(targetPos.y - currPos.y) <= 0.01 &&
					abs(targetPos.z - currPos.z) <= 0.01) {
					seperated = true;
					seperate = false;
					Transform trans = texQuads[2].getTransform();
					trans.setPosition(currPos);
					texQuads[2].setTransform(trans);
				} else {
					float s = 0.7;
					currPos.x = s * currPos.x + (1 - s) * targetPos.x;
					currPos.y = s * currPos.y + (1 - s) * targetPos.y;
					currPos.z = s * currPos.z + (1 - s) * targetPos.z;
					Transform trans = texQuads[2].getTransform();
					trans.setPosition(currPos);
					texQuads[2].setTransform(trans);
				}
			}
		}
	} else {
	//debug mode
		float speed = 0.1f;


		if (isKeyDown(GLFW_KEY_W)) {
			Vector3 move = cam.getLookDir();
			move.mul(speed);
			cam.setPosition(Vector3::add(cam.getPosition(), move));
			cam.setLook(Vector3::add(cam.getLook(), move));
			printInfo();
		}
		if (isKeyDown(GLFW_KEY_S)) {
			Vector3 move = cam.getLookDir();
			move.mul(-speed);
			cam.setPosition(Vector3::add(cam.getPosition(), move));
			cam.setLook(Vector3::add(cam.getLook(), move));
			printInfo();
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
			Vector3 move = cam.getRight();
			move.mul(-speed);
			cam.setPosition(Vector3::add(cam.getPosition(), move));
			cam.setLook(Vector3::add(cam.getLook(), move));
			printInfo();
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS &&
			glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) != GLFW_PRESS) {
			Vector3 move = cam.getRight();
			move.mul(speed);
			cam.setPosition(Vector3::add(cam.getPosition(), move));
			cam.setLook(Vector3::add(cam.getLook(), move));
			printInfo();
		}
		if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS) {
			debugMode = false;
			printInfo();
		}
		if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) {
			cam.setFocus(cam.getFocus() * 1.01);
			printInfo();
		}
		if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
			cam.setFocus(cam.getFocus() / 1.01);
			printInfo();
		}
		if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS) {
			cam.setEyeSep(cam.getEyeSep() * 1.01);
			printInfo();
		}
		if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) {
			cam.setEyeSep(cam.getEyeSep() / 1.01);
			printInfo();
		}
		if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) {
			cam.setNear(cam.getNear() * 1.01);
			printInfo();
		}
		if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
			cam.setNear(cam.getNear() / 1.01);
			printInfo();
		}
		if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
			cam.turnOffStereo();
			printInfo();
		}
		if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
			cam.turnOnStereo();
			printInfo();
		}		
		if (isKeyDown(GLFW_KEY_LEFT_CONTROL) &&
			//debug mode
			isKeyPressed(GLFW_KEY_D)) {
			debugMode = false;
			cam.setPosition(Vector3(0,0,5));
			cam.setLook(Vector3(0, 0, 0));
			printInfo();
		}
	}
}

void printInfo() {
	cout << string(100, '\n');
	if (!debugMode) {
		
		if (pickSquare) {
			if (!waitRelease) {
				cout << "Picked Window Coordinates: p1 = (" << mouseX << ", " << mouseY << ") " << endl;
				cout << "Picked Texture Coordinates: p1 = (" << texcoords[0] << ", " << texcoords[1] << ") " << endl;
			}else {
				cout << "Picked Window Coordinates: p1 = (" << pickcoords[0] << ", " << pickcoords[1] << "); p2 = (" << mouseX << ", " << mouseY << ")" << endl;
				cout << "Picked Texture Coordinates: p1 = (" << texcoords[0] << ", " << texcoords[1] << "); p2 = (" << texcoords[2] << ", " << texcoords[3] << ")" << endl;
			}
		}else { //no picking (standard mode)
			cout << "press '1' and choose the Area you want the segment to be chosen from (if not used, whole picture will be used for segmentation)" << endl;
			if (pickedSquare) {
				cout << "\tpicked Texture Coordinates: p1 = (" << texcoords[0] << ", " << texcoords[1] << "); p2 = (" << texcoords[2] << ", " << texcoords[3] << ")" << endl;
			}
			cout << "press '2' and pick a color on the rendered Texture to select for the threshold segmentation (if not used Color (255, 255, 255) will be used" << endl;
			if (pickedColor) {
				cout << "\t Picked Color: (" << pickcolor[0] << ", " << pickcolor[1] << ", " << pickcolor[2] << ")" << endl;
				if (thresholdDir) {
					cout << "\t Color Range : [" << pickcolor[0] << "; 255], [" << pickcolor[1] << "; 255], [" << pickcolor[2] << "; 255]" << endl;
				}
				else {
					cout << "\t Color Range : [0; " << pickcolor[0] << "], [0; " << pickcolor[1] << "], [0; " << pickcolor[2] << "]" << endl;
				}
			}
			cout << "press '+' on a Keypad to set the color range to 'greater equals' chosen color" << endl;
			cout << "press '-' on a Keypad to set the color range to 'less than' chosen color" << endl;
			cout << "press 'S' to toggle stereoscopic rendering" << endl;
			cout << "	stereoscopic rendering: ";
			if (cam.isStereo()) cout << "on" << endl;
			else cout << "off" << endl;
			cout << "press Enter to start the Segmentation and Seperation of the 2 Segments in 3D Space" << endl;
			cout << "press 'cntrl' + 'D' to start debug mode. Caution! No return from debug mode yet except for restarting program" << endl;
		}

	} else { //debug mode
		cout << "Camera Position: (" << cam.getPosition().x << ", " << cam.getPosition().y << ", " << cam.getPosition().z << ")" << endl;
		cout << "Camera Look: (" << cam.getLook().x << ", " << cam.getLook().y << ", " << cam.getLook().z << ")" << endl;
		cout << "Camera LookDir: (" << cam.getLookDir().x << ", " << cam.getLookDir().y << ", " << cam.getLookDir().z << ")" << endl;
		cout << "Camera Up : (" << cam.getUp().x << ", " << cam.getUp().y << ", " << cam.getUp().z << ")" << endl;
		cout << "Camera Right : (" << cam.getRight().x << ", " << cam.getRight().y << ", " << cam.getRight().z << ")" << endl;
		cout << "Tex Quad 0: " << endl;
		cout << "  Pos: (" << texQuads[0].getTransform().getPosition().x << ", " << texQuads[0].getTransform().getPosition().y << ", " << texQuads[0].getTransform().getPosition().z << ")" << endl;
		cout << "  Fro: (" << texQuads[0].getTransform().getFront().x << ", " << texQuads[0].getTransform().getFront().y << ", " << texQuads[0].getTransform().getFront().z << ")" << endl;
		cout << "  Up:  (" << texQuads[0].getTransform().getUp().x << ", " << texQuads[0].getTransform().getUp().y << ", " << texQuads[0].getTransform().getUp().z << ")" << endl;
		cout << "  Rig: (" << texQuads[0].getTransform().getRight().x << ", " << texQuads[0].getTransform().getRight().y << ", " << texQuads[0].getTransform().getRight().z << ")" << endl;
		cout << "  Sca: (" << texQuads[0].getTransform().getScale().x << ", " << texQuads[0].getTransform().getScale().y << ", " << texQuads[0].getTransform().getScale().z << ")" << endl;
		cout << "Tex Quad 1: " << endl;
		cout << "  Pos: (" << texQuads[1].getTransform().getPosition().x << ", " << texQuads[1].getTransform().getPosition().y << ", " << texQuads[1].getTransform().getPosition().z << ")" << endl;
		cout << "  Fro: (" << texQuads[1].getTransform().getFront().x << ", " << texQuads[1].getTransform().getFront().y << ", " << texQuads[1].getTransform().getFront().z << ")" << endl;
		cout << "  Up:  (" << texQuads[1].getTransform().getUp().x << ", " << texQuads[1].getTransform().getUp().y << ", " << texQuads[1].getTransform().getUp().z << ")" << endl;
		cout << "  Rig: (" << texQuads[1].getTransform().getRight().x << ", " << texQuads[1].getTransform().getRight().y << ", " << texQuads[1].getTransform().getRight().z << ")" << endl;
		cout << "  Sca: (" << texQuads[1].getTransform().getScale().x << ", " << texQuads[1].getTransform().getScale().y << ", " << texQuads[1].getTransform().getScale().z << ")" << endl;
		cout << "Tex Quad 2: " << endl;
		cout << "  Pos: (" << texQuads[2].getTransform().getPosition().x << ", " << texQuads[2].getTransform().getPosition().y << ", " << texQuads[2].getTransform().getPosition().z << ")" << endl;
		cout << "  Fro: (" << texQuads[2].getTransform().getFront().x << ", " << texQuads[2].getTransform().getFront().y << ", " << texQuads[2].getTransform().getFront().z << ")" << endl;
		cout << "  Up:  (" << texQuads[2].getTransform().getUp().x << ", " << texQuads[2].getTransform().getUp().y << ", " << texQuads[2].getTransform().getUp().z << ")" << endl;
		cout << "  Rig: (" << texQuads[2].getTransform().getRight().x << ", " << texQuads[2].getTransform().getRight().y << ", " << texQuads[2].getTransform().getRight().z << ")" << endl;
		cout << "  Sca: (" << texQuads[2].getTransform().getScale().x << ", " << texQuads[2].getTransform().getScale().y << ", " << texQuads[2].getTransform().getScale().z << ")" << endl;
		if (cam.isStereo()) {
			float L, R, B, T;
			float dispo = cam.getEyeSep() * 0.5f * cam.getNear() / cam.getFocus();
			glm::vec3 camMove = cam.getRightGLM() / cam.getRight().length();
			camMove = camMove * cam.getEyeSep() * 0.5f;

			//Left eye
			T = tan(cam.getFovRad() / 2) * cam.getNear();
			B = -T;
			L = (B * cam.getAspect()) + dispo;
			R = (T * cam.getAspect()) + dispo;
			cout << "Camera: Near: " << cam.getNear() << ", Far: " << cam.getFar() << endl;
			cout << "Camera: Focus: " << cam.getFocus() << ", EyeSep: " << cam.getEyeSep() << endl;
			cout << "Calculated Things: dispo: " << dispo << ", camMove: (" << camMove.x << ", " << camMove.y << ", " << camMove.z << ")" << endl;
			cout << "Camera1Frustum: T: " << T << ", B: " << B << ", L: " << L << ", R: " << R << endl;

			T = tan(cam.getFovRad() / 2) * cam.getNear();
			B = -T;
			L = (B * cam.getAspect()) - dispo;
			R = (T * cam.getAspect()) - dispo;
			cout << "Camera2Frustum: T: " << T << ", B: " << B << ", L: " << L << ", R: " << R << endl;
		}
	}
};




//opengl setup for selection square
void setupSelection() {
	string *vShaderCode = getShaderFromFile(simplevsf);
	const char* vShaderCodeChar = (*vShaderCode).c_str();
	string *fShaderCode = getShaderFromFile(simplefsf);
	const char* fShaderCodeChar = (*fShaderCode).c_str();
	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vShaderCodeChar, NULL);
	glCompileShader(vertexShader);
	int success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		cout << "ERROR::SHADER:VERTEX::COMPILATION::FAILED\n" << infoLog << endl;
	}
	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fShaderCodeChar, NULL);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << endl;
	}
	sshader = glCreateProgram();
	glAttachShader(sshader, vertexShader);
	glAttachShader(sshader, fragmentShader);
	glLinkProgram(sshader);
	glGetProgramiv(sshader, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(sshader, 512, NULL, infoLog);
		cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << endl;
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	delete(vShaderCode);
	delete(fShaderCode);
	glGenBuffers(1, &sVBO);
	glGenBuffers(1, &sEBO);
	glGenVertexArrays(1, &sVAO);
	glBindVertexArray(sVAO);
	glBindBuffer(GL_ARRAY_BUFFER, sVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(selectcoords), selectcoords, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(selectindex), selectindex, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

//drawing of selection square
void drawSelection(glm::mat4 view, glm::mat4 projection) {
	if ((pickSquare && waitRelease)|| pickedSquare) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glUseProgram(sshader); 
		int viewLoc = glGetUniformLocation(sshader, "view");
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		int projectionLoc = glGetUniformLocation(sshader, "projection");
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
		
		glBindBuffer(GL_ARRAY_BUFFER, sVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(selectcoords), selectcoords, GL_DYNAMIC_DRAW);

		glBindVertexArray(sVAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		glDisable(GL_BLEND);
	}
}



int main(int argc, char** argv)
{
	start();
	return 0;
}
