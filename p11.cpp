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
GLvoid TimerFunction(int value);
//--- 필요한 변수 선언
GLuint shaderProgramID; //--- 세이더 프로그램 이름
GLuint vertexShader; //--- 버텍스 세이더 객체
GLuint fragmentShader; //--- 프래그먼트 세이더 객체

std::random_device rd;  // 시드값을 얻기 위한 random_device 생성.
std::mt19937 gen(rd());	// random_device 를 통해 난수 생성 엔진을 초기화 한다.
std::uniform_real_distribution<float> dis_color(0.0f, 0.7f); // 0.0f 부터 1.0f 까지 균등하게 나타나는 난수열을 생성하기 위해 균등 분포 정의.
std::uniform_real_distribution<float> dis_x(-0.7f, 0.4f);
std::uniform_real_distribution<float> dis_y(-0.7f, 0.7f);

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

int shapeType = 0;
bool timerRunning = false;
float bgColorR = 0.0f;
float bgColorG = 0.0f;
float bgColorB = 0.0f;

struct Shape {
	GLenum drawMode;
	int startIndex;
	int vertexCount;
};

// 스파이럴 애니메이션을 위한 구조체
struct SpiralAnimation {
	float centerX, centerY;  // 스파이럴 중심점
	float angle;             // 현재 각도
	float radius;            // 현재 반지름
	bool active;             // 애니메이션 활성화 상태
	int pointCount;          // 현재까지 생성된 점의 개수
	int maxPoints;           // 최대 점 개수
	bool isExpanding;		// true : 확장 중, false : 수축 중
	float maxRadius;		// 최대 반지름
	bool hasPreviousPoint;	// 각 스파이럴별 이전 점 존재 여부
	float prevX, prevY;		// 각 스파이럴별 이전 점 좌표
};

std::vector<GLfloat> allVertices;
std::vector<GLfloat> allColors;
std::vector<Shape> shapes;
GLuint vao, vbo[2];

// 스파이럴 애니메이션 객체
std::vector<SpiralAnimation> spirals;

float mapToGLCoordX(int x) {
	return (static_cast<float>(x) / (WINDOW_WIDTH / 2)) - 1.0f;
}

float mapToGLCoordY(int y) {
	return 1.0f - (static_cast<float>(y) / (WINDOW_HEIGHT / 2));
}

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
	glClearColor(bgColorR, bgColorG, bgColorB, 1.0f);		// 흰색
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
	glPointSize(3.0f);

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
		shapeType = 0; // 점
		break;
	case 'l':
		shapeType = 1; // 선
		break;
	case '1':
		spirals.clear();
		spirals.push_back({
			dis_x(gen), dis_y(gen),
			0.0f,
			0.01f,
			true,
			0,
			64,
			true,
			0.3f,
			false,
			0.0f, 0.0f
			});
		bgColorR = dis_color(gen);
		bgColorG = dis_color(gen);
		bgColorB = dis_color(gen);

		if (!timerRunning) {
			timerRunning = true;
			glutTimerFunc(16, TimerFunction, 1);
		}
		break;
	case '2':
	case '3':
	case '4':
	case '5':
	{
		// 숫자만큼 스파이럴 생성
		int numSpirals = key - '0';
		spirals.clear();

		for (int i = 0; i < numSpirals; ++i) {
			spirals.push_back({
				dis_x(gen), dis_y(gen),  // centerX, centerY
				0.0f,                    // angle
				0.01f,                   // radius
				true,                    // active
				0,                       // pointCount
				64,                      // maxPoints
				true,                    // isExpanding
				0.3f,                    // maxRadius
				false,                   // hasPreviousPoint
				0.0f, 0.0f              // prevX, prevY
				});
		}

		bgColorR = dis_color(gen);
		bgColorG = dis_color(gen);
		bgColorB = dis_color(gen);

		if (!timerRunning) {
			timerRunning = true;
			glutTimerFunc(16, TimerFunction, 1);
		}
	}
	break;
	case 'c':
		allVertices.clear();
		allColors.clear();
		shapes.clear();
		spirals.clear();
		timerRunning = false;
		break;
	case 'q':
		exit(0);
		break;
	}
	glutPostRedisplay(); //--- 배경색이 바뀔 때마다 출력 콜백 함수를 호출하여 화면을 refresh 한다
}

