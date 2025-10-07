#define _CRT_SECURE_NO_WARNINGS //--- ���α׷� �� �տ� ������ ��
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
//--- �ʿ��� ������� ����
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <random>
#include <vector>
#include <cmath>
//--- �Ʒ� 5�� �Լ��� ����� ���� �Լ���
void make_vertexShaders();
void make_fragmentShaders();
GLuint make_shaderProgram();
GLvoid drawScene();
GLvoid Reshape(int w, int h);
GLvoid TimerFunction(int value);
//--- �ʿ��� ���� ����
GLint width, height;
GLuint shaderProgramID; //--- ���̴� ���α׷� �̸�
GLuint vertexShader; //--- ���ؽ� ���̴� ��ü
GLuint fragmentShader; //--- �����׸�Ʈ ���̴� ��ü

std::random_device rd;  // �õ尪�� ��� ���� random_device ����.
std::mt19937 gen(rd());	// random_device �� ���� ���� ���� ������ �ʱ�ȭ �Ѵ�.
std::uniform_real_distribution<float> dis_color(0.0f, 1.0f); // 0.0f ���� 1.0f ���� �յ��ϰ� ��Ÿ���� �������� �����ϱ� ���� �յ� ���� ����.
std::uniform_real_distribution<float> dis_x(0.1f, 0.9f);
std::uniform_real_distribution<float> dis_y(0.1f, 0.7f);
std::uniform_real_distribution<float> dis_size(0.05f, 0.15f);

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

int shapeType = 0;
int selectedShapeIndex = -1;
int triangleMode = 0; // 0: ��(GL_TRIANGLES), 1: ��(GL_LINE_LOOP)

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

// �̵� ���� ������
bool num1, num2, num3, num4 = false;
bool timerRunning = false;

// �밢��/������� �̵��� ������
std::vector<float> triangleDirX, triangleDirY;

// �� �����̷� �̵��� ������
std::vector<float> spiralAngle, spiralRadius;
std::vector<float> triangleCenterX, triangleCenterY;

