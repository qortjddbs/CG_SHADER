#define _CRT_SECURE_NO_WARNINGS //--- 프로그램 맨 앞에 선언할 것
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
//--- 필요한 헤더파일 선언
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <random>
#include <vector>
#include <cmath>
//--- 아래 5개 함수는 사용자 정의 함수임
void make_vertexShaders();
void make_fragmentShaders();
GLuint make_shaderProgram();
GLvoid drawScene();
GLvoid Reshape(int w, int h);
//--- 필요한 변수 선언
GLint width, height;
GLuint shaderProgramID; //--- 세이더 프로그램 이름
GLuint vertexShader; //--- 버텍스 세이더 객체
GLuint fragmentShader; //--- 프래그먼트 세이더 객체

std::random_device rd;  // 시드값을 얻기 위한 random_device 생성.
std::mt19937 gen(rd());	// random_device 를 통해 난수 생성 엔진을 초기화 한다.
std::uniform_real_distribution<float> dis_color(0.0f, 1.0f); // 0.0f 부터 1.0f 까지 균등하게 나타나는 난수열을 생성하기 위해 균등 분포 정의.


#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

int currentShape = 0;
int selectedShapeIndex = -1;

struct Shape {
	GLenum drawMode;
	int startIndex;
	int vertexCount;
	float centerX, centerY;
	int shapeType;
};

std::vector<GLfloat> allVertices;
std::vector<GLfloat> allColors;
std::vector<Shape> shapes;
GLuint vao, vbo[2];

float mapToGLCoordX(int x) {
	return (static_cast<float>(x) / (WINDOW_WIDTH / 2)) - 1.0f;
}

float mapToGLCoordY(int y) {
	return 1.0f - (static_cast<float>(y) / (WINDOW_HEIGHT / 2));
}

bool IsMouseInShape(float x, float y, const Shape& shape) {
	float dx = x - shape.centerX;
	float dy = y - shape.centerY;

	if (dx < 0) dx = -dx;
	if (dy < 0) dy = -dy;

	switch (shape.shapeType) {
		case 0: // 점
			return (dx <= 0.05f && dy <= 0.05f); // 반지름 0.05
		case 1: // 선
			return (dy <= 0.01f && dx <= 0.1f); // 선 중심 주변 영역
		case 2: { // 삼각형
			// 삼각형의 세 꼭짓점 (AddShape에서 정의된 좌표 기준)
			float x1 = shape.centerX;           // 위쪽 꼭짓점
			float y1 = shape.centerY + 0.15f;
			float x2 = shape.centerX - 0.15f;   // 왼쪽 아래 꼭짓점
			float y2 = shape.centerY - 0.1f;
			float x3 = shape.centerX + 0.15f;   // 오른쪽 아래 꼭짓점
			float y3 = shape.centerY - 0.1f;

			// 무게중심 좌표를 이용한 삼각형 내부 판별
			float denominator = (y2 - y3) * (x1 - x3) + (x3 - x2) * (y1 - y3);
			if (abs(denominator) < 0.0001f) return false; // 삼각형이 아닌 경우

			float a = ((y2 - y3) * (x - x3) + (x3 - x2) * (y - y3)) / denominator;
			float b = ((y3 - y1) * (x - x3) + (x1 - x3) * (y - y3)) / denominator;
			float c = 1 - a - b;

			return (a >= 0 && b >= 0 && c >= 0);
		}
		case 3: // 사각형
			return abs(dx) <= 0.1f && abs(dy) <= 0.1f; // 사각형 영역
	}
	return false;
}

//void MoveShape(int shapeIndex, float deltaX, float deltaY) {
//	if (shapeIndex < 0 || shapeIndex >= shapes.size()) return;
//
//	Shape& shape = shapes[shapeIndex];
//	int startVertex = shape.startIndex * 3;
//	int endVertex
//}

void UpdateBuffer() {
	if (allVertices.empty()) return;

	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, allVertices.size() * sizeof(GLfloat), allVertices.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, allColors.size() * sizeof(GLfloat), allColors.data(), GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
}