void AddSpiralPoint(SpiralAnimation& spiral) {
	if (!spiral.active) return;

	// 스파이럴 위치 계산
	float x = spiral.centerX + spiral.radius * cos(spiral.angle);
	float y = spiral.centerY + spiral.radius * sin(spiral.angle);

	Shape newShape;
	// 점 추가
	if (shapeType == 0) {
		newShape.drawMode = GL_POINTS;
		newShape.startIndex = allVertices.size() / 3;
		newShape.vertexCount = 1;

		allVertices.push_back(x);
		allVertices.push_back(y);
		allVertices.push_back(0.0f);

		for (int i = 0; i < 3; ++i) {
			allColors.push_back(1.0f);
		}

		shapes.push_back(newShape);
	}
	else if (shapeType == 1) {
		if (spiral.hasPreviousPoint) {
			Shape newShape;
			newShape.drawMode = GL_LINES;
			newShape.startIndex = allVertices.size() / 3;
			newShape.vertexCount = 2;
			
			allVertices.push_back(spiral.prevX);
			allVertices.push_back(spiral.prevY);
			allVertices.push_back(0.0f);

			allVertices.push_back(x);
			allVertices.push_back(y);
			allVertices.push_back(0.0f);

			for (int i = 0; i < 6; ++i) {
				allColors.push_back(1.0f);
			}

			shapes.push_back(newShape);
		}

		spiral.prevX = x;
		spiral.prevY = y;
		spiral.hasPreviousPoint = true;
	}

		if (spiral.isExpanding) {
			spiral.angle += 0.3f;
			spiral.radius += 0.005f;

			if (spiral.pointCount >= spiral.maxPoints - 2) {
				spiral.isExpanding = false;

				float finalX = spiral.centerX + spiral.radius * cos(spiral.angle + 3.141592f);
				float finalY = spiral.centerY + spiral.radius * sin(spiral.angle + 3.141592f);

				//currentSpiral.centerX = finalX  * cos(currentSpiral.angle + 3.141592f) + 0.001f;
				//currentSpiral.centerY = finalY  * sin(currentSpiral.angle + 3.141592f) - 0.2f;

				spiral.centerX += 0.3145864306 * 2;
				spiral.centerY += 0.016387239 * 2;

				spiral.angle += 3.141592f;
			}
		}
		else {
			spiral.angle -= 0.3f;
			spiral.radius -= 0.005f;

			if (spiral.radius <= 0.01f) {
				spiral.active = false;
				spiral.hasPreviousPoint = false;
			}
		}

		spiral.pointCount++;

	UpdateBuffer();
}

GLvoid TimerFunction(int value) {
	bool hasActiveSpiral = false;

	for (auto& spiral : spirals) {
		if (spiral.active) {
			AddSpiralPoint(spiral);
			hasActiveSpiral = true;
		}
	}

	if (hasActiveSpiral) {
		UpdateBuffer();
		glutPostRedisplay();
		glutTimerFunc(16, TimerFunction, 1); // 다음 프레임을 위한 타이머 설정
	}
	else {
		timerRunning = false;
	}
}

GLvoid Mouse(int button, int state, int x, int y)
{
	float Mouse_x = mapToGLCoordX(x);
	float Mouse_y = mapToGLCoordY(y);
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		// 새로운 스파이럴 하나 추가
		spirals.push_back({
			Mouse_x, Mouse_y,        // centerX, centerY
			0.0f,                    // angle
			0.01f,                   // radius
			true,                    // active
			0,                       // pointCount
			64,                      // maxPoints
			true,                    // isExpanding
			0.3f,                    // maxRadius
			false,                   // hasPreviousPoint
			0.0f, 0.0f              // prevX, prevY
			});
		bgColorR = dis_color(gen);
		bgColorG = dis_color(gen);
		bgColorB = dis_color(gen);

		if (!timerRunning) {
			timerRunning = true;
			glutTimerFunc(16, TimerFunction, 1);
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
	glutCreateWindow("Practice 11");
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