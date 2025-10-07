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
GLint width, height;
GLuint shaderProgramID; //--- 세이더 프로그램 이름
GLuint vertexShader; //--- 버텍스 세이더 객체
GLuint fragmentShader; //--- 프래그먼트 세이더 객체

std::random_device rd;  // 시드값을 얻기 위한 random_device 생성.
std::mt19937 gen(rd());	// random_device 를 통해 난수 생성 엔진을 초기화 한다.
std::uniform_real_distribution<float> dis_color(0.0f, 1.0f); // 0.0f 부터 1.0f 까지 균등하게 나타나는 난수열을 생성하기 위해 균등 분포 정의.
std::uniform_real_distribution<float> dis_x(0.1f, 0.9f);
std::uniform_real_distribution<float> dis_y(0.1f, 0.7f);
std::uniform_real_distribution<float> dis_size(0.05f, 0.15f);

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

int shapeType = 0;
int selectedShapeIndex = -1;
int triangleMode = 0; // 0: 면(GL_TRIANGLES), 1: 선(GL_LINE_LOOP)

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

// 이동 관련 변수들
bool num1, num2, num3, num4 = false;
bool timerRunning = false;

// 대각선/지그재그 이동용 변수들
std::vector<float> triangleDirX, triangleDirY;

// 원 스파이럴 이동용 변수들
std::vector<float> spiralAngle, spiralRadius;
std::vector<float> triangleCenterX, triangleCenterY;

// 사각 스파이럴 이동용 변수들
std::vector<int> triangleSpiralDirection, triangleStepsInDirection, triangleStepsToMove;
std::vector<float> triangleStepSize;

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

void InitMovementData() {
	// 삼각형만 이동시키므로 삼각형 개수만큼 벡터 초기화
	int triangleCount = 0;
	for (const auto& shape : shapes) {
		if (shape.shapeType == 2) { // 삼각형만
			triangleCount++;
		}
	}
	
	triangleDirX.resize(triangleCount);
	triangleDirY.resize(triangleCount);
	spiralAngle.resize(triangleCount);
	spiralRadius.resize(triangleCount);
	triangleCenterX.resize(triangleCount);
	triangleCenterY.resize(triangleCount);
	triangleSpiralDirection.resize(triangleCount);
	triangleStepsInDirection.resize(triangleCount);
	triangleStepsToMove.resize(triangleCount);
	triangleStepSize.resize(triangleCount);
	
	// 초기값 설정
	for (int i = 0; i < triangleCount; i++) {
		triangleDirX[i] = (i % 2 == 0) ? 0.02f : 0.015f;
		triangleDirY[i] = (i % 2 == 0) ? 0.02f : 0.015f;
		spiralAngle[i] = 0.0f;
		spiralRadius[i] = (i % 2 == 0) ? 0.1f : 0.08f;
		triangleSpiralDirection[i] = 0;
		triangleStepsInDirection[i] = 0;
		triangleStepsToMove[i] = 1;
		triangleStepSize[i] = (i % 2 == 0) ? 0.02f : 0.015f;
	}
}

