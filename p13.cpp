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
GLvoid Timer(int value);
//--- 필요한 변수 선언
GLint width, height;
GLuint shaderProgramID; //--- 세이더 프로그램 이름
GLuint vertexShader; //--- 버텍스 세이더 객체
GLuint fragmentShader; //--- 프래그먼트 세이더 객체

std::random_device rd;  // 시드값을 얻기 위한 random_device 생성.
std::mt19937 gen(rd());	// random_device 를 통해 난수 생성 엔진을 초기화 한다.
std::uniform_real_distribution<float> dis_color(0.0f, 1.0f); // 0.0f 부터 1.0f 까지 균등하게 나타나는 난수열을 생성하기 위해 균등 분포 정의.
std::uniform_real_distribution<float> dis_x(-0.7f, 0.7f); // -0.1f 부터 0.1f 까지 균등하게 나타나는 난수열을 생성하기 위해 균등 분포 정의.
std::uniform_real_distribution<float> dis_y(-0.7f, 0.7f); // -0.1f 부터 0.1f 까지 균등하게 나타나는 난수열을 생성하기 위해 균등 분포 정의.
std::uniform_real_distribution<float> dis_size(0.05f, 0.15f); // 삼각형 크기용 랜덤 분포

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

int command = 0;

// 애니메이션 관련 변수
bool isAnimating = false;

struct Shape {
	GLenum drawMode;
	int startIndex;
	int vertexCount;
	float centerX, centerY, size;
	int shapeType;	// 1 : 점, 2 : 선, 3 : 삼각형, 4 : 사각형, 5 : 오각형
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

void InitShapes() {

	// 오각형
	for (int i = 0; i < 3; ++i) {
		Shape shape5;
		shape5.startIndex = allVertices.size() / 3;
		shape5.centerX = dis_x(gen); shape5.centerY = dis_y(gen); shape5.size = dis_size(gen);
		float x = shape5.centerX; float y = shape5.centerY;	float size = shape5.size;

		// 중앙 삼각형: (0, +0.3), (-0.15, -0.3), (+0.15, -0.3)
		allVertices.push_back(x); allVertices.push_back(y + size * 2); allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));

		allVertices.push_back(x - size); allVertices.push_back(y - size * 2); allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));

		allVertices.push_back(x + size); allVertices.push_back(y - size * 2); allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));

		// 왼쪽 삼각형: (-0.3, +0.05), (0, +0.3), (-0.15, -0.3)
		allVertices.push_back(x - size * 2); allVertices.push_back(y); allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));

		allVertices.push_back(x); allVertices.push_back(y + size * 2); allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));

		allVertices.push_back(x - size); allVertices.push_back(y - size * 2); allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));

		// 오른쪽 삼각형: (+0.3, +0.05), (0, +0.3), (+0.15, -0.3)
		allVertices.push_back(x + size * 2); allVertices.push_back(y); allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));

		allVertices.push_back(x); allVertices.push_back(y + size * 2); allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));

		allVertices.push_back(x + size); allVertices.push_back(y - size * 2); allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));

		shape5.drawMode = GL_TRIANGLES;
		shape5.vertexCount = 9;
		shape5.shapeType = 5;
		shapes.push_back(shape5);
	}
	
	// 사각형
	for (int i = 0; i < 3; ++i) {
		Shape shape4;
		shape4.startIndex = allVertices.size() / 3;
		shape4.centerX = dis_x(gen); shape4.centerY = dis_y(gen); shape4.size = dis_size(gen);
		float x = shape4.centerX; float y = shape4.centerY;	float size = shape4.size;

		// 좌상단
		allVertices.push_back(x - size); allVertices.push_back(y + size); allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));

		// 좌하단
		allVertices.push_back(x - size); allVertices.push_back(y - size); allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));

		// 우하단
		allVertices.push_back(x + size); allVertices.push_back(y - size); allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));

		// 좌상단
		allVertices.push_back(x - size); allVertices.push_back(y + size); allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));

		// 우하단
		allVertices.push_back(x + size); allVertices.push_back(y - size); allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));

		// 우상단
		allVertices.push_back(x + size); allVertices.push_back(y + size); allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));

		shape4.drawMode = GL_TRIANGLES;
		shape4.vertexCount = 6;
		shape4.shapeType = 4;
		shapes.push_back(shape4);
	}
	
	// 삼각형
	for (int i = 0; i < 3; ++i) {
		Shape shape3;
		shape3.startIndex = allVertices.size() / 3;
		shape3.centerX = dis_x(gen); shape3.centerY = dis_y(gen); shape3.size = dis_size(gen);
		float x = shape3.centerX; float y = shape3.centerY;	float size = shape3.size;

		allVertices.push_back(x); allVertices.push_back(y + size * 2); allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));

		allVertices.push_back(x - size); allVertices.push_back(y - size); allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));

		allVertices.push_back(x + size); allVertices.push_back(y - size); allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));

		shape3.drawMode = GL_TRIANGLES;
		shape3.vertexCount = 3;
		shape3.shapeType = 3;
		shapes.push_back(shape3);
	}
	

	// 선
	for (int i = 0; i < 3; ++i) {
		Shape shape2;
		shape2.startIndex = allVertices.size() / 3;
		shape2.centerX = dis_x(gen); shape2.centerY = dis_y(gen);  shape2.size = dis_size(gen);
		float x = shape2.centerX;
		float y = shape2.centerY;
		float size = shape2.size;
		allVertices.push_back(x - size); allVertices.push_back(y - size); allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));
		allVertices.push_back(x + size); allVertices.push_back(y + size); allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));

		shape2.drawMode = GL_LINES;
		shape2.vertexCount = 2;
		shape2.shapeType = 2;
		shapes.push_back(shape2);
	}

	// 점 (개작은 삼각형)
	for (int i = 0; i < 3; ++i) {
		Shape shape1;
		shape1.startIndex = allVertices.size() / 3;
		shape1.centerX = dis_x(gen); shape1.centerY = dis_y(gen);  shape1.size = dis_size(gen);
		float x = shape1.centerX;
		float y = shape1.centerY;
		float size = shape1.size;

		allVertices.push_back(x); allVertices.push_back(y + 0.02f); allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));

		allVertices.push_back(x - 0.01f); allVertices.push_back(y - 0.01f); allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));

		allVertices.push_back(x + 0.01f); allVertices.push_back(y - 0.01f); allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));

		shape1.drawMode = GL_TRIANGLES;
		shape1.vertexCount = 3;
		shape1.shapeType = 1;
		shapes.push_back(shape1);
	}
	


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
	case 'a':
		allVertices.clear();
		allColors.clear();
		shapes.clear();
		InitShapes();
		break;
	case 'q':
		exit(0);
		break;
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
	glutCreateWindow("Practice 13");
	//--- GLEW 초기화하기
	glewExperimental = GL_TRUE;
	glewInit();
	shaderProgramID = make_shaderProgram();
	InitBuffer();
	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
	InitShapes();
	// glutTimerFunc(16, Timer, 1); // 타이머 시작
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