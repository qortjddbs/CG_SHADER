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
void UpdateBuffer();
GLuint make_shaderProgram();
GLvoid drawScene();
GLvoid Reshape(int w, int h);
GLvoid Mouse(int button, int state, int x, int y);
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
std::uniform_int_distribution<int> dis_motion(1, 2); // 1: 대각선, 2: 지그재그

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

int command = 0;
int selected = -1;
bool isDragging = false;
float lastMouseX = 0.0f;
float lastMouseY = 0.0f;
bool animationEnabled = true;
bool timerRunning = false;

struct Shape {
	GLenum drawMode;
	int startIndex;
	int vertexCount;
	float centerX, centerY, size;
	int shapeType;	// 1 : 점, 2 : 선, 3 : 삼각형, 4 : 사각형, 5 : 오각형
	GLfloat r, g, b;	// 고정 색상 추가

	// 애니메이션 관련 변수들
	bool isAnimated;
	int motionType;  // 1: 대각선, 2: 지그재그
	float dirX, dirY;
	float zigzagStepY; // 지그재그용 Y축 이동량
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

// 두 도형이 겹치는지 확인하는 함수
bool AreShapesOverlapping(const Shape& shape1, const Shape& shape2) {
	float dx = shape1.centerX - shape2.centerX;
	float dy = shape1.centerY - shape2.centerY;
	float distance = sqrt(dx * dx + dy * dy);

	// 두 도형의 크기를 고려한 충돌 거리 계산
	float collisionDistance = (shape1.size + shape2.size) * 1.5f;

	return distance < collisionDistance;
}

// 새로운 도형을 생성하는 함수
// 새로운 도형을 생성하는 함수 (고정 색상 사용)
void CreateShape(int shapeType, float centerX, float centerY, float size, bool animated = false) {
	Shape newShape;
	newShape.startIndex = allVertices.size() / 3;
	newShape.centerX = centerX;
	newShape.centerY = centerY;
	newShape.size = size;
	newShape.shapeType = shapeType;
	newShape.isAnimated = animated;

	// 애니메이션 설정
	if (animated) {
		newShape.motionType = dis_motion(gen);
		if (newShape.motionType == 1) { // 대각선
			newShape.dirX = 0.02f;
			newShape.dirY = 0.02f;
		}
		else { // 지그재그
			newShape.dirX = 0.02f;
			newShape.dirY = 0.0f;
			newShape.zigzagStepY = 0.2f;
		}
	}
	else {
		newShape.motionType = 0;
		newShape.dirX = 0.0f;
		newShape.dirY = 0.0f;
		newShape.zigzagStepY = 0.0f;
	}

	// 도형 타입별 고정 색상 설정
	switch (shapeType) {
	case 1: newShape.r = 1.0f; newShape.g = 0.0f; newShape.b = 0.0f; break; // 빨강 (점)
	case 2: newShape.r = 0.0f; newShape.g = 1.0f; newShape.b = 0.0f; break; // 초록 (선)
	case 3: newShape.r = 0.0f; newShape.g = 0.0f; newShape.b = 1.0f; break; // 파랑 (삼각형)
	case 4: newShape.r = 1.0f; newShape.g = 1.0f; newShape.b = 0.0f; break; // 노랑 (사각형)
	case 5: newShape.r = 1.0f; newShape.g = 0.0f; newShape.b = 1.0f; break; // 보라 (오각형)
	}

	float x = centerX;
	float y = centerY;

	switch (shapeType) {
	case 1: // 점 (작은 삼각형)
		allVertices.push_back(x); allVertices.push_back(y + 0.02f); allVertices.push_back(0.0f);
		allColors.push_back(newShape.r); allColors.push_back(newShape.g); allColors.push_back(newShape.b);

		allVertices.push_back(x - 0.01f); allVertices.push_back(y - 0.01f); allVertices.push_back(0.0f);
		allColors.push_back(newShape.r); allColors.push_back(newShape.g); allColors.push_back(newShape.b);

		allVertices.push_back(x + 0.01f); allVertices.push_back(y - 0.01f); allVertices.push_back(0.0f);
		allColors.push_back(newShape.r); allColors.push_back(newShape.g); allColors.push_back(newShape.b);

		newShape.drawMode = GL_TRIANGLES;
		newShape.vertexCount = 3;
		break;

	case 2: // 선
		allVertices.push_back(x - size); allVertices.push_back(y); allVertices.push_back(0.0f);
		allColors.push_back(newShape.r); allColors.push_back(newShape.g); allColors.push_back(newShape.b);
		allVertices.push_back(x + size); allVertices.push_back(y); allVertices.push_back(0.0f);
		allColors.push_back(newShape.r); allColors.push_back(newShape.g); allColors.push_back(newShape.b);

		newShape.drawMode = GL_LINES;
		newShape.vertexCount = 2;
		break;

	case 3: // 삼각형
		allVertices.push_back(x); allVertices.push_back(y + size * 2); allVertices.push_back(0.0f);
		allColors.push_back(newShape.r); allColors.push_back(newShape.g); allColors.push_back(newShape.b);

		allVertices.push_back(x - size); allVertices.push_back(y - size); allVertices.push_back(0.0f);
		allColors.push_back(newShape.r); allColors.push_back(newShape.g); allColors.push_back(newShape.b);

		allVertices.push_back(x + size); allVertices.push_back(y - size); allVertices.push_back(0.0f);
		allColors.push_back(newShape.r); allColors.push_back(newShape.g); allColors.push_back(newShape.b);

		newShape.drawMode = GL_TRIANGLES;
		newShape.vertexCount = 3;
		break;

	case 4: // 사각형
		// 좌상단
		allVertices.push_back(x - size); allVertices.push_back(y + size); allVertices.push_back(0.0f);
		allColors.push_back(newShape.r); allColors.push_back(newShape.g); allColors.push_back(newShape.b);

		// 좌하단
		allVertices.push_back(x - size); allVertices.push_back(y - size); allVertices.push_back(0.0f);
		allColors.push_back(newShape.r); allColors.push_back(newShape.g); allColors.push_back(newShape.b);

		// 우하단
		allVertices.push_back(x + size); allVertices.push_back(y - size); allVertices.push_back(0.0f);
		allColors.push_back(newShape.r); allColors.push_back(newShape.g); allColors.push_back(newShape.b);

		// 좌상단
		allVertices.push_back(x - size); allVertices.push_back(y + size); allVertices.push_back(0.0f);
		allColors.push_back(newShape.r); allColors.push_back(newShape.g); allColors.push_back(newShape.b);

		// 우하단
		allVertices.push_back(x + size); allVertices.push_back(y - size); allVertices.push_back(0.0f);
		allColors.push_back(newShape.r); allColors.push_back(newShape.g); allColors.push_back(newShape.b);

		// 우상단
		allVertices.push_back(x + size); allVertices.push_back(y + size); allVertices.push_back(0.0f);
		allColors.push_back(newShape.r); allColors.push_back(newShape.g); allColors.push_back(newShape.b);

		newShape.drawMode = GL_TRIANGLES;
		newShape.vertexCount = 6;
		break;

	case 5: // 오각형
		// 중앙 삼각형
		allVertices.push_back(x); allVertices.push_back(y + size * 2); allVertices.push_back(0.0f);
		allColors.push_back(newShape.r); allColors.push_back(newShape.g); allColors.push_back(newShape.b);

		allVertices.push_back(x - size); allVertices.push_back(y - size * 2); allVertices.push_back(0.0f);
		allColors.push_back(newShape.r); allColors.push_back(newShape.g); allColors.push_back(newShape.b);

		allVertices.push_back(x + size); allVertices.push_back(y - size * 2); allVertices.push_back(0.0f);
		allColors.push_back(newShape.r); allColors.push_back(newShape.g); allColors.push_back(newShape.b);

		// 왼쪽 삼각형
		allVertices.push_back(x - size * 2); allVertices.push_back(y); allVertices.push_back(0.0f);
		allColors.push_back(newShape.r); allColors.push_back(newShape.g); allColors.push_back(newShape.b);

		allVertices.push_back(x); allVertices.push_back(y + size * 2); allVertices.push_back(0.0f);
		allColors.push_back(newShape.r); allColors.push_back(newShape.g); allColors.push_back(newShape.b);

		allVertices.push_back(x - size); allVertices.push_back(y - size * 2); allVertices.push_back(0.0f);
		allColors.push_back(newShape.r); allColors.push_back(newShape.g); allColors.push_back(newShape.b);

		// 오른쪽 삼각형
		allVertices.push_back(x + size * 2); allVertices.push_back(y); allVertices.push_back(0.0f);
		allColors.push_back(newShape.r); allColors.push_back(newShape.g); allColors.push_back(newShape.b);

		allVertices.push_back(x); allVertices.push_back(y + size * 2); allVertices.push_back(0.0f);
		allColors.push_back(newShape.r); allColors.push_back(newShape.g); allColors.push_back(newShape.b);

		allVertices.push_back(x + size); allVertices.push_back(y - size * 2); allVertices.push_back(0.0f);
		allColors.push_back(newShape.r); allColors.push_back(newShape.g); allColors.push_back(newShape.b);

		newShape.drawMode = GL_TRIANGLES;
		newShape.vertexCount = 9;
		break;
	}

	shapes.push_back(newShape);
}

// 두 도형을 합치는 함수
void MergeShapes(int shapeIndex1, int shapeIndex2) {
	const Shape& shape1 = shapes[shapeIndex1];
	const Shape& shape2 = shapes[shapeIndex2];

	// 꼭짓점 개수 합계 계산
	int totalVertices = shape1.shapeType + shape2.shapeType;
	int newShapeType;

	// 합쳐진 꼭짓점 수에 따른 새로운 도형 타입 결정
	if (totalVertices >= 6) {
		newShapeType = 1; // 점
	}
	else {
		newShapeType = totalVertices;
	}

	// 새 도형의 위치는 두 도형의 중점
	float newCenterX = (shape1.centerX + shape2.centerX) / 2.0f;
	float newCenterY = (shape1.centerY + shape2.centerY) / 2.0f;
	float newSize = (shape1.size + shape2.size) / 2.0f;

	// 기존 도형들을 shapes 벡터에서 제거 (인덱스가 큰 것부터 제거)
	int firstToRemove = std::max(shapeIndex1, shapeIndex2);
	int secondToRemove = std::min(shapeIndex1, shapeIndex2);

	shapes.erase(shapes.begin() + firstToRemove);
	shapes.erase(shapes.begin() + secondToRemove);

	// 전체 버텍스와 컬러 배열을 다시 구성
	allVertices.clear();
	allColors.clear();

	// 남은 도형들을 다시 추가
	std::vector<Shape> tempShapes = shapes;
	shapes.clear();

	for (const auto& shape : tempShapes) {
		CreateShape(shape.shapeType, shape.centerX, shape.centerY, shape.size, shape.isAnimated);
		if (shape.isAnimated) {
			shapes.back().motionType = shape.motionType;
			shapes.back().dirX = shape.dirX;
			shapes.back().dirY = shape.dirY;
			shapes.back().zigzagStepY = shape.zigzagStepY;
		}
	}

	// 새로운 합쳐진 도형 추가 (애니메이션 활성화)
	CreateShape(newShapeType, newCenterX, newCenterY, newSize, true);

	// 애니메이션이 활성화되어 있다면 타이머 시작
	if (animationEnabled && !timerRunning) {
		timerRunning = true;
		glutTimerFunc(16, Timer, 1);
	}

	UpdateBuffer();
}

bool IsMouseInShape(float x, float y, const Shape& shape) {
	float dx = x - shape.centerX;
	float dy = y - shape.centerY;

	// 절댓값으로 변경
	if (dx < 0) dx = -dx;
	if (dy < 0) dy = -dy;

	switch (shape.shapeType) {
	case 1: // 점
		return (dx <= 0.02f && dy <= 0.02f);	// 사각형 영역
	case 2: // 선
		return (dy <= 0.01f && dx <= shape.size * 2); // 선 중심 주변 영역
	case 3: { // 삼각형
		// 삼각형의 세 꼭짓점 (AddShape에서 정의된 좌표 기준)
		float x1 = shape.centerX;           // 위쪽 꼭짓점
		float y1 = shape.centerY + shape.size * 2;
		float x2 = shape.centerX - shape.size;   // 왼쪽 아래 꼭짓점
		float y2 = shape.centerY - shape.size;
		float x3 = shape.centerX + shape.size;   // 오른쪽 아래 꼭짓점
		float y3 = shape.centerY - shape.size;

		// 무게중심 좌표를 이용한 삼각형 내부 판별
		float denominator = (y2 - y3) * (x1 - x3) + (x3 - x2) * (y1 - y3);
		if (abs(denominator) < 0.0001f) return false; // 삼각형이 아닌 경우

		float a = ((y2 - y3) * (x - x3) + (x3 - x2) * (y - y3)) / denominator;
		float b = ((y3 - y1) * (x - x3) + (x1 - x3) * (y - y3)) / denominator;
		float c = 1 - a - b;

		return (a >= 0 && b >= 0 && c >= 0);
	}
	case 4: // 사각형
		return abs(dx) <= shape.size && abs(dy) <= shape.size; // 사각형 영역
	case 5: {	// 오각형
		// 오각형을 구성하는 3개 삼각형에 대해 각각 내부 판별

		// 중앙 삼각형 판별
		float x1 = shape.centerX;
		float y1 = shape.centerY + shape.size * 2;
		float x2 = shape.centerX - shape.size;
		float y2 = shape.centerY - shape.size * 2;
		float x3 = shape.centerX + shape.size;
		float y3 = shape.centerY - shape.size * 2;

		// 무게중심 좌표로 중앙 삼각형 내부 판별
		float denom1 = (y2 - y3) * (x1 - x3) + (x3 - x2) * (y1 - y3);
		if (abs(denom1) > 0.0001f) {
			float a1 = ((y2 - y3) * (x - x3) + (x3 - x2) * (y - y3)) / denom1;
			float b1 = ((y3 - y1) * (x - x3) + (x1 - x3) * (y - y3)) / denom1;
			float c1 = 1 - a1 - b1;
			if (a1 >= 0 && b1 >= 0 && c1 >= 0) return true;
		}

		// 왼쪽 삼각형 판별
		x1 = shape.centerX - shape.size * 2;
		y1 = shape.centerY;
		x2 = shape.centerX;
		y2 = shape.centerY + shape.size * 2;
		x3 = shape.centerX - shape.size;
		y3 = shape.centerY - shape.size * 2;

		float denom2 = (y2 - y3) * (x1 - x3) + (x3 - x2) * (y1 - y3);
		if (abs(denom2) > 0.0001f) {
			float a2 = ((y2 - y3) * (x - x3) + (x3 - x2) * (y - y3)) / denom2;
			float b2 = ((y3 - y1) * (x - x3) + (x1 - x3) * (y - y3)) / denom2;
			float c2 = 1 - a2 - b2;
			if (a2 >= 0 && b2 >= 0 && c2 >= 0) return true;
		}

		// 오른쪽 삼각형 판별
		x1 = shape.centerX + shape.size * 2;
		y1 = shape.centerY;
		x2 = shape.centerX;
		y2 = shape.centerY + shape.size * 2;
		x3 = shape.centerX + shape.size;
		y3 = shape.centerY - shape.size * 2;

		float denom3 = (y2 - y3) * (x1 - x3) + (x3 - x2) * (y1 - y3);
		if (abs(denom3) > 0.0001f) {
			float a3 = ((y2 - y3) * (x - x3) + (x3 - x2) * (y - y3)) / denom3;
			float b3 = ((y3 - y1) * (x - x3) + (x1 - x3) * (y - y3)) / denom3;
			float c3 = 1 - a3 - b3;
			if (a3 >= 0 && b3 >= 0 && c3 >= 0) return true;
		}

		return false;
	}
	}  // switch문 닫는 중괄호 추가

	return false;  // 기본 반환값 추가
}  // 함수 닫는 중괄호 추가

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
		float x = dis_x(gen);
		float y = dis_y(gen);
		float size = dis_size(gen);
		CreateShape(5, x, y, size);
	}

	// 사각형
	for (int i = 0; i < 3; ++i) {
		float x = dis_x(gen);
		float y = dis_y(gen);
		float size = dis_size(gen);
		CreateShape(4, x, y, size);
	}

	// 삼각형
	for (int i = 0; i < 3; ++i) {
		float x = dis_x(gen);
		float y = dis_y(gen);
		float size = dis_size(gen);
		CreateShape(3, x, y, size);
	}

	// 선
	for (int i = 0; i < 3; ++i) {
		float x = dis_x(gen);
		float y = dis_y(gen);
		float size = dis_size(gen);
		CreateShape(2, x, y, size);
	}

	// 점
	for (int i = 0; i < 3; ++i) {
		float x = dis_x(gen);
		float y = dis_y(gen);
		float size = dis_size(gen);
		CreateShape(1, x, y, size);
	}

	UpdateBuffer();
}