void InitShapes() {
	// X축 그리기
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
	LineX.shapeType = 1; // 선 타입
	shapes.push_back(LineX);

	// Y축 그리기
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
	LineY.shapeType = 1; // 선 타입
	shapes.push_back(LineY);

	// 1사분면 삼각형
	Shape triangle1;
	triangle1.startIndex = allVertices.size() / 3;
	triangle1.centerX = dis_x(gen);
	triangle1.centerY = dis_y(gen);
	float x = triangle1.centerX;
	float y = triangle1.centerY;

	allVertices.push_back(x);
	allVertices.push_back(y + 0.3f);
	allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allVertices.push_back(x - 0.1f);
	allVertices.push_back(y - 0.1f);
	allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allVertices.push_back(x + 0.1f);
	allVertices.push_back(y - 0.1f);
	allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	triangle1.drawMode = GL_TRIANGLES;
	triangle1.vertexCount = 3;
	triangle1.shapeType = 2; // 삼각형 타입
	shapes.push_back(triangle1);

	// 2사분면 삼각형
	Shape triangle2;
	triangle2.startIndex = allVertices.size() / 3;
	triangle2.centerX = -dis_x(gen);
	triangle2.centerY = dis_y(gen);
	x = triangle2.centerX;
	y = triangle2.centerY;

	allVertices.push_back(x);
	allVertices.push_back(y + 0.3f);
	allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allVertices.push_back(x - 0.1f);
	allVertices.push_back(y - 0.1f);
	allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allVertices.push_back(x + 0.1f);
	allVertices.push_back(y - 0.1f);
	allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	triangle2.drawMode = GL_TRIANGLES;
	triangle2.vertexCount = 3;
	triangle2.shapeType = 2; // 삼각형 타입
	shapes.push_back(triangle2);

	// 3사분면 삼각형
	Shape triangle3;
	triangle3.startIndex = allVertices.size() / 3;
	triangle3.centerX = -dis_x(gen);
	triangle3.centerY = -dis_y(gen);
	x = triangle3.centerX;
	y = triangle3.centerY;

	allVertices.push_back(x);
	allVertices.push_back(y + 0.3f);
	allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allVertices.push_back(x - 0.1f);
	allVertices.push_back(y - 0.1f);
	allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allVertices.push_back(x + 0.1f);
	allVertices.push_back(y - 0.1f);
	allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	triangle3.drawMode = GL_TRIANGLES;
	triangle3.vertexCount = 3;
	triangle3.shapeType = 2; // 삼각형 타입
	shapes.push_back(triangle3);

	// 4사분면 삼각형
	Shape triangle4;
	triangle4.startIndex = allVertices.size() / 3;
	triangle4.centerX = dis_x(gen);
	triangle4.centerY = -dis_y(gen);
	x = triangle4.centerX;
	y = triangle4.centerY;

	allVertices.push_back(x);
	allVertices.push_back(y + 0.3f);
	allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allVertices.push_back(x - 0.1f);
	allVertices.push_back(y - 0.1f);
	allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allVertices.push_back(x + 0.1f);
	allVertices.push_back(y - 0.1f);
	allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	triangle4.drawMode = GL_TRIANGLES;
	triangle4.vertexCount = 3;
	triangle4.shapeType = 2; // 삼각형 타입
	shapes.push_back(triangle4);

	UpdateBuffer();
	InitMovementData();
}

void AddShape(float x, float y, int shapeType) {
	Shape newShape;
	newShape.startIndex = allVertices.size() / 3;
	
	// 랜덤 크기 생성 (이등변삼각형용)
	float size = dis_size(gen);
	float height = size * 1.5f; // 높이는 밑변보다 크게

	allVertices.push_back(x);
	allVertices.push_back(y + height);
	allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allVertices.push_back(x - size + 0.01f);
	allVertices.push_back(y - height * 1.0f);
	allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allVertices.push_back(x + size - 0.01f);
	allVertices.push_back(y - height * 1.0f);
	allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	
	// 삼각형 그리기 모드에 따라 drawMode 설정
	if (triangleMode == 0) {
		newShape.drawMode = GL_TRIANGLES;  // 면
	} else {
		newShape.drawMode = GL_LINE_LOOP;  // 선
	}

	newShape.vertexCount = 3;
	newShape.centerX = x;
	newShape.centerY = y;
	newShape.shapeType = 2; // 삼각형 타입 설정

	shapes.push_back(newShape);
	UpdateBuffer();
	InitMovementData(); // 새 삼각형 추가 시 이동 데이터 다시 초기화
}

int CountTrianglesInQuadrant(int quadrant) {
    int count = 0;
    for (const auto& shape : shapes) {
        if (shape.drawMode == GL_TRIANGLES && 
            (&shape - &shapes[0]) >= 2) {
            
            float x = shape.centerX;
            float y = shape.centerY;
            
            switch (quadrant) {
            case 1:
                if (x > 0 && y > 0) count++;
                break;
            case 2:
                if (x < 0 && y > 0) count++;
                break;
            case 3:
                if (x < 0 && y < 0) count++;
                break;
            case 4:
                if (x > 0 && y < 0) count++;
                break;
            }
        } else if (shape.drawMode == GL_LINE_LOOP &&
			(&shape - &shapes[0]) >= 2) {

			float x = shape.centerX;
			float y = shape.centerY;

			switch (quadrant) {
			case 1:
				if (x > 0 && y > 0) count++;
				break;
			case 2:
				if (x < 0 && y > 0) count++;
				break;
			case 3:
				if (x < 0 && y < 0) count++;
				break;
			case 4:
				if (x > 0 && y < 0) count++;
				break;
			}
		}
    }
    return count;
}

