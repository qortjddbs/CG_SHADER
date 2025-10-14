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
void UpdateBuffer();
GLuint make_shaderProgram();
GLvoid drawScene();
GLvoid Reshape(int w, int h);
GLvoid Mouse(int button, int state, int x, int y);
GLvoid Timer(int value);
//--- �ʿ��� ���� ����
GLint width, height;
GLuint shaderProgramID; //--- ���̴� ���α׷� �̸�
GLuint vertexShader; //--- ���ؽ� ���̴� ��ü
GLuint fragmentShader; //--- �����׸�Ʈ ���̴� ��ü

std::random_device rd;  // �õ尪�� ��� ���� random_device ����.
std::mt19937 gen(rd());	// random_device �� ���� ���� ���� ������ �ʱ�ȭ �Ѵ�.
std::uniform_real_distribution<float> dis_color(0.0f, 1.0f); // 0.0f ���� 1.0f ���� �յ��ϰ� ��Ÿ���� �������� �����ϱ� ���� �յ� ���� ����.
std::uniform_real_distribution<float> dis_x(-0.7f, 0.7f); // -0.1f ���� 0.1f ���� �յ��ϰ� ��Ÿ���� �������� �����ϱ� ���� �յ� ���� ����.
std::uniform_real_distribution<float> dis_y(-0.7f, 0.7f); // -0.1f ���� 0.1f ���� �յ��ϰ� ��Ÿ���� �������� �����ϱ� ���� �յ� ���� ����.
std::uniform_real_distribution<float> dis_size(0.05f, 0.15f); // �ﰢ�� ũ��� ���� ����
std::uniform_int_distribution<int> dis_motion(1, 2); // 1: �밢��, 2: �������

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
	int shapeType;	// 1 : ��, 2 : ��, 3 : �ﰢ��, 4 : �簢��, 5 : ������
	GLfloat r, g, b;	// ���� ���� �߰�

	// �ִϸ��̼� ���� ������
	bool isAnimated;
	int motionType;  // 1: �밢��, 2: �������
	float dirX, dirY;
	float zigzagStepY; // ������׿� Y�� �̵���
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

// �� ������ ��ġ���� Ȯ���ϴ� �Լ�
bool AreShapesOverlapping(const Shape& shape1, const Shape& shape2) {
	float dx = shape1.centerX - shape2.centerX;
	float dy = shape1.centerY - shape2.centerY;
	float distance = sqrt(dx * dx + dy * dy);

	// �� ������ ũ�⸦ ����� �浹 �Ÿ� ���
	float collisionDistance = (shape1.size + shape2.size) * 1.5f;

	return distance < collisionDistance;
}