// �簢 �����̷� �̵��� ������
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
	// �ﰢ���� �̵���Ű�Ƿ� �ﰢ�� ������ŭ ���� �ʱ�ȭ
	int triangleCount = 0;
	for (const auto& shape : shapes) {
		if (shape.shapeType == 2) { // �ﰢ����
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
	
	// �ʱⰪ ����
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
	// X�� �׸���
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
	LineX.shapeType = 1; // �� Ÿ��
	shapes.push_back(LineX);

	// Y�� �׸���
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
	LineY.shapeType = 1; // �� Ÿ��
	shapes.push_back(LineY);

	// 1��и� �ﰢ��
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
	triangle1.shapeType = 2; // �ﰢ�� Ÿ��
	shapes.push_back(triangle1);

	// 2��и� �ﰢ��
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
	triangle2.shapeType = 2; // �ﰢ�� Ÿ��
	shapes.push_back(triangle2);

	// 3��и� �ﰢ��
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
	triangle3.shapeType = 2; // �ﰢ�� Ÿ��
	shapes.push_back(triangle3);

	// 4��и� �ﰢ��
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
	triangle4.shapeType = 2; // �ﰢ�� Ÿ��
	shapes.push_back(triangle4);

	UpdateBuffer();
	InitMovementData();
}

void AddShape(float x, float y, int shapeType) {
	Shape newShape;
	newShape.startIndex = allVertices.size() / 3;
	
	// ���� ũ�� ���� (�̵�ﰢ����)
	float size = dis_size(gen);
	float height = size * 1.5f; // ���̴� �غ����� ũ��

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
	
	// �ﰢ�� �׸��� ��忡 ���� drawMode ����
	if (triangleMode == 0) {
		newShape.drawMode = GL_TRIANGLES;  // ��
	} else {
		newShape.drawMode = GL_LINE_LOOP;  // ��
	}

	newShape.vertexCount = 3;
	newShape.centerX = x;
	newShape.centerY = y;
	newShape.shapeType = 2; // �ﰢ�� Ÿ�� ����

	shapes.push_back(newShape);
	UpdateBuffer();
	InitMovementData(); // �� �ﰢ�� �߰� �� �̵� ������ �ٽ� �ʱ�ȭ
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
    // ���ο� ���͵��� ���� �����ϰ� �籸��
    std::vector<GLfloat> newVertices;
    std::vector<GLfloat> newColors;
    std::vector<Shape> newShapes;
    
    // ��(��) ���� - �ε��� 0, 1
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
    
    // ������ �ﰢ���� �߿��� �ش� ��и��� �ƴ� �͵鸸 ����
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
    
    // ���ο� �����ͷ� ��ü
    allVertices = newVertices;
    allColors = newColors;
    shapes = newShapes;
    
    UpdateBuffer();
    InitMovementData(); // �ﰢ�� ���� �� �̵� ������ �ٽ� �ʱ�ȭ
}

void RemoveFirstTriangleInQuadrant(int quadrant) {
	// ���ο� ���͵��� ���� �����ϰ� �籸��
	std::vector<GLfloat> newVertices;
	std::vector<GLfloat> newColors;
	std::vector<Shape> newShapes;

	// ��(��) ���� - �ε��� 0, 1
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

	// ������ �ﰢ������ ó��
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

			// �ش� ��и��� ù ��° �ﰢ���� ����, �������� ����
			if (isInTargetQuadrant && !firstInQuadrantRemoved) {
				firstInQuadrantRemoved = true;
				continue; // ù ��° �ﰢ�� ����
			}

			// ������ �ﰢ������ ����
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

	// ���ο� �����ͷ� ��ü
	allVertices = newVertices;
	allColors = newColors;
	shapes = newShapes;

	UpdateBuffer();
	InitMovementData(); // �ﰢ�� ���� �� �̵� ������ �ٽ� �ʱ�ȭ
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

//--- �׸��� �ݹ� �Լ�
GLvoid drawScene()
{
	//--- ����� ���� ����
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (allVertices.empty()) {
		glutSwapBuffers();
		return;
	}

	//--- ������ ���������ο� ���̴� �ҷ�����
	glUseProgram(shaderProgramID);
	//--- ����� VAO �ҷ�����
	glBindVertexArray(vao);

	// �� ũ�� ����
	glPointSize(10.0f);

	// �� �������� �׸���
	for (const auto& shape : shapes) {
		glDrawArrays(shape.drawMode, shape.startIndex, shape.vertexCount);
	}

	glutSwapBuffers(); //--- ȭ�鿡 ����ϱ�
}

GLvoid Keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 'a':
		triangleMode = 0; // ��
		break;
	case 'b':
		triangleMode = 1; // ��
		break;
	case 'c':
		allVertices.clear();
		allColors.clear();
		shapes.clear();
		selectedShapeIndex = -1;
		// Ÿ�̸� ����
		num1 = num2 = num3 = num4 = false;
		timerRunning = false;
		InitShapes();
		break;
	case '1':		// �밢�� �̵�
		num1 = !num1;
		if (num1) {
			num2 = num3 = num4 = false;

			if (!timerRunning) {
				timerRunning = true;
				glutTimerFunc(16, TimerFunction, 1);
			}
		}
		break;
	case '2':		// ������� �̵�
		num2 = !num2;
		if (num2) {
			num1 = num3 = num4 = false;

			if (!timerRunning) {
				timerRunning = true;
				glutTimerFunc(16, TimerFunction, 1);
			}
		}
		break;
	case '3':		// �簢 �����̷� �̵�
		num3 = !num3;
		if (num3) {
			num1 = num2 = num4 = false;
			
			// �簢 �����̷� �ʱ�ȭ
			int triangleIndex = 0;
			for (int i = 0; i < shapes.size(); i++) {
				if (shapes[i].shapeType == 2) { // �ﰢ����
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
	case '4':		// �� �����̷� �̵�
		num4 = !num4;
		if (num4) {
			num1 = num2 = num3 = false;
			
			// �� �����̷� �ʱ�ȭ
			int triangleIndex = 0;
			for (int i = 0; i < shapes.size(); i++) {
				if (shapes[i].shapeType == 2) { // �ﰢ����
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
		
		// ��и� ����
		if (Mouse_x > 0 && Mouse_y > 0) {
			quadrant = 1; // 1��и�
		} else if (Mouse_x < 0 && Mouse_y > 0) {
			quadrant = 2; // 2��и�
		} else if (Mouse_x < 0 && Mouse_y < 0) {
			quadrant = 3; // 3��и�
		} else if (Mouse_x > 0 && Mouse_y < 0) {
			quadrant = 4; // 4��и�
		}
		
		if (quadrant > 0) { // ���� �ƴ� ��и鿡 Ŭ���� ���
			if (button == GLUT_LEFT_BUTTON) {
				// ���� Ŭ��: �ش� ��и��� ���� �ﰢ���� ���� �� �� �ﰢ�� ����
				RemoveInitTriangleInQuadrant(quadrant);
				AddShape(Mouse_x, Mouse_y, shapeType);
			}
			else if (button == GLUT_RIGHT_BUTTON) {
				// ������ Ŭ��: �ش� ��и鿡 �ִ� 4������ �ﰢ�� �߰�
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
		if (shapes[i].shapeType != 2) continue; // �ﰢ���� �ƴϸ� ��ŵ
		
		Shape& triangle = shapes[i];
		
		if (num1) {		// �밢�� �̵�
			// ��� üũ
			if (triangle.centerX + 0.1f >= 1.0f || triangle.centerX - 0.1f <= -1.0f) {
				triangleDirX[triangleIndex] *= -1;
			}
			if (triangle.centerY + 0.3f >= 1.0f || triangle.centerY - 0.1f <= -1.0f) {
				triangleDirY[triangleIndex] *= -1;
			}

			// ��� ���� �̵�
			for (int j = 0; j < 3; ++j) {
				allVertices[(triangle.startIndex + j) * 3] += triangleDirX[triangleIndex];
				allVertices[(triangle.startIndex + j) * 3 + 1] += triangleDirY[triangleIndex];
			}

			triangle.centerX += triangleDirX[triangleIndex];
			triangle.centerY += triangleDirY[triangleIndex];
		}
		else if (num2) {		// ������� �̵�
			// �¿� ��迡 ������ ���� �ٲٰ� ���η� �̵�
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

			// x �� ����
			for (int j = 0; j < 3; ++j) {
				allVertices[(triangle.startIndex + j) * 3] += triangleDirX[triangleIndex];
			}

			triangle.centerX += triangleDirX[triangleIndex];
		}
		else if (num3) {		// �簢 �����̷� �̵�
			float deltaX = 0.0f, deltaY = 0.0f;
			switch (triangleSpiralDirection[triangleIndex]) {
			case 0: deltaX = triangleStepSize[triangleIndex]; break;   // ������
			case 1: deltaY = -triangleStepSize[triangleIndex]; break;  // �Ʒ�
			case 2: deltaX = -triangleStepSize[triangleIndex]; break;  // ����
			case 3: deltaY = triangleStepSize[triangleIndex]; break;   // ��
			}

			// ��� ���� �̵�
			for (int j = 0; j < 3; ++j) {
				allVertices[(triangle.startIndex + j) * 3] += deltaX;
				allVertices[(triangle.startIndex + j) * 3 + 1] += deltaY;
			}

			triangle.centerX += deltaX;
			triangle.centerY += deltaY;

			// ���� ����
			triangleStepsInDirection[triangleIndex]++;
			if (triangleStepsInDirection[triangleIndex] >= triangleStepsToMove[triangleIndex]) {
				triangleStepsInDirection[triangleIndex] = 0;
				triangleSpiralDirection[triangleIndex] = (triangleSpiralDirection[triangleIndex] + 1) % 4;
				
				// �������̳� ���� ������ ������ �̵� �Ÿ� ����
				if (triangleSpiralDirection[triangleIndex] == 2 || triangleSpiralDirection[triangleIndex] == 0) {
					triangleStepsToMove[triangleIndex]++;
				}
			}

			// ȭ�� ��踦 ����� ����
			if (abs(triangle.centerX - triangleCenterX[triangleIndex]) > 0.8f || 
				abs(triangle.centerY - triangleCenterY[triangleIndex]) > 0.8f) {
				triangleSpiralDirection[triangleIndex] = 0;
				triangleStepsInDirection[triangleIndex] = 0;
				triangleStepsToMove[triangleIndex] = 1;
				triangle.centerX = triangleCenterX[triangleIndex];
				triangle.centerY = triangleCenterY[triangleIndex];
				
				// �����鵵 �߽����� ����
				allVertices[triangle.startIndex * 3] = triangleCenterX[triangleIndex];
				allVertices[triangle.startIndex * 3 + 1] = triangleCenterY[triangleIndex] + 0.3f;
				allVertices[(triangle.startIndex + 1) * 3] = triangleCenterX[triangleIndex] - 0.1f;
				allVertices[(triangle.startIndex + 1) * 3 + 1] = triangleCenterY[triangleIndex] - 0.1f;
				allVertices[(triangle.startIndex + 2) * 3] = triangleCenterX[triangleIndex] + 0.1f;
				allVertices[(triangle.startIndex + 2) * 3 + 1] = triangleCenterY[triangleIndex] - 0.1f;
			}
		}
		else if (num4) {		// �� �����̷� �̵�
			// ���� ���� (�� �ﰢ������ �ٸ� �ӵ�)
			spiralAngle[triangleIndex] += (triangleIndex % 2 == 0) ? 0.1f : 0.15f;

			// ������ ���� (�����̷� ȿ��)
			spiralRadius[triangleIndex] += (triangleIndex % 2 == 0) ? 0.002f : 0.001f;

			// ���ο� ��ġ ���
			float newCenterX = triangleCenterX[triangleIndex] + spiralRadius[triangleIndex] * cos(spiralAngle[triangleIndex]);
			float newCenterY = triangleCenterY[triangleIndex] + spiralRadius[triangleIndex] * sin(spiralAngle[triangleIndex]);

			// �̵��� �Ÿ� ���
			float deltaX = newCenterX - triangle.centerX;
			float deltaY = newCenterY - triangle.centerY;

			// ��� ������ ���ο� ��ġ�� �̵�
			for (int j = 0; j < 3; ++j) {
				allVertices[(triangle.startIndex + j) * 3] += deltaX;
				allVertices[(triangle.startIndex + j) * 3 + 1] += deltaY;
			}

			// �߽��� ������Ʈ
			triangle.centerX = newCenterX;
			triangle.centerY = newCenterY;

			// ȭ�� ��踦 ����� ����
			if (spiralRadius[triangleIndex] > 0.8f) {
				spiralRadius[triangleIndex] = (triangleIndex % 2 == 0) ? 0.1f : 0.08f;
				spiralAngle[triangleIndex] = 0.0f;
			}
		}

		triangleIndex++;
	}

	UpdateBuffer();
	glutPostRedisplay();
	glutTimerFunc(16, TimerFunction, 1); // ���� �������� ���� Ÿ�̸� ����
}

//--- �ٽñ׸��� �ݹ� �Լ�
GLvoid Reshape(int w, int h)
{
	glViewport(0, 0, w, h);
}

//--- ���ؽ� ���̴� ��ü �����
void make_vertexShaders()
{
	GLchar* vertexSource;
	//--- ���ؽ� ���̴� �о� �����ϰ� ������ �ϱ�
	//--- filetobuf: ��������� �Լ��� �ؽ�Ʈ�� �о ���ڿ��� �����ϴ� �Լ�
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
		std::cerr << "ERROR: vertex shader ������ ����\n" << errorLog << std::endl;
		free(vertexSource); // ���̴� �Լ��鿡�� �޸� ����
		return;
	}
	free(vertexSource); // ���̴� �Լ��鿡�� �޸� ����
}

//--- �����׸�Ʈ ���̴� ��ü �����
void make_fragmentShaders()
{
	GLchar* fragmentSource;
	//--- �����׸�Ʈ ���̴� �о� �����ϰ� �������ϱ�
	fragmentSource = filetobuf("fragment.glsl"); // �����׼��̴� �о����
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
		std::cerr << "ERROR: frag_shader ������ ����\n" << errorLog << std::endl;
		free(fragmentSource); // ���̴� �Լ��鿡�� �޸� ����
		return;
	}
	free(fragmentSource); // ���̴� �Լ��鿡�� �޸� ����
}

void InitBuffer()
{
	glGenVertexArrays(1, &vao); //--- VAO �� �����ϰ� �Ҵ��ϱ�
	glBindVertexArray(vao); //--- VAO�� ���ε��ϱ�
	glGenBuffers(2, vbo); //--- 2���� VBO�� �����ϰ� �Ҵ��ϱ�

	// �ʱ⿡�� �� ���۷� ����
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
}

void main(int argc, char** argv) //--- ������ ����ϰ� �ݹ��Լ� ����
{
	//--- ������ �����ϱ�
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Practice 10 - Complete Triangle System");
	//--- GLEW �ʱ�ȭ�ϱ�
	glewExperimental = GL_TRUE;
	glewInit();
	shaderProgramID = make_shaderProgram();
	InitBuffer();
	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
	glutMouseFunc(Mouse);
	InitShapes();
	glutKeyboardFunc(Keyboard); //--- Ű���� �Է� �ݹ��Լ� ����
	glutMainLoop(); // �̺�Ʈ ó�� ����
}

//--- ���̴� ���α׷� ����� ���̴� ��ü ��ũ�ϱ�
GLuint make_shaderProgram()
{
	// ���� ���̴����� ������
	make_vertexShaders();
	make_fragmentShaders();

	GLint result;
	GLchar errorLog[512];
	GLuint shaderID;

	shaderID = glCreateProgram(); //--- ���̴� ���α׷� �����
	glAttachShader(shaderID, vertexShader); //--- ���̴� ���α׷��� ���ؽ� ���̴� ���̱�
	glAttachShader(shaderID, fragmentShader); //--- ���̴� ���α׷��� �����׸�Ʈ ���̴� ���̱�
	glLinkProgram(shaderID); //--- ���̴� ���α׷� ��ũ�ϱ�

	glDeleteShader(vertexShader); //--- ���̴� ��ü�� ���̴� ���α׷��� ��ũ��������, ���̴� ��ü ��ü�� ���� ����
	glDeleteShader(fragmentShader);

	glGetProgramiv(shaderID, GL_LINK_STATUS, &result); // ---���̴��� �� ����Ǿ����� üũ�ϱ�
	if (!result) {
		glGetProgramInfoLog(shaderID, 512, NULL, errorLog);
		std::cerr << "ERROR: shader program ���� ����\n" << errorLog << std::endl;
		return 0;
	}

	return shaderID;
}