void RemoveInitTriangleInQuadrant(int quadrant) {
    // 새로운 벡터들을 만들어서 안전하게 재구성
    std::vector<GLfloat> newVertices;
    std::vector<GLfloat> newColors;
    std::vector<Shape> newShapes;
    
    // 축(선) 유지 - 인덱스 0, 1
    for (int i = 0; i < std::min(2, (int)shapes.size()); i++) {
        Shape newShape = shapes[i];
        newShape.startIndex = newVertices.size() / 3;
        
        int startVertex = shapes[i].startIndex * 3;
        int vertexCount = shapes[i].vertexCount * 3;
        
        for (int j = 0; j < vertexCount; j++) {
            newVertices.push_back(allVertices[startVertex + j]);
            newColors.push_back(allColors[startVertex + j]);
        }
        
        newShapes.push_back(newShape);
    }
    
    // 나머지 삼각형들 중에서 해당 사분면이 아닌 것들만 유지
    for (int i = 2; i < shapes.size(); i++) {
        if (shapes[i].drawMode == GL_TRIANGLES || shapes[i].drawMode == GL_LINE_LOOP) {
            float x = shapes[i].centerX;
            float y = shapes[i].centerY;
            
            bool shouldKeep = true;
            switch (quadrant) {				
            case 1:
                if (x > 0 && y > 0) shouldKeep = false;
                break;
            case 2:
                if (x < 0 && y > 0) shouldKeep = false;
                break;
            case 3:
                if (x < 0 && y < 0) shouldKeep = false;
                break;
            case 4:
                if (x > 0 && y < 0) shouldKeep = false;
                break;
            }
            
            if (shouldKeep) {
                Shape newShape = shapes[i];
                newShape.startIndex = newVertices.size() / 3;
                
                int startVertex = shapes[i].startIndex * 3;
                int vertexCount = shapes[i].vertexCount * 3;
                
                for (int j = 0; j < vertexCount; j++) {
                    newVertices.push_back(allVertices[startVertex + j]);
                    newColors.push_back(allColors[startVertex + j]);
                }
                
                newShapes.push_back(newShape);
            }
        }
    }
    
    // 새로운 데이터로 교체
    allVertices = newVertices;
    allColors = newColors;
    shapes = newShapes;
    
    UpdateBuffer();
    InitMovementData(); // 삼각형 제거 후 이동 데이터 다시 초기화
}