void AddShape(float x, float y, int shapeType) {
	Shape newShape;
	newShape.startIndex = allVertices.size() / 3;

	switch (shapeType) {
	case 0: // 점
		allVertices.push_back(x);
		allVertices.push_back(y);
		allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen));
		allColors.push_back(dis_color(gen));
		allColors.push_back(dis_color(gen));

		newShape.drawMode = GL_POINTS;
		newShape.vertexCount = 1;
		newShape.centerX = x;
		newShape.centerY = y;
		newShape.shapeType = 0;
		break;

	case 1: // 선
		allVertices.push_back(x - 0.1f);
		allVertices.push_back(y);
		allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen));
		allColors.push_back(dis_color(gen));
		allColors.push_back(dis_color(gen));
		allVertices.push_back(x + 0.1f);
		allVertices.push_back(y);
		allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen));
		allColors.push_back(dis_color(gen));
		allColors.push_back(dis_color(gen));
		newShape.drawMode = GL_LINES;
		newShape.vertexCount = 2;
		newShape.centerX = x;
		newShape.centerY = y;
		newShape.shapeType = 1;
		break;

	case 2: // 삼각형
		allVertices.push_back(x);
		allVertices.push_back(y + 0.15f);
		allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen));
		allColors.push_back(dis_color(gen));
		allColors.push_back(dis_color(gen));
		allVertices.push_back(x - 0.15f);
		allVertices.push_back(y - 0.1f);
		allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen));
		allColors.push_back(dis_color(gen));
		allColors.push_back(dis_color(gen));
		allVertices.push_back(x + 0.15f);
		allVertices.push_back(y - 0.1f);
		allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen));
		allColors.push_back(dis_color(gen));
		allColors.push_back(dis_color(gen));
		newShape.drawMode = GL_TRIANGLES;
		newShape.vertexCount = 3;
		newShape.centerX = x;
		newShape.centerY = y;
		newShape.shapeType = 2;
		break;

	case 3: // 사각형
		// 왼쪽 위
		allVertices.push_back(x - 0.1f);
		allVertices.push_back(y + 0.1f);
		allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen));
		allColors.push_back(dis_color(gen));
		allColors.push_back(dis_color(gen));
		// 오른쪽 위
		allVertices.push_back(x + 0.1f);
		allVertices.push_back(y + 0.1f);
		allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen));
		allColors.push_back(dis_color(gen));
		allColors.push_back(dis_color(gen));
		// 오른쪽 아래
		allVertices.push_back(x + 0.1f);
		allVertices.push_back(y - 0.1f);
		allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen));
		allColors.push_back(dis_color(gen));
		allColors.push_back(dis_color(gen));
		// 왼쪽 위
		allVertices.push_back(x - 0.1f);
		allVertices.push_back(y + 0.1f);
		allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen));
		allColors.push_back(dis_color(gen));
		allColors.push_back(dis_color(gen));
		// 왼쪽 아래
		allVertices.push_back(x - 0.1f);
		allVertices.push_back(y - 0.1f);
		allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen));
		allColors.push_back(dis_color(gen));
		allColors.push_back(dis_color(gen));
		// 오른쪽 아래
		allVertices.push_back(x + 0.1f);
		allVertices.push_back(y - 0.1f);
		allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen));
		allColors.push_back(dis_color(gen));
		allColors.push_back(dis_color(gen));
		newShape.drawMode = GL_TRIANGLES;
		newShape.vertexCount = 6;
		newShape.centerX = x;
		newShape.centerY = y;
		newShape.shapeType = 3;
		break;
	}

	shapes.push_back(newShape);
	UpdateBuffer();
}

char* filetobuf(const char* file)
{
	FILE* fptr;
	long length;
	char* buf;
	fptr = fopen(file, "rb"); // Open file for reading
	if (!fptr) // Return NULL on failure
		return NULL;
	fseek(fptr, 0, SEEK_END); // Seek to the end of the file
	length = ftell(fptr); // Find out how many bytes into the file we are
	buf = (char*)malloc(length + 1); // Allocate a buffer for the entire length of the file and a null terminator
	fseek(fptr, 0, SEEK_SET); // Go back to the beginning of the file
	fread(buf, length, 1, fptr); // Read the contents of the file in to the buffer
	fclose(fptr); // Close the file
	buf[length] = 0; // Null terminator
	return buf; // Return the buffer
}

