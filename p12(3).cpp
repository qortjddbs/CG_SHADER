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
std::uniform_real_distribution<float> dis_x(0.1f, 0.9f); // -0.1f 부터 0.1f 까지 균등하게 나타나는 난수열을 생성하기 위해 균등 분포 정의.
std::uniform_real_distribution<float> dis_y(0.1f, 0.7f); // -0.1f 부터 0.1f 까지 균등하게 나타나는 난수열을 생성하기 위해 균등 분포 정의.
std::uniform_real_distribution<float> dis_size(0.05f, 0.15f); // 삼각형 크기용 랜덤 분포

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

int command = 0;

// 애니메이션 관련 변수
bool isAnimating = false;
// int animationStep = 0;

struct Shape {
	GLenum drawMode;
	int startIndex;
	int vertexCount;
	float centerX, centerY;
	int shapeType;	// 1 : 점, 2 : 선, 3 : 삼각형, 4 : 사각형, 5 : 오각형
	bool isTransforming;
	int quadrant;	// 도형이 어느 사분면에 있는지
	int targetShapeType; // 변환할 목표 도형 타입
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

void InitShapes() {	// x,y 축하고 각 사분면마다 삼각형 하나씩
	Shape LineX;
	LineX.startIndex = allVertices.size() / 3;
	allVertices.push_back(-1.0f);
	allVertices.push_back(0.0f);
	allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allVertices.push_back(1.0f);
	allVertices.push_back(0.0f);
	allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	LineX.drawMode = GL_LINES;
	LineX.vertexCount = 2;
	LineX.centerX = 0.0f;
	LineX.centerY = 0.0f;
	LineX.shapeType = 0; // 축
	LineX.quadrant = 0;
	LineX.targetShapeType = 0;

	shapes.push_back(LineX);
	UpdateBuffer();

	Shape LineY;
	LineY.startIndex = allVertices.size() / 3;
	allVertices.push_back(0.0f);
	allVertices.push_back(1.0f);
	allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allVertices.push_back(0.0f);
	allVertices.push_back(-1.0f);
	allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	LineY.drawMode = GL_LINES;
	LineY.vertexCount = 2;
	LineY.centerX = 0.0f;
	LineY.centerY = 0.0f;
	LineY.shapeType = 0; // 축
	LineY.quadrant = 0;
	LineY.targetShapeType = 0;

	shapes.push_back(LineY);
	
	// 1사분면 - 삼각형 (0.5, 0.5) - 5개 점으로
	Shape shape1;
	shape1.startIndex = allVertices.size() / 3;
	shape1.centerX = 0.5f; shape1.centerY = 0.5f;
	// 삼각형: 꼭짓점 3개 + 왼쪽 변 중점 + 오른쪽 변 중점
	// 상단 꼭짓점
	allVertices.push_back(0.5f); allVertices.push_back(0.8f); allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));
	// 좌하 꼭짓점
	allVertices.push_back(0.35f); allVertices.push_back(0.35f); allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));
	// 우하 꼭짓점
	allVertices.push_back(0.65f); allVertices.push_back(0.35f); allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));
	// 왼쪽 변 중점
	allVertices.push_back(0.425f); allVertices.push_back(0.575f); allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));
	// 오른쪽 변 중점
	allVertices.push_back(0.575f); allVertices.push_back(0.575f); allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));
	
	shape1.drawMode = GL_LINE_LOOP; shape1.vertexCount = 3;  // 삼각형은 첫 3개 점만 사용
	shape1.shapeType = 3; shape1.quadrant = 1;
	shape1.isTransforming = false;
	shape1.targetShapeType = 3;
	shapes.push_back(shape1);

	// 2사분면 - 선 (-0.5, 0.5) - 5개 점이 일직선으로
	Shape shape2;
	shape2.startIndex = allVertices.size() / 3;
	shape2.centerX = -0.5f; shape2.centerY = 0.5f;
	// 선: 5개 점이 일정한 간격으로 일직선으로
	float lineStartX = -0.75f, lineStartY = 0.25f;
	float lineEndX = -0.25f, lineEndY = 0.75f;
	for (int i = 0; i < 5; ++i) {
		float t = i / 4.0f; // 0, 0.25, 0.5, 0.75, 1.0
		float x = lineStartX + t * (lineEndX - lineStartX);
		float y = lineStartY + t * (lineEndY - lineStartY);
		allVertices.push_back(x); allVertices.push_back(y); allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));
	}
	
	shape2.drawMode = GL_LINE_STRIP; shape2.vertexCount = 5;  // 5개 점을 선으로 연결
	shape2.shapeType = 2; shape2.quadrant = 2;
	shape2.isTransforming = false;
	shape2.targetShapeType = 2;
	shapes.push_back(shape2);

	// 3사분면 - 사각형 (-0.5, -0.5) - 4개 꼭짓점 + 중심
	Shape shape3;
	shape3.startIndex = allVertices.size() / 3;
	shape3.centerX = -0.5f; shape3.centerY = -0.5f;
	// 사각형: 4개 꼭짓점 (시계방향으로 배치)
	// 좌하 꼭짓점
	allVertices.push_back(-0.75f); allVertices.push_back(-0.75f); allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));
	// 우하 꼭짓점
	allVertices.push_back(-0.25f); allVertices.push_back(-0.75f); allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));
	// 우상 꼭짓점
	allVertices.push_back(-0.25f); allVertices.push_back(-0.25f); allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));
	// 좌상 꼭짓점
	allVertices.push_back(-0.75f); allVertices.push_back(-0.25f); allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));
	// 중심점 (더미, 사용하지 않음)
	allVertices.push_back(-0.5f); allVertices.push_back(-0.5f); allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));
	
	shape3.drawMode = GL_LINE_LOOP; shape3.vertexCount = 4;  // 사각형은 첫 4개 점만 사용
	shape3.shapeType = 4; shape3.quadrant = 3;
	shape3.isTransforming = false;
	shape3.targetShapeType = 4;
	shapes.push_back(shape3);

	// 4사분면 - 오각형 (0.5, -0.5) - 5개 꼭짓점
	Shape shape4;
	shape4.startIndex = allVertices.size() / 3;
	shape4.centerX = 0.5f; shape4.centerY = -0.5f;
	// 오각형: 5개 꼭짓점 (정오각형)
	float radius = 0.3f;
	for (int i = 0; i < 5; ++i) {
		float angle = (i * 2.0f * 3.14159f / 5.0f) - (3.14159f / 2.0f); // -90도부터 시작
		float x = shape4.centerX + radius * cos(angle);
		float y = shape4.centerY + radius * sin(angle);
		allVertices.push_back(x); allVertices.push_back(y); allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));
	}
	
	shape4.drawMode = GL_LINE_LOOP; shape4.vertexCount = 5;  // 5개 점을 연결해서 오각형
	shape4.shapeType = 5; shape4.quadrant = 4;
	shape4.isTransforming = false;
	shape4.targetShapeType = 5;
	shapes.push_back(shape4);

	UpdateBuffer();
}