void RemoveFirstTriangleInQuadrant(int quadrant) {
	// 새로운 벡터들을 만들어서 안전하게 재구성
	std::vector<GLfloat> newVertices;
	std::vector<GLfloat> newColors;
	std::vector<Shape> newShapes;

	// 축(선) 유지 - 인덱스 0, 1
	for (int i = 0; i < std::min(2, (int)shapes.size()); i++) {
		Shape newShape = shapes[i];
		newShape.startIndex = newVertices.size() / 3;

		int startVertex = shapes[i].startIndex * 3;
		int vertexCount = shapes[i].vertexCount * 3;

		for (int j = 0; j < vertexCount; j++) {
			newVertices.push_back(allVertices[startVertex + j]);
			newColors.push_back(allColors[startVertex + j]);
		}

		newShapes.push_back(newShape);
	}

	bool firstInQuadrantRemoved = false;

	// 나머지 삼각형들을 처리
	for (int i = 2; i < shapes.size(); i++) {
		if (shapes[i].drawMode == GL_TRIANGLES || shapes[i].drawMode == GL_LINE_LOOP) {
			float x = shapes[i].centerX;
			float y = shapes[i].centerY;

			bool isInTargetQuadrant = false;
			switch (quadrant) {
			case 1:
				if (x > 0 && y > 0) isInTargetQuadrant = true;
				break;
			case 2:
				if (x < 0 && y > 0) isInTargetQuadrant = true;
				break;
			case 3:
				if (x < 0 && y < 0) isInTargetQuadrant = true;
				break;
			case 4:
				if (x > 0 && y < 0) isInTargetQuadrant = true;
				break;
			}

			// 해당 사분면의 첫 번째 삼각형은 제거, 나머지는 유지
			if (isInTargetQuadrant && !firstInQuadrantRemoved) {
				firstInQuadrantRemoved = true;
				continue; // 첫 번째 삼각형 제거
			}

			// 나머지 삼각형들은 유지
			Shape newShape = shapes[i];
			newShape.startIndex = newVertices.size() / 3;

			int startVertex = shapes[i].startIndex * 3;
			int vertexCount = shapes[i].vertexCount * 3;

			for (int j = 0; j < vertexCount; j++) {
				newVertices.push_back(allVertices[startVertex + j]);
				newColors.push_back(allColors[startVertex + j]);
			}

			newShapes.push_back(newShape);
		}
	}

	// 새로운 데이터로 교체
	allVertices = newVertices;
	allColors = newColors;
	shapes = newShapes;

	UpdateBuffer();
	InitMovementData(); // 삼각형 제거 후 이동 데이터 다시 초기화
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
		triangleMode = 0; // 면
		break;
	case 'b':
		triangleMode = 1; // 선
		break;
	case 'c':
		allVertices.clear();
		allColors.clear();
		shapes.clear();
		selectedShapeIndex = -1;
		// 타이머 정지
		num1 = num2 = num3 = num4 = false;
		timerRunning = false;
		InitShapes();
		break;
	case '1':		// 대각선 이동
		num1 = !num1;
		if (num1) {
			num2 = num3 = num4 = false;

			if (!timerRunning) {
				timerRunning = true;
				glutTimerFunc(16, TimerFunction, 1);
			}
		}
		break;
	case '2':		// 지그재그 이동
		num2 = !num2;
		if (num2) {
			num1 = num3 = num4 = false;

			if (!timerRunning) {
				timerRunning = true;
				glutTimerFunc(16, TimerFunction, 1);
			}
		}
		break;
	case '3':		// 사각 스파이럴 이동
		num3 = !num3;
		if (num3) {
			num1 = num2 = num4 = false;
			
			// 사각 스파이럴 초기화
			int triangleIndex = 0;
			for (int i = 0; i < shapes.size(); i++) {
				if (shapes[i].shapeType == 2) { // 삼각형만
					triangleSpiralDirection[triangleIndex] = 0;
					triangleStepsInDirection[triangleIndex] = 0;
					triangleStepsToMove[triangleIndex] = 1;
					triangleCenterX[triangleIndex] = shapes[i].centerX;
					triangleCenterY[triangleIndex] = shapes[i].centerY;
					triangleIndex++;
				}
			}

			if (!timerRunning) {
				timerRunning = true;
				glutTimerFunc(16, TimerFunction, 1);
			}
		}
		break;
	case '4':		// 원 스파이럴 이동
		num4 = !num4;
		if (num4) {
			num1 = num2 = num3 = false;
			
			// 원 스파이럴 초기화
			int triangleIndex = 0;
			for (int i = 0; i < shapes.size(); i++) {
				if (shapes[i].shapeType == 2) { // 삼각형만
					spiralAngle[triangleIndex] = 0.0f;
					spiralRadius[triangleIndex] = (triangleIndex % 2 == 0) ? 0.1f : 0.08f;
					triangleCenterX[triangleIndex] = shapes[i].centerX;
					triangleCenterY[triangleIndex] = shapes[i].centerY;
					triangleIndex++;
				}
			}

			if (!timerRunning) {
				timerRunning = true;
				glutTimerFunc(16, TimerFunction, 1);
			}
		}
		break;
	case 'q':
		exit(0);
		break;
	}
	glutPostRedisplay();
}