void UpdateShapeAnimation(Shape& shape) {
	if (!shape.isAnimated) return;

	float deltaX = 0.0f, deltaY = 0.0f;

	if (shape.motionType == 1) { // 대각선 이동
		// 경계 체크 및 방향 반전
		if (shape.centerX + shape.size >= 1.0f || shape.centerX - shape.size <= -1.0f) {
			shape.dirX *= -1;
		}
		if (shape.centerY + shape.size * 2 >= 1.0f || shape.centerY - shape.size * 2 <= -1.0f) {
			shape.dirY *= -1;
		}

		deltaX = shape.dirX;
		deltaY = shape.dirY;
	}
	else if (shape.motionType == 2) { // 지그재그 이동
		// 좌우 경계 체크 및 방향 반전 + Y축 이동
		if (shape.centerX + shape.size >= 1.0f || shape.centerX - shape.size <= -1.0f) {
			shape.dirX *= -1;
			deltaY = shape.zigzagStepY;
		}

		// Y축 경계 체크
		if (shape.centerY + shape.size * 2 >= 1.0f || shape.centerY - shape.size * 2 <= -1.0f) {
			shape.zigzagStepY *= -1;
		}

		deltaX = shape.dirX;
	}

	// 정점들 이동
	int startIdx = shape.startIndex * 3;
	int endIdx = startIdx + shape.vertexCount * 3;

	for (int i = startIdx; i < endIdx; i += 3) {
		allVertices[i] += deltaX;     // x 좌표
		allVertices[i + 1] += deltaY; // y 좌표
	}

	// 중심점 업데이트
	shape.centerX += deltaX;
	shape.centerY += deltaY;
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
	case 'c':
		allVertices.clear();
		allColors.clear();
		shapes.clear();
		InitShapes();
		animationEnabled = true;
		break;
	case 's':
		animationEnabled = !animationEnabled;
		if (animationEnabled && !timerRunning) {
			// 애니메이션 활성화된 도형이 있는지 확인
			bool hasAnimatedShape = false;
			for (const auto& shape : shapes) {
				if (shape.isAnimated) {
					hasAnimatedShape = true;
					break;
				}
			}
			if (hasAnimatedShape) {
				timerRunning = true;
				glutTimerFunc(16, Timer, 1);
			}
		}
		break;
	case 'q':
		exit(0);
		break;
	}
	glutPostRedisplay(); //--- 배경색이 바뀔 때마다 출력 콜백 함수를 호출하여 화면을 refresh 한다
}