//--- 그리기 콜백 함수
GLvoid drawScene()
{
	//--- 변경된 배경색 설정
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//--- 렌더링 파이프라인에 세이더 불러오기

	if (allVertices.empty()) {
		glutSwapBuffers();
		return;
	}

	//--- 렌더링 파이프라인에 세이더 불러오기
	glUseProgram(shaderProgramID);
	//--- 사용할 VAO 불러오기
	glBindVertexArray(vao);

	// 점 크기 설정
	glPointSize(10.0f);

	// 각 도형별로 그리기
	for (const auto& shape : shapes) {
		glDrawArrays(shape.drawMode, shape.startIndex, shape.vertexCount);
	}
	
	glutSwapBuffers(); //--- 화면에 출력하기
}

GLvoid Keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 'p':
		currentShape = 0; // 점
		break;
	case 'l':	// 우하향
		currentShape = 1; // 선
		if (selectedShapeIndex != -1) {
			Shape& shape = shapes[selectedShapeIndex];
			for (int i = 0; i < shape.vertexCount; ++i) {
				allVertices[(shape.startIndex + i) * 3 + 1] -= 0.05f; // y 좌표 감소
				allVertices[(shape.startIndex + i) * 3] += 0.05f; // x 좌표 감소
			}
			UpdateBuffer();
		}
		break;
	case 't':
		currentShape = 2; // 삼각형
		break;
	case 'r':
		currentShape = 3; // 사각형
		break;
	case 'w':
		if (selectedShapeIndex != -1) {
			Shape& shape = shapes[selectedShapeIndex];
			for (int i = 0; i < shape.vertexCount; ++i) {
				allVertices[(shape.startIndex + i) * 3 + 1] += 0.05f; // y 좌표 증가
			}
			UpdateBuffer();
		}
		break;
	case 's':
		if (selectedShapeIndex != -1) {
			Shape& shape = shapes[selectedShapeIndex];
			for (int i = 0; i < shape.vertexCount; ++i) {
				allVertices[(shape.startIndex + i) * 3 + 1] -= 0.05f; // y 좌표 감소
			}
			UpdateBuffer();
		}
		break;
	case 'd':
		if (selectedShapeIndex != -1) {
			Shape& shape = shapes[selectedShapeIndex];
			for (int i = 0; i < shape.vertexCount; ++i) {
				allVertices[(shape.startIndex + i) * 3] += 0.05f; // x 좌표 증가
			}
			UpdateBuffer();
		}
		break;
	case 'a':
		if (selectedShapeIndex != -1) {
			Shape& shape = shapes[selectedShapeIndex];
			for (int i = 0; i < shape.vertexCount; ++i) {
				allVertices[(shape.startIndex + i) * 3] -= 0.05f; // x 좌표 감소
			}
			UpdateBuffer();
		}
		break;
	case 'i':	// 좌상향
		if (selectedShapeIndex != -1) {
			Shape& shape = shapes[selectedShapeIndex];
			for (int i = 0; i < shape.vertexCount; ++i) {
				allVertices[(shape.startIndex + i) * 3] -= 0.05f; // x 좌표 감소
				allVertices[(shape.startIndex + i) * 3 + 1] += 0.05f; // y 좌표 증가
			}
			UpdateBuffer();
		}
		break;
	case 'j':	// 우상향
		if (selectedShapeIndex != -1) {
			Shape& shape = shapes[selectedShapeIndex];
			for (int i = 0; i < shape.vertexCount; ++i) {
				allVertices[(shape.startIndex + i) * 3] += 0.05f; // x 좌표 감소
				allVertices[(shape.startIndex + i) * 3 + 1] += 0.05f; // y 좌표 증가
			}
			UpdateBuffer();
		}
		break;
	case 'k':	// 좌하향
		if (selectedShapeIndex != -1) {
			Shape& shape = shapes[selectedShapeIndex];
			for (int i = 0; i < shape.vertexCount; ++i) {
				allVertices[(shape.startIndex + i) * 3] -= 0.05f; // x 좌표 감소
				allVertices[(shape.startIndex + i) * 3 + 1] -= 0.05f; // y 좌표 증가
			}
			UpdateBuffer();
		}
		break;
	case 'c':
		allVertices.clear();
		allColors.clear();
		shapes.clear();
		selectedShapeIndex = -1;
		break;
	}
	glutPostRedisplay(); //--- 배경색이 바뀔 때마다 출력 콜백 함수를 호출하여 화면을 refresh 한다
}