GLvoid Mouse(int button, int state, int x, int y)
{
	float Mouse_x = mapToGLCoordX(x);
	float Mouse_y = mapToGLCoordY(y);
	
	if (state == GLUT_DOWN) {
		int quadrant = 0;
		
		// 사분면 결정
		if (Mouse_x > 0 && Mouse_y > 0) {
			quadrant = 1; // 1사분면
		} else if (Mouse_x < 0 && Mouse_y > 0) {
			quadrant = 2; // 2사분면
		} else if (Mouse_x < 0 && Mouse_y < 0) {
			quadrant = 3; // 3사분면
		} else if (Mouse_x > 0 && Mouse_y < 0) {
			quadrant = 4; // 4사분면
		}
		
		if (quadrant > 0) { // 축이 아닌 사분면에 클릭한 경우
			if (button == GLUT_LEFT_BUTTON) {
				// 왼쪽 클릭: 해당 사분면의 기존 삼각형들 제거 후 새 삼각형 생성
				RemoveInitTriangleInQuadrant(quadrant);
				AddShape(Mouse_x, Mouse_y, shapeType);
			}
			else if (button == GLUT_RIGHT_BUTTON) {
				// 오른쪽 클릭: 해당 사분면에 최대 4개까지 삼각형 추가
				if (CountTrianglesInQuadrant(quadrant) < 4) {
					AddShape(Mouse_x, Mouse_y, shapeType);
				}
				else if (CountTrianglesInQuadrant(quadrant) >= 4) {
					RemoveFirstTriangleInQuadrant(quadrant);
					AddShape(Mouse_x, Mouse_y, shapeType);
				}
			}
		}
	}

	glutPostRedisplay();
}