GLvoid Timer(int value) {
	bool anyAnimating = false;

	for (auto& shape : shapes) {
		if (shape.isTransforming) {
			anyAnimating = true;
			int baseIndex = shape.startIndex * 3;
			float centerX = shape.centerX;
			float centerY = shape.centerY;

			// 현재 도형 타입과 목표 도형 타입에 따라 변환 처리
			if (shape.shapeType == 2 && shape.targetShapeType == 3) { // 선 -> 삼각형
				shape.drawMode = GL_LINE_LOOP;
				shape.vertexCount = 3;

				// 삼각형 목표 좌표: 꼭짓점 3개
				float targets[5][2] = {
					{centerX, centerY + 0.3f},         // 상단 꼭짓점
					{centerX - 0.15f, centerY - 0.15f}, // 좌하 꼭짓점
					{centerX + 0.15f, centerY - 0.15f}, // 우하 꼭짓점
					{centerX - 0.075f, centerY + 0.075f}, // 왼쪽 변 중점 (더미)
					{centerX + 0.075f, centerY + 0.075f}  // 오른쪽 변 중점 (더미)
				};

				bool allReached = true;
				for (int i = 0; i < 5; ++i) {
					float targetX = targets[i][0];
					float targetY = targets[i][1];
					int vertexIndex = baseIndex + i * 3;

					if (fabs(allVertices[vertexIndex] - targetX) > 0.01f) {
						allVertices[vertexIndex] += (targetX - allVertices[vertexIndex]) * 0.05f;
						allReached = false;
					}
					if (fabs(allVertices[vertexIndex + 1] - targetY) > 0.01f) {
						allVertices[vertexIndex + 1] += (targetY - allVertices[vertexIndex + 1]) * 0.05f;
						allReached = false;
					}
				}

				if (allReached) {
					for (int i = 0; i < 5; ++i) {
						int vertexIndex = baseIndex + i * 3;
						allVertices[vertexIndex] = targets[i][0];
						allVertices[vertexIndex + 1] = targets[i][1];
						allVertices[vertexIndex + 2] = 0.0f;
					}
					shape.shapeType = 3;
					shape.targetShapeType = 3;
					shape.isTransforming = false;
				}
			}
			else if (shape.shapeType == 3 && shape.targetShapeType == 4) { // 삼각형 -> 사각형
				shape.drawMode = GL_LINE_LOOP;
				shape.vertexCount = 4;

				// 사각형 목표 좌표: 4개 꼭짓점 (시계방향)
				float targets[5][2] = {
					{centerX - 0.25f, centerY - 0.25f}, // 좌하
					{centerX + 0.25f, centerY - 0.25f}, // 우하
					{centerX + 0.25f, centerY + 0.25f}, // 우상
					{centerX - 0.25f, centerY + 0.25f}, // 좌상
					{centerX, centerY}                   // 중심 (더미)
				};

				bool allReached = true;
				for (int i = 0; i < 5; ++i) {
					float targetX = targets[i][0];
					float targetY = targets[i][1];
					int vertexIndex = baseIndex + i * 3;

					if (fabs(allVertices[vertexIndex] - targetX) > 0.01f) {
						allVertices[vertexIndex] += (targetX - allVertices[vertexIndex]) * 0.05f;
						allReached = false;
					}
					if (fabs(allVertices[vertexIndex + 1] - targetY) > 0.01f) {
						allVertices[vertexIndex + 1] += (targetY - allVertices[vertexIndex + 1]) * 0.05f;
						allReached = false;
					}
				}

				if (allReached) {
					for (int i = 0; i < 5; ++i) {
						int vertexIndex = baseIndex + i * 3;
						allVertices[vertexIndex] = targets[i][0];
						allVertices[vertexIndex + 1] = targets[i][1];
						allVertices[vertexIndex + 2] = 0.0f;
					}
					shape.shapeType = 4;
					shape.targetShapeType = 4;
					shape.isTransforming = false;
				}
			}
			else if (shape.shapeType == 4 && shape.targetShapeType == 5) { // 사각형 -> 오각형
				shape.drawMode = GL_LINE_LOOP;
				shape.vertexCount = 5;

				// 오각형 목표 좌표: 5개 꼭짓점
				float radius = 0.3f;
				float targets[5][2];
				for (int i = 0; i < 5; ++i) {
					float angle = (i * 2.0f * 3.14159f / 5.0f) - (3.14159f / 2.0f);
					targets[i][0] = centerX + radius * cos(angle);
					targets[i][1] = centerY + radius * sin(angle);
				}

				bool allReached = true;
				for (int i = 0; i < 5; ++i) {
					float targetX = targets[i][0];
					float targetY = targets[i][1];
					int vertexIndex = baseIndex + i * 3;

					if (fabs(allVertices[vertexIndex] - targetX) > 0.01f) {
						allVertices[vertexIndex] += (targetX - allVertices[vertexIndex]) * 0.05f;
						allReached = false;
					}
					if (fabs(allVertices[vertexIndex + 1] - targetY) > 0.01f) {
						allVertices[vertexIndex + 1] += (targetY - allVertices[vertexIndex + 1]) * 0.05f;
						allReached = false;
					}
				}

				if (allReached) {
					for (int i = 0; i < 5; ++i) {
						int vertexIndex = baseIndex + i * 3;
						allVertices[vertexIndex] = targets[i][0];
						allVertices[vertexIndex + 1] = targets[i][1];
						allVertices[vertexIndex + 2] = 0.0f;
					}
					shape.shapeType = 5;
					shape.targetShapeType = 5;
					shape.isTransforming = false;
				}
			}
			else if (shape.shapeType == 5 && shape.targetShapeType == 2) { // 오각형 -> 선
				shape.drawMode = GL_LINE_STRIP;
				shape.vertexCount = 5;

				// 선 목표 좌표: 5개 점이 일직선으로
				float lineStartX = centerX - 0.25f;
				float lineStartY = centerY - 0.25f;
				float lineEndX = centerX + 0.25f;
				float lineEndY = centerY + 0.25f;

				float targets[5][2];
				for (int i = 0; i < 5; ++i) {
					float t = i / 4.0f;
					targets[i][0] = lineStartX + t * (lineEndX - lineStartX);
					targets[i][1] = lineStartY + t * (lineEndY - lineStartY);
				}

				bool allReached = true;
				for (int i = 0; i < 5; ++i) {
					float targetX = targets[i][0];
					float targetY = targets[i][1];
					int vertexIndex = baseIndex + i * 3;

					if (fabs(allVertices[vertexIndex] - targetX) > 0.01f) {
						allVertices[vertexIndex] += (targetX - allVertices[vertexIndex]) * 0.05f;
						allReached = false;
					}
					if (fabs(allVertices[vertexIndex + 1] - targetY) > 0.01f) {
						allVertices[vertexIndex + 1] += (targetY - allVertices[vertexIndex + 1]) * 0.05f;
						allReached = false;
					}
				}

				if (allReached) {
					for (int i = 0; i < 5; ++i) {
						int vertexIndex = baseIndex + i * 3;
						allVertices[vertexIndex] = targets[i][0];
						allVertices[vertexIndex + 1] = targets[i][1];
						allVertices[vertexIndex + 2] = 0.0f;
					}
					shape.shapeType = 2;
					shape.targetShapeType = 2;
					shape.isTransforming = false;
				}
			}
		}
	}

	// isAnimating 상태 업데이트
	isAnimating = anyAnimating;

	if (anyAnimating) {
		UpdateBuffer();
		glutPostRedisplay();
		glutTimerFunc(16, Timer, 1);
	}
	else {
		// 애니메이션이 완료되면 타이머를 멈추고, 필요할 때 다시 시작
		glutPostRedisplay();
	}
}