GLvoid Timer(int value) {
	if (!animationEnabled) {
		timerRunning = false;
		return;
	}

	bool hasAnimatedShape = false;

	// 애니메이션이 활성화된 모든 도형들 업데이트
	for (auto& shape : shapes) {
		if (shape.isAnimated) {
			hasAnimatedShape = true;
			UpdateShapeAnimation(shape);
		}
	}

	if (!hasAnimatedShape) {
		timerRunning = false;
		return;
	}

	UpdateBuffer();
	glutPostRedisplay();
	glutTimerFunc(16, Timer, 1); // 다음 프레임을 위한 타이머 설정
}

GLvoid Mouse(int button, int state, int x, int y)
{
	float Mouse_x = mapToGLCoordX(x);
	float Mouse_y = mapToGLCoordY(y);
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		selected = -1;

		for (int i = shapes.size() - 1; i >= 0; --i) {
			if (IsMouseInShape(Mouse_x, Mouse_y, shapes[i])) {
				selected = i;
				isDragging = true;
				lastMouseX = Mouse_x;
				lastMouseY = Mouse_y;
				break;
			}
		}
	}
	else if (button == GLUT_LEFT_BUTTON && GLUT_UP) {
		if (isDragging && selected != -1) {
			// 드래그가 끝났을 때 다른 도형과 겹치는지 확인
			for (int i = 0; i < shapes.size(); ++i) {
				if (i != selected && AreShapesOverlapping(shapes[selected], shapes[i])) {
					// 겹치는 도형 발견 시 합치기
					MergeShapes(selected, i);
					break;
				}
			}
		}
		isDragging = false;
		selected = -1;
	}

	glutPostRedisplay(); //--- 배경색이 바뀔 때마다 출력 콜백 함수를 호출하여 화면을 refresh 한다
}

GLvoid Motion(int x, int y)
{
	if (isDragging && selected != -1) {
		float Mouse_x = mapToGLCoordX(x);
		float Mouse_y = mapToGLCoordY(y);

		float deltaX = Mouse_x - lastMouseX;
		float deltaY = Mouse_y - lastMouseY;

		// 선택된 도형의 중심점 이동
		shapes[selected].centerX += deltaX;
		shapes[selected].centerY += deltaY;

		// 해당 도형의 모든 정점들을 이동
		int startIdx = shapes[selected].startIndex * 3;
		int endIdx = startIdx + shapes[selected].vertexCount * 3;

		for (int i = startIdx; i < endIdx; i += 3) {
			allVertices[i] += deltaX;     // x 좌표
			allVertices[i + 1] += deltaY; // y 좌표
		}

		UpdateBuffer();

		lastMouseX = Mouse_x;
		lastMouseY = Mouse_y;

		glutPostRedisplay();
	}
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
	glutMouseFunc(Mouse);
	glutMotionFunc(Motion);
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