GLvoid TimerFunction(int value) {
	if (!num1 && !num2 && !num3 && !num4) {
		timerRunning = false;
		return;
	}

	int triangleIndex = 0;
	
	for (int i = 0; i < shapes.size(); i++) {
		if (shapes[i].shapeType != 2) continue; // 삼각형이 아니면 스킵
		
		Shape& triangle = shapes[i];
		
		if (num1) {		// 대각선 이동
			// 경계 체크
			if (triangle.centerX + 0.1f >= 1.0f || triangle.centerX - 0.1f <= -1.0f) {
				triangleDirX[triangleIndex] *= -1;
			}
			if (triangle.centerY + 0.3f >= 1.0f || triangle.centerY - 0.1f <= -1.0f) {
				triangleDirY[triangleIndex] *= -1;
			}

			// 모든 정점 이동
			for (int j = 0; j < 3; ++j) {
				allVertices[(triangle.startIndex + j) * 3] += triangleDirX[triangleIndex];
				allVertices[(triangle.startIndex + j) * 3 + 1] += triangleDirY[triangleIndex];
			}

			triangle.centerX += triangleDirX[triangleIndex];
			triangle.centerY += triangleDirY[triangleIndex];
		}
		else if (num2) {		// 지그재그 이동
			// 좌우 경계에 닿으면 방향 바꾸고 세로로 이동
			if (triangle.centerX + 0.1f >= 1.0f || triangle.centerX - 0.1f <= -1.0f) {
				triangleDirX[triangleIndex] *= -1;
				for (int j = 0; j < 3; ++j) {
					allVertices[(triangle.startIndex + j) * 3 + 1] += triangleDirY[triangleIndex];
				}
				triangle.centerY += triangleDirY[triangleIndex];
			}
			if (triangle.centerY + 0.5f >= 1.0f || triangle.centerY - 0.2f <= -1.0f) {
				triangleDirY[triangleIndex] *= -1;
			}

			// x 값 변경
			for (int j = 0; j < 3; ++j) {
				allVertices[(triangle.startIndex + j) * 3] += triangleDirX[triangleIndex];
			}

			triangle.centerX += triangleDirX[triangleIndex];
		}
		else if (num3) {		// 사각 스파이럴 이동
			float deltaX = 0.0f, deltaY = 0.0f;
			switch (triangleSpiralDirection[triangleIndex]) {
			case 0: deltaX = triangleStepSize[triangleIndex]; break;   // 오른쪽
			case 1: deltaY = -triangleStepSize[triangleIndex]; break;  // 아래
			case 2: deltaX = -triangleStepSize[triangleIndex]; break;  // 왼쪽
			case 3: deltaY = triangleStepSize[triangleIndex]; break;   // 위
			}

			// 모든 정점 이동
			for (int j = 0; j < 3; ++j) {
				allVertices[(triangle.startIndex + j) * 3] += deltaX;
				allVertices[(triangle.startIndex + j) * 3 + 1] += deltaY;
			}

			triangle.centerX += deltaX;
			triangle.centerY += deltaY;

			// 방향 관리
			triangleStepsInDirection[triangleIndex]++;
			if (triangleStepsInDirection[triangleIndex] >= triangleStepsToMove[triangleIndex]) {
				triangleStepsInDirection[triangleIndex] = 0;
				triangleSpiralDirection[triangleIndex] = (triangleSpiralDirection[triangleIndex] + 1) % 4;
				
				// 오른쪽이나 왼쪽 방향이 끝나면 이동 거리 증가
				if (triangleSpiralDirection[triangleIndex] == 2 || triangleSpiralDirection[triangleIndex] == 0) {
					triangleStepsToMove[triangleIndex]++;
				}
			}

			// 화면 경계를 벗어나면 리셋
			if (abs(triangle.centerX - triangleCenterX[triangleIndex]) > 0.8f || 
				abs(triangle.centerY - triangleCenterY[triangleIndex]) > 0.8f) {
				triangleSpiralDirection[triangleIndex] = 0;
				triangleStepsInDirection[triangleIndex] = 0;
				triangleStepsToMove[triangleIndex] = 1;
				triangle.centerX = triangleCenterX[triangleIndex];
				triangle.centerY = triangleCenterY[triangleIndex];
				
				// 정점들도 중심으로 리셋
				allVertices[triangle.startIndex * 3] = triangleCenterX[triangleIndex];
				allVertices[triangle.startIndex * 3 + 1] = triangleCenterY[triangleIndex] + 0.3f;
				allVertices[(triangle.startIndex + 1) * 3] = triangleCenterX[triangleIndex] - 0.1f;
				allVertices[(triangle.startIndex + 1) * 3 + 1] = triangleCenterY[triangleIndex] - 0.1f;
				allVertices[(triangle.startIndex + 2) * 3] = triangleCenterX[triangleIndex] + 0.1f;
				allVertices[(triangle.startIndex + 2) * 3 + 1] = triangleCenterY[triangleIndex] - 0.1f;
			}
		}
		else if (num4) {		// 원 스파이럴 이동
			// 각도 증가 (각 삼각형마다 다른 속도)
			spiralAngle[triangleIndex] += (triangleIndex % 2 == 0) ? 0.1f : 0.15f;

			// 반지름 증가 (스파이럴 효과)
			spiralRadius[triangleIndex] += (triangleIndex % 2 == 0) ? 0.002f : 0.001f;

			// 새로운 위치 계산
			float newCenterX = triangleCenterX[triangleIndex] + spiralRadius[triangleIndex] * cos(spiralAngle[triangleIndex]);
			float newCenterY = triangleCenterY[triangleIndex] + spiralRadius[triangleIndex] * sin(spiralAngle[triangleIndex]);

			// 이동할 거리 계산
			float deltaX = newCenterX - triangle.centerX;
			float deltaY = newCenterY - triangle.centerY;

			// 모든 정점을 새로운 위치로 이동
			for (int j = 0; j < 3; ++j) {
				allVertices[(triangle.startIndex + j) * 3] += deltaX;
				allVertices[(triangle.startIndex + j) * 3 + 1] += deltaY;
			}

			// 중심점 업데이트
			triangle.centerX = newCenterX;
			triangle.centerY = newCenterY;

			// 화면 경계를 벗어나면 리셋
			if (spiralRadius[triangleIndex] > 0.8f) {
				spiralRadius[triangleIndex] = (triangleIndex % 2 == 0) ? 0.1f : 0.08f;
				spiralAngle[triangleIndex] = 0.0f;
			}
		}

		triangleIndex++;
	}

	UpdateBuffer();
	glutPostRedisplay();
	glutTimerFunc(16, TimerFunction, 1); // 다음 프레임을 위한 타이머 설정
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
	glutCreateWindow("Practice 10 - Complete Triangle System");
	//--- GLEW 초기화하기
	glewExperimental = GL_TRUE;
	glewInit();
	shaderProgramID = make_shaderProgram();
	InitBuffer();
	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
	glutMouseFunc(Mouse);
	InitShapes();
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