GLvoid Mouse(int button, int state, int x, int y)
{
	float Mouse_x = mapToGLCoordX(x);
	float Mouse_y = mapToGLCoordY(y);
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		if (shapes.size() < 10) AddShape(Mouse_x, Mouse_y, currentShape);
		else {
			selectedShapeIndex = -1;

			for (int i = shapes.size() - 1; i >= 0; --i) {
				if (IsMouseInShape(Mouse_x, Mouse_y, shapes[i])) {
					selectedShapeIndex = i;
					break;
				}
			}
		}
	}
	
	glutPostRedisplay(); //--- 배경색이 바뀔 때마다 출력 콜백 함수를 호출하여 화면을 refresh 한다
}

//--- 다시그리기 콜백 함수
GLvoid Reshape(int w, int h)
{
	glViewport(0, 0, w, h);
}

//--- 버텍스 세이더 객체 만들기
void make_vertexShaders()
{
	GLchar* vertexSource;
	//--- 버텍스 세이더 읽어 저장하고 컴파일 하기
	//--- filetobuf: 사용자정의 함수로 텍스트를 읽어서 문자열에 저장하는 함수
	vertexSource = filetobuf("vertex.glsl");
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, errorLog);
		std::cerr << "ERROR: vertex shader 컴파일 실패\n" << errorLog << std::endl;
		free(vertexSource); // 셰이더 함수들에서 메모리 해제
		return;
	}
	free(vertexSource); // 셰이더 함수들에서 메모리 해제
}

//--- 프래그먼트 세이더 객체 만들기
void make_fragmentShaders()
{
	GLchar* fragmentSource;
	//--- 프래그먼트 세이더 읽어 저장하고 컴파일하기
	fragmentSource = filetobuf("fragment.glsl"); // 프래그세이더 읽어오기
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
		std::cerr << "ERROR: frag_shader 컴파일 실패\n" << errorLog << std::endl;
		free(fragmentSource); // 셰이더 함수들에서 메모리 해제
		return;
	}
	free(fragmentSource); // 셰이더 함수들에서 메모리 해제
}

void InitBuffer()
{
	glGenVertexArrays(1, &vao); //--- VAO 를 지정하고 할당하기
	glBindVertexArray(vao); //--- VAO를 바인드하기
	glGenBuffers(2, vbo); //--- 2개의 VBO를 지정하고 할당하기

	// 초기에는 빈 버퍼로 설정
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
}

void main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정
{
	//--- 윈도우 생성하기
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Practice 8");
	//--- GLEW 초기화하기
	glewExperimental = GL_TRUE;
	glewInit();
	shaderProgramID = make_shaderProgram();
	InitBuffer();
	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
	glutMouseFunc(Mouse);
	glutKeyboardFunc(Keyboard); //--- 키보드 입력 콜백함수 지정
	glutMainLoop(); // 이벤트 처리 시작
}

//--- 세이더 프로그램 만들고 세이더 객체 링크하기
GLuint make_shaderProgram()
{
	// 먼저 셰이더들을 컴파일
	make_vertexShaders();
	make_fragmentShaders();

	GLint result;
	GLchar errorLog[512];
	GLuint shaderID;

	shaderID = glCreateProgram(); //--- 세이더 프로그램 만들기
	glAttachShader(shaderID, vertexShader); //--- 세이더 프로그램에 버텍스 세이더 붙이기
	glAttachShader(shaderID, fragmentShader); //--- 세이더 프로그램에 프래그먼트 세이더 붙이기
	glLinkProgram(shaderID); //--- 세이더 프로그램 링크하기

	glDeleteShader(vertexShader); //--- 세이더 객체를 세이더 프로그램에 링크했음으로, 세이더 객체 자체는 삭제 가능
	glDeleteShader(fragmentShader);

	glGetProgramiv(shaderID, GL_LINK_STATUS, &result); // ---세이더가 잘 연결되었는지 체크하기
	if (!result) {
		glGetProgramInfoLog(shaderID, 512, NULL, errorLog);
		std::cerr << "ERROR: shader program 연결 실패\n" << errorLog << std::endl;
		return 0;
	}

	return shaderID;
}