// ���ο� ������ �����ϴ� �Լ�
// ���ο� ������ �����ϴ� �Լ� (���� ���� ���)
void CreateShape(int shapeType, float centerX, float centerY, float size, bool animated = false) {
	Shape newShape;
	newShape.startIndex = allVertices.size() / 3;
	newShape.centerX = centerX;
	newShape.centerY = centerY;
	newShape.size = size;
	newShape.shapeType = shapeType;
	newShape.isAnimated = animated;

	// �ִϸ��̼� ����
	if (animated) {
		newShape.motionType = dis_motion(gen);
		if (newShape.motionType == 1) { // �밢��
			newShape.dirX = 0.02f;
			newShape.dirY = 0.02f;
		}
		else { // �������
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

	// ���� Ÿ�Ժ� ���� ���� ����
	switch (shapeType) {
	case 1: newShape.r = 1.0f; newShape.g = 0.0f; newShape.b = 0.0f; break; // ���� (��)
	case 2: newShape.r = 0.0f; newShape.g = 1.0f; newShape.b = 0.0f; break; // �ʷ� (��)
	case 3: newShape.r = 0.0f; newShape.g = 0.0f; newShape.b = 1.0f; break; // �Ķ� (�ﰢ��)
	case 4: newShape.r = 1.0f; newShape.g = 1.0f; newShape.b = 0.0f; break; // ��� (�簢��)
	case 5: newShape.r = 1.0f; newShape.g = 0.0f; newShape.b = 1.0f; break; // ���� (������)
	}

	float x = centerX;
	float y = centerY;

	switch (shapeType) {
	case 1: // �� (���� �ﰢ��)
		allVertices.push_back(x); allVertices.push_back(y + 0.02f); allVertices.push_back(0.0f);
		allColors.push_back(newShape.r); allColors.push_back(newShape.g); allColors.push_back(newShape.b);

		allVertices.push_back(x - 0.01f); allVertices.push_back(y - 0.01f); allVertices.push_back(0.0f);
		allColors.push_back(newShape.r); allColors.push_back(newShape.g); allColors.push_back(newShape.b);

		allVertices.push_back(x + 0.01f); allVertices.push_back(y - 0.01f); allVertices.push_back(0.0f);
		allColors.push_back(newShape.r); allColors.push_back(newShape.g); allColors.push_back(newShape.b);

		newShape.drawMode = GL_TRIANGLES;
		newShape.vertexCount = 3;
		break;

	case 2: // ��
		allVertices.push_back(x - size); allVertices.push_back(y); allVertices.push_back(0.0f);
		allColors.push_back(newShape.r); allColors.push_back(newShape.g); allColors.push_back(newShape.b);
		allVertices.push_back(x + size); allVertices.push_back(y); allVertices.push_back(0.0f);
		allColors.push_back(newShape.r); allColors.push_back(newShape.g); allColors.push_back(newShape.b);

		newShape.drawMode = GL_LINES;
		newShape.vertexCount = 2;
		break;

	case 3: // �ﰢ��
		allVertices.push_back(x); allVertices.push_back(y + size * 2); allVertices.push_back(0.0f);
		allColors.push_back(newShape.r); allColors.push_back(newShape.g); allColors.push_back(newShape.b);

		allVertices.push_back(x - size); allVertices.push_back(y - size); allVertices.push_back(0.0f);
		allColors.push_back(newShape.r); allColors.push_back(newShape.g); allColors.push_back(newShape.b);

		allVertices.push_back(x + size); allVertices.push_back(y - size); allVertices.push_back(0.0f);
		allColors.push_back(newShape.r); allColors.push_back(newShape.g); allColors.push_back(newShape.b);

		newShape.drawMode = GL_TRIANGLES;
		newShape.vertexCount = 3;
		break;

	case 4: // �簢��
		// �»��
		allVertices.push_back(x - size); allVertices.push_back(y + size); allVertices.push_back(0.0f);
		allColors.push_back(newShape.r); allColors.push_back(newShape.g); allColors.push_back(newShape.b);

		// ���ϴ�
		allVertices.push_back(x - size); allVertices.push_back(y - size); allVertices.push_back(0.0f);
		allColors.push_back(newShape.r); allColors.push_back(newShape.g); allColors.push_back(newShape.b);

		// ���ϴ�
		allVertices.push_back(x + size); allVertices.push_back(y - size); allVertices.push_back(0.0f);
		allColors.push_back(newShape.r); allColors.push_back(newShape.g); allColors.push_back(newShape.b);

		// �»��
		allVertices.push_back(x - size); allVertices.push_back(y + size); allVertices.push_back(0.0f);
		allColors.push_back(newShape.r); allColors.push_back(newShape.g); allColors.push_back(newShape.b);

		// ���ϴ�
		allVertices.push_back(x + size); allVertices.push_back(y - size); allVertices.push_back(0.0f);
		allColors.push_back(newShape.r); allColors.push_back(newShape.g); allColors.push_back(newShape.b);

		// ����
		allVertices.push_back(x + size); allVertices.push_back(y + size); allVertices.push_back(0.0f);
		allColors.push_back(newShape.r); allColors.push_back(newShape.g); allColors.push_back(newShape.b);

		newShape.drawMode = GL_TRIANGLES;
		newShape.vertexCount = 6;
		break;

	case 5: // ������
		// �߾� �ﰢ��
		allVertices.push_back(x); allVertices.push_back(y + size * 2); allVertices.push_back(0.0f);
		allColors.push_back(newShape.r); allColors.push_back(newShape.g); allColors.push_back(newShape.b);

		allVertices.push_back(x - size); allVertices.push_back(y - size * 2); allVertices.push_back(0.0f);
		allColors.push_back(newShape.r); allColors.push_back(newShape.g); allColors.push_back(newShape.b);

		allVertices.push_back(x + size); allVertices.push_back(y - size * 2); allVertices.push_back(0.0f);
		allColors.push_back(newShape.r); allColors.push_back(newShape.g); allColors.push_back(newShape.b);

		// ���� �ﰢ��
		allVertices.push_back(x - size * 2); allVertices.push_back(y); allVertices.push_back(0.0f);
		allColors.push_back(newShape.r); allColors.push_back(newShape.g); allColors.push_back(newShape.b);

		allVertices.push_back(x); allVertices.push_back(y + size * 2); allVertices.push_back(0.0f);
		allColors.push_back(newShape.r); allColors.push_back(newShape.g); allColors.push_back(newShape.b);

		allVertices.push_back(x - size); allVertices.push_back(y - size * 2); allVertices.push_back(0.0f);
		allColors.push_back(newShape.r); allColors.push_back(newShape.g); allColors.push_back(newShape.b);

		// ������ �ﰢ��
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

// �� ������ ��ġ�� �Լ�
void MergeShapes(int shapeIndex1, int shapeIndex2) {
	const Shape& shape1 = shapes[shapeIndex1];
	const Shape& shape2 = shapes[shapeIndex2];

	// ������ ���� �հ� ���
	int totalVertices = shape1.shapeType + shape2.shapeType;
	int newShapeType;

	// ������ ������ ���� ���� ���ο� ���� Ÿ�� ����
	if (totalVertices >= 6) {
		newShapeType = 1; // ��
	}
	else {
		newShapeType = totalVertices;
	}

	// �� ������ ��ġ�� �� ������ ����
	float newCenterX = (shape1.centerX + shape2.centerX) / 2.0f;
	float newCenterY = (shape1.centerY + shape2.centerY) / 2.0f;
	float newSize = (shape1.size + shape2.size) / 2.0f;

	// ���� �������� shapes ���Ϳ��� ���� (�ε����� ū �ͺ��� ����)
	int firstToRemove = std::max(shapeIndex1, shapeIndex2);
	int secondToRemove = std::min(shapeIndex1, shapeIndex2);

	shapes.erase(shapes.begin() + firstToRemove);
	shapes.erase(shapes.begin() + secondToRemove);

	// ��ü ���ؽ��� �÷� �迭�� �ٽ� ����
	allVertices.clear();
	allColors.clear();

	// ���� �������� �ٽ� �߰�
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

	// ���ο� ������ ���� �߰� (�ִϸ��̼� Ȱ��ȭ)
	CreateShape(newShapeType, newCenterX, newCenterY, newSize, true);

	// �ִϸ��̼��� Ȱ��ȭ�Ǿ� �ִٸ� Ÿ�̸� ����
	if (animationEnabled && !timerRunning) {
		timerRunning = true;
		glutTimerFunc(16, Timer, 1);
	}

	UpdateBuffer();
}

bool IsMouseInShape(float x, float y, const Shape& shape) {
	float dx = x - shape.centerX;
	float dy = y - shape.centerY;

	// �������� ����
	if (dx < 0) dx = -dx;
	if (dy < 0) dy = -dy;

	switch (shape.shapeType) {
	case 1: // ��
		return (dx <= 0.02f && dy <= 0.02f);	// �簢�� ����
	case 2: // ��
		return (dy <= 0.01f && dx <= shape.size * 2); // �� �߽� �ֺ� ����
	case 3: { // �ﰢ��
		// �ﰢ���� �� ������ (AddShape���� ���ǵ� ��ǥ ����)
		float x1 = shape.centerX;           // ���� ������
		float y1 = shape.centerY + shape.size * 2;
		float x2 = shape.centerX - shape.size;   // ���� �Ʒ� ������
		float y2 = shape.centerY - shape.size;
		float x3 = shape.centerX + shape.size;   // ������ �Ʒ� ������
		float y3 = shape.centerY - shape.size;

		// �����߽� ��ǥ�� �̿��� �ﰢ�� ���� �Ǻ�
		float denominator = (y2 - y3) * (x1 - x3) + (x3 - x2) * (y1 - y3);
		if (abs(denominator) < 0.0001f) return false; // �ﰢ���� �ƴ� ���

		float a = ((y2 - y3) * (x - x3) + (x3 - x2) * (y - y3)) / denominator;
		float b = ((y3 - y1) * (x - x3) + (x1 - x3) * (y - y3)) / denominator;
		float c = 1 - a - b;

		return (a >= 0 && b >= 0 && c >= 0);
	}
	case 4: // �簢��
		return abs(dx) <= shape.size && abs(dy) <= shape.size; // �簢�� ����
	case 5: {	// ������
		// �������� �����ϴ� 3�� �ﰢ���� ���� ���� ���� �Ǻ�

		// �߾� �ﰢ�� �Ǻ�
		float x1 = shape.centerX;
		float y1 = shape.centerY + shape.size * 2;
		float x2 = shape.centerX - shape.size;
		float y2 = shape.centerY - shape.size * 2;
		float x3 = shape.centerX + shape.size;
		float y3 = shape.centerY - shape.size * 2;

		// �����߽� ��ǥ�� �߾� �ﰢ�� ���� �Ǻ�
		float denom1 = (y2 - y3) * (x1 - x3) + (x3 - x2) * (y1 - y3);
		if (abs(denom1) > 0.0001f) {
			float a1 = ((y2 - y3) * (x - x3) + (x3 - x2) * (y - y3)) / denom1;
			float b1 = ((y3 - y1) * (x - x3) + (x1 - x3) * (y - y3)) / denom1;
			float c1 = 1 - a1 - b1;
			if (a1 >= 0 && b1 >= 0 && c1 >= 0) return true;
		}

		// ���� �ﰢ�� �Ǻ�
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

		// ������ �ﰢ�� �Ǻ�
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
	}  // switch�� �ݴ� �߰�ȣ �߰�

	return false;  // �⺻ ��ȯ�� �߰�
}  // �Լ� �ݴ� �߰�ȣ �߰�

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
	// ������
	for (int i = 0; i < 3; ++i) {
		float x = dis_x(gen);
		float y = dis_y(gen);
		float size = dis_size(gen);
		CreateShape(5, x, y, size);
	}

	// �簢��
	for (int i = 0; i < 3; ++i) {
		float x = dis_x(gen);
		float y = dis_y(gen);
		float size = dis_size(gen);
		CreateShape(4, x, y, size);
	}

	// �ﰢ��
	for (int i = 0; i < 3; ++i) {
		float x = dis_x(gen);
		float y = dis_y(gen);
		float size = dis_size(gen);
		CreateShape(3, x, y, size);
	}

	// ��
	for (int i = 0; i < 3; ++i) {
		float x = dis_x(gen);
		float y = dis_y(gen);
		float size = dis_size(gen);
		CreateShape(2, x, y, size);
	}

	// ��
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

	if (shape.motionType == 1) { // �밢�� �̵�
		// ��� üũ �� ���� ����
		if (shape.centerX + shape.size >= 1.0f || shape.centerX - shape.size <= -1.0f) {
			shape.dirX *= -1;
		}
		if (shape.centerY + shape.size * 2 >= 1.0f || shape.centerY - shape.size * 2 <= -1.0f) {
			shape.dirY *= -1;
		}

		deltaX = shape.dirX;
		deltaY = shape.dirY;
	}
	else if (shape.motionType == 2) { // ������� �̵�
		// �¿� ��� üũ �� ���� ���� + Y�� �̵�
		if (shape.centerX + shape.size >= 1.0f || shape.centerX - shape.size <= -1.0f) {
			shape.dirX *= -1;
			deltaY = shape.zigzagStepY;
		}

		// Y�� ��� üũ
		if (shape.centerY + shape.size * 2 >= 1.0f || shape.centerY - shape.size * 2 <= -1.0f) {
			shape.zigzagStepY *= -1;
		}

		deltaX = shape.dirX;
	}

	// ������ �̵�
	int startIdx = shape.startIndex * 3;
	int endIdx = startIdx + shape.vertexCount * 3;

	for (int i = startIdx; i < endIdx; i += 3) {
		allVertices[i] += deltaX;     // x ��ǥ
		allVertices[i + 1] += deltaY; // y ��ǥ
	}

	// �߽��� ������Ʈ
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

//--- �׸��� �ݹ� �Լ�
GLvoid drawScene()
{
	//--- ����� ���� ����
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//--- ������ ���������ο� ���̴� �ҷ�����

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
			// �ִϸ��̼� Ȱ��ȭ�� ������ �ִ��� Ȯ��
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
	glutPostRedisplay(); //--- ������ �ٲ� ������ ��� �ݹ� �Լ��� ȣ���Ͽ� ȭ���� refresh �Ѵ�
}

GLvoid Timer(int value) {
	if (!animationEnabled) {
		timerRunning = false;
		return;
	}

	bool hasAnimatedShape = false;

	// �ִϸ��̼��� Ȱ��ȭ�� ��� ������ ������Ʈ
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
	glutTimerFunc(16, Timer, 1); // ���� �������� ���� Ÿ�̸� ����
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
			// �巡�װ� ������ �� �ٸ� ������ ��ġ���� Ȯ��
			for (int i = 0; i < shapes.size(); ++i) {
				if (i != selected && AreShapesOverlapping(shapes[selected], shapes[i])) {
					// ��ġ�� ���� �߰� �� ��ġ��
					MergeShapes(selected, i);
					break;
				}
			}
		}
		isDragging = false;
		selected = -1;
	}

	glutPostRedisplay(); //--- ������ �ٲ� ������ ��� �ݹ� �Լ��� ȣ���Ͽ� ȭ���� refresh �Ѵ�
}

GLvoid Motion(int x, int y)
{
	if (isDragging && selected != -1) {
		float Mouse_x = mapToGLCoordX(x);
		float Mouse_y = mapToGLCoordY(y);

		float deltaX = Mouse_x - lastMouseX;
		float deltaY = Mouse_y - lastMouseY;

		// ���õ� ������ �߽��� �̵�
		shapes[selected].centerX += deltaX;
		shapes[selected].centerY += deltaY;

		// �ش� ������ ��� �������� �̵�
		int startIdx = shapes[selected].startIndex * 3;
		int endIdx = startIdx + shapes[selected].vertexCount * 3;

		for (int i = startIdx; i < endIdx; i += 3) {
			allVertices[i] += deltaX;     // x ��ǥ
			allVertices[i + 1] += deltaY; // y ��ǥ
		}

		UpdateBuffer();

		lastMouseX = Mouse_x;
		lastMouseY = Mouse_y;

		glutPostRedisplay();
	}
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
	glutCreateWindow("Practice 13");
	//--- GLEW �ʱ�ȭ�ϱ�
	glewExperimental = GL_TRUE;
	glewInit();
	shaderProgramID = make_shaderProgram();
	InitBuffer();
	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
	InitShapes();
	// glutTimerFunc(16, Timer, 1); // Ÿ�̸� ����
	glutMouseFunc(Mouse);
	glutMotionFunc(Motion);
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