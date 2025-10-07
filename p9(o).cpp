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
std::uniform_real_distribution<float> dis_x(0.1f, 0.9f); // -0.1f 부터 0.1f 까지 균등하게 나타나는 난수열을 생성하기 위해 균등 분포 정의.
std::uniform_real_distribution<float> dis_y(0.1f, 0.7f); // -0.1f 부터 0.1f 까지 균등하게 나타나는 난수열을 생성하기 위해 균등 분포 정의.
std::uniform_real_distribution<float> dis_size(0.05f, 0.15f); // 삼각형 크기용 랜덤 분포

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
	LineX.shapeType = 1; // 선 타입

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
	LineY.shapeType = 1; // 선 타입

	shapes.push_back(LineY);
	UpdateBuffer();

	// 1사분면
	Shape newShape1;
	newShape1.startIndex = allVertices.size() / 3;
	newShape1.centerX = dis_x(gen);
	newShape1.centerY = dis_y(gen);
	float x = newShape1.centerX;
	float y = newShape1.centerY;

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
	newShape1.drawMode = GL_TRIANGLES;
	newShape1.vertexCount = 3;
	newShape1.shapeType = 2; // 삼각형 타입

	shapes.push_back(newShape1);
	UpdateBuffer();

	// 2사분면
	Shape newShape2;
	newShape2.startIndex = allVertices.size() / 3;
	newShape2.centerX = dis_x(gen);
	newShape2.centerY = dis_y(gen);
	newShape2.centerX = -newShape2.centerX;

	allVertices.push_back(newShape2.centerX);
	allVertices.push_back(y + 0.3f);
	allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allVertices.push_back(newShape2.centerX - 0.1f);
	allVertices.push_back(y - 0.1f);
	allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allVertices.push_back(newShape2.centerX + 0.1f);
	allVertices.push_back(y - 0.1f);
	allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	newShape2.drawMode = GL_TRIANGLES;
	newShape2.vertexCount = 3;
	newShape2.shapeType = 2; // 삼각형 타입

	shapes.push_back(newShape2);
	UpdateBuffer();

	// 3사분면
	Shape newShape3;
	newShape3.startIndex = allVertices.size() / 3;
	newShape3.centerX = dis_x(gen);
	newShape3.centerY = dis_y(gen);
	newShape3.centerX = -newShape3.centerX;
	newShape3.centerY = -newShape3.centerY;

	allVertices.push_back(newShape3.centerX);
	allVertices.push_back(newShape3.centerY + 0.3f);
	allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allVertices.push_back(newShape3.centerX - 0.1f);
	allVertices.push_back(newShape3.centerY - 0.1f);
	allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allVertices.push_back(newShape3.centerX + 0.1f);
	allVertices.push_back(newShape3.centerY - 0.1f);
	allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	newShape3.drawMode = GL_TRIANGLES;
	newShape3.vertexCount = 3;
	newShape3.shapeType = 2; // 삼각형 타입

	shapes.push_back(newShape3);
	UpdateBuffer();

	// 4사분면
	Shape newShape4;
	newShape4.startIndex = allVertices.size() / 3;
	newShape4.centerX = dis_x(gen);
	newShape4.centerY = dis_y(gen);
	x = newShape4.centerX;
	newShape4.centerY = -newShape4.centerY;

	allVertices.push_back(x);
	allVertices.push_back(newShape4.centerY + 0.3f);
	allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allVertices.push_back(x - 0.1f);
	allVertices.push_back(newShape4.centerY - 0.1f);
	allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allVertices.push_back(x + 0.1f);
	allVertices.push_back(newShape4.centerY - 0.1f);
	allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	newShape4.drawMode = GL_TRIANGLES;
	newShape4.vertexCount = 3;
	newShape4.shapeType = 2; // 삼각형 타입

	shapes.push_back(newShape4);
	UpdateBuffer();
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
	allVertices.push_back(x - size);
	allVertices.push_back(y - height * 0.5f);
	allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allVertices.push_back(x + size);
	allVertices.push_back(y - height * 0.5f);
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
        if (shapes[i].drawMode == GL_TRIANGLES) {
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
        }  else if (shapes[i].drawMode == GL_LINE_LOOP) {
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
		InitShapes();
		break;
	case 'q':
		exit(0);
		break;
	}
	glutPostRedisplay(); //--- 배경색이 바뀔 때마다 출력 콜백 함수를 호출하여 화면을 refresh 한다
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