void changeShapes() {
	bool startTimer = false;

	for (auto& shape : shapes) {
		if (shape.quadrant == 0) continue; // 축은 제외

		if (command == 2 && shape.shapeType == 2) { // l: 선 -> 삼각형
			shape.isTransforming = true;
			shape.targetShapeType = 3;
			startTimer = true;
		}
		else if (command == 3 && shape.shapeType == 3) { // t: 삼각형 -> 사각형
			shape.isTransforming = true;
			shape.targetShapeType = 4;
			startTimer = true;
		}
		else if (command == 4 && shape.shapeType == 4) { // r: 사각형 -> 오각형
			shape.isTransforming = true;
			shape.targetShapeType = 5;
			startTimer = true;
		}
		else if (command == 5 && shape.shapeType == 5) { // p: 오각형 -> 선
			shape.isTransforming = true;
			shape.targetShapeType = 2;
			startTimer = true;
		}
	}

	if (startTimer && !isAnimating) {
		isAnimating = true;
		glutTimerFunc(16, Timer, 1);
	}
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
	case 'l':
		command = 2;
		changeShapes();
		break;
	case 't':
		command = 3;
		changeShapes();
		break;
	case 'r':
		command = 4;
		changeShapes();
		break;
	case 'p':
		command = 5;
		changeShapes();
		break;
	case 'a':
		// 모든 애니메이션 중지
		isAnimating = false;
		for (auto& shape : shapes) {
			shape.isTransforming = false;
		}

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
	glutCreateWindow("Practice 12");
	//--- GLEW 초기화하기
	glewExperimental = GL_TRUE;
	glewInit();
	shaderProgramID = make_shaderProgram();
	InitBuffer();
	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
	InitShapes();
	glutTimerFunc(16, Timer, 1); // 타이머 시작
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