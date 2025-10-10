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
#include <chrono>
//--- �Ʒ� 5�� �Լ��� ����� ���� �Լ���
void make_vertexShaders();
void make_fragmentShaders();
GLuint make_shaderProgram();
GLvoid drawScene();
GLvoid Reshape(int w, int h);
GLvoid Timer(int value);
//--- �ʿ��� ���� ����
GLint width, height;
GLuint shaderProgramID; //--- ���̴� ���α׷� �̸�
GLuint vertexShader; //--- ���ؽ� ���̴� ��ü
GLuint fragmentShader; //--- �����׸�Ʈ ���̴� ��ü

std::random_device rd;  // �õ尪�� ��� ���� random_device ����.
std::mt19937 gen(rd());	// random_device �� ���� ���� ���� ������ �ʱ�ȭ �Ѵ�.
std::uniform_real_distribution<float> dis_color(0.0f, 1.0f); // 0.0f ���� 1.0f ���� �յ��ϰ� ��Ÿ���� �������� �����ϱ� ���� �յ� ���� ����.
std::uniform_real_distribution<float> dis_x(0.1f, 0.9f); // -0.1f ���� 0.1f ���� �յ��ϰ� ��Ÿ���� �������� �����ϱ� ���� �յ� ���� ����.
std::uniform_real_distribution<float> dis_y(0.1f, 0.7f); // -0.1f ���� 0.1f ���� �յ��ϰ� ��Ÿ���� �������� �����ϱ� ���� �յ� ���� ����.
std::uniform_real_distribution<float> dis_size(0.05f, 0.15f); // �ﰢ�� ũ��� ���� ����

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

int command = 0;

struct Shape {
	GLenum drawMode;
	int startIndex;
	int vertexCount;
	float centerX, centerY;
	int shapeType;	// 1 : ��, 2 : ��, 3 : �ﰢ��, 4 : �簢��, 5 : ������
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

void InitShapes() {	// x,y ���ϰ� �� ��и鸶�� �ﰢ�� �ϳ���
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
	LineX.shapeType = 0; // ��

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
	LineY.shapeType = 0; // ��

	shapes.push_back(LineY);
	UpdateBuffer();

	// ���ϴ� �ִϸ��̼� �־�ߵǴϱ� �� �����鿡 ���ؽ� 9���� �Ҵ��ϱ� (�������� 9�� �ʿ��ϱ� ����)
	// 1��и� - �ﰢ��
	Shape newShape1;
	newShape1.startIndex = allVertices.size() / 3;
	newShape1.centerX = 0.5f;
	newShape1.centerY = 0.5f;
	float x = newShape1.centerX;
	float y = newShape1.centerY;

	allVertices.push_back(x);
	allVertices.push_back(y + 0.3f);
	allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allVertices.push_back(x - 0.15f);
	allVertices.push_back(y - 0.15f);
	allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allVertices.push_back(x + 0.15f);
	allVertices.push_back(y - 0.15f);
	allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));

	newShape1.drawMode = GL_TRIANGLES;
	newShape1.vertexCount = 3;
	newShape1.shapeType = 3; // �ﰢ�� Ÿ��

	shapes.push_back(newShape1);

	// 2��и� - ��
	Shape newShape2;
	newShape2.startIndex = allVertices.size() / 3;
	newShape2.centerX = -0.5f;
	newShape2.centerY = 0.5f;
	x = newShape2.centerX;
	y = newShape2.centerY;

	allVertices.push_back(x + 0.25f);
	allVertices.push_back(y + 0.25f);
	allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allVertices.push_back(x - 0.25f);
	allVertices.push_back(y - 0.25f);
	allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));

	newShape2.drawMode = GL_LINES;
	newShape2.vertexCount = 2;
	newShape2.shapeType = 2;

	shapes.push_back(newShape2);

	// 3��и� - �簢��
	Shape newShape3;
	newShape3.startIndex = allVertices.size() / 3;
	newShape3.centerX = -0.5f;
	newShape3.centerY = -0.5f;
	x = newShape3.centerX;
	y = newShape3.centerY;

	allVertices.push_back(x + 0.25f);
	allVertices.push_back(y + 0.25f);
	allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allVertices.push_back(x - 0.25f);
	allVertices.push_back(y - 0.25f);
	allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allVertices.push_back(x - 0.25f);
	allVertices.push_back(y + 0.25f);
	allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));

	allVertices.push_back(x + 0.25f);
	allVertices.push_back(y + 0.25f);
	allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allVertices.push_back(x - 0.25f);
	allVertices.push_back(y - 0.25f);
	allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allVertices.push_back(x + 0.25f);
	allVertices.push_back(y - 0.25f);
	allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));

	newShape3.drawMode = GL_TRIANGLES;
	newShape3.vertexCount = 6;
	newShape3.shapeType = 4;
	shapes.push_back(newShape3);

	// 4��и� - ������ (�ﰢ�� 3��)
	Shape newShape4;
	newShape4.startIndex = allVertices.size() / 3;
	newShape4.centerX = 0.5f;
	newShape4.centerY = -0.5f;
	x = newShape4.centerX;
	y = newShape4.centerY;

	// �߾� �ﰢ��
	allVertices.push_back(x);
	allVertices.push_back(y + 0.3f);
	allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allVertices.push_back(x - 0.15f);
	allVertices.push_back(y - 0.3f);
	allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allVertices.push_back(x + 0.15f);
	allVertices.push_back(y - 0.3f);
	allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));

	// ���� �ﰢ��
	allVertices.push_back(x - 0.3f);
	allVertices.push_back(y + 0.05f);
	allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allVertices.push_back(x);
	allVertices.push_back(y + 0.3f);
	allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allVertices.push_back(x - 0.15f);
	allVertices.push_back(y - 0.3f);
	allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));

	// ������ �ﰢ��
	allVertices.push_back(x + 0.3f);
	allVertices.push_back(y + 0.05f);
	allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allVertices.push_back(x);
	allVertices.push_back(y + 0.3f);
	allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allVertices.push_back(x + 0.15f);
	allVertices.push_back(y - 0.3f);
	allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));

	newShape4.drawMode = GL_TRIANGLES;
	newShape4.vertexCount = 9;
	newShape4.shapeType = 5; // �ﰢ�� Ÿ��

	shapes.push_back(newShape4);
	UpdateBuffer();
}

void changeShapes() {
	for (int i = shapes.size() - 1; i >= 0; --i) {
		// �� -> �ﰢ��
		if (command == 2 && shapes[i].shapeType == 2) {
			Shape newShape1;
			newShape1.startIndex = allVertices.size() / 3;
			newShape1.centerX = shapes[i].centerX;
			newShape1.centerY = shapes[i].centerY;
			float x = newShape1.centerX;
			float y = newShape1.centerY;

			allVertices.push_back(x);
			allVertices.push_back(y + 0.3f);
			allVertices.push_back(0.0f);
			allColors.push_back(dis_color(gen));
			allColors.push_back(dis_color(gen));
			allColors.push_back(dis_color(gen));
			allVertices.push_back(x - 0.15f);
			allVertices.push_back(y - 0.15f);
			allVertices.push_back(0.0f);
			allColors.push_back(dis_color(gen));
			allColors.push_back(dis_color(gen));
			allColors.push_back(dis_color(gen));
			allVertices.push_back(x + 0.15f);
			allVertices.push_back(y - 0.15f);
			allVertices.push_back(0.0f);
			allColors.push_back(dis_color(gen));
			allColors.push_back(dis_color(gen));
			allColors.push_back(dis_color(gen));
			newShape1.drawMode = GL_TRIANGLES;
			newShape1.vertexCount = 3;
			newShape1.shapeType = 3; // �ﰢ�� Ÿ��

			shapes.push_back(newShape1);
			shapes.erase(shapes.begin() + i);
		}
		// �ﰢ�� -> �簢��
		else if (command == 3 && shapes[i].shapeType == 3) {
			Shape newShape3;
			newShape3.startIndex = allVertices.size() / 3;
			newShape3.centerX = shapes[i].centerX;
			newShape3.centerY = shapes[i].centerY;
			float x = newShape3.centerX;
			float y = newShape3.centerY;

			allVertices.push_back(x + 0.25f);
			allVertices.push_back(y + 0.25f);
			allVertices.push_back(0.0f);
			allColors.push_back(dis_color(gen));
			allColors.push_back(dis_color(gen));
			allColors.push_back(dis_color(gen));
			allVertices.push_back(x - 0.25f);
			allVertices.push_back(y - 0.25f);
			allVertices.push_back(0.0f);
			allColors.push_back(dis_color(gen));
			allColors.push_back(dis_color(gen));
			allColors.push_back(dis_color(gen));
			allVertices.push_back(x - 0.25f);
			allVertices.push_back(y + 0.25f);
			allVertices.push_back(0.0f);
			allColors.push_back(dis_color(gen));
			allColors.push_back(dis_color(gen));
			allColors.push_back(dis_color(gen));

			allVertices.push_back(x + 0.25f);
			allVertices.push_back(y + 0.25f);
			allVertices.push_back(0.0f);
			allColors.push_back(dis_color(gen));
			allColors.push_back(dis_color(gen));
			allColors.push_back(dis_color(gen));
			allVertices.push_back(x - 0.25f);
			allVertices.push_back(y - 0.25f);
			allVertices.push_back(0.0f);
			allColors.push_back(dis_color(gen));
			allColors.push_back(dis_color(gen));
			allColors.push_back(dis_color(gen));
			allVertices.push_back(x + 0.25f);
			allVertices.push_back(y - 0.25f);
			allVertices.push_back(0.0f);
			allColors.push_back(dis_color(gen));
			allColors.push_back(dis_color(gen));
			allColors.push_back(dis_color(gen));

			newShape3.drawMode = GL_TRIANGLES;
			newShape3.vertexCount = 6;
			newShape3.shapeType = 4;
			shapes.push_back(newShape3);
			shapes.erase(shapes.begin() + i);
		}
		// �簢�� -> ������
		else if (command == 4 && shapes[i].shapeType == 4) {
			Shape newShape4;
			newShape4.startIndex = allVertices.size() / 3;
			newShape4.centerX = shapes[i].centerX;
			newShape4.centerY = shapes[i].centerY;
			float x = newShape4.centerX;
			float y = newShape4.centerY;

			// �߾� �ﰢ��
			allVertices.push_back(x);
			allVertices.push_back(y + 0.3f);
			allVertices.push_back(0.0f);
			allColors.push_back(dis_color(gen));
			allColors.push_back(dis_color(gen));
			allColors.push_back(dis_color(gen));
			allVertices.push_back(x - 0.15f);
			allVertices.push_back(y - 0.3f);
			allVertices.push_back(0.0f);
			allColors.push_back(dis_color(gen));
			allColors.push_back(dis_color(gen));
			allColors.push_back(dis_color(gen));
			allVertices.push_back(x + 0.15f);
			allVertices.push_back(y - 0.3f);
			allVertices.push_back(0.0f);
			allColors.push_back(dis_color(gen));
			allColors.push_back(dis_color(gen));
			allColors.push_back(dis_color(gen));

			// ���� �ﰢ��
			allVertices.push_back(x - 0.3f);
			allVertices.push_back(y + 0.05f);
			allVertices.push_back(0.0f);
			allColors.push_back(dis_color(gen));
			allColors.push_back(dis_color(gen));
			allColors.push_back(dis_color(gen));
			allVertices.push_back(x);
			allVertices.push_back(y + 0.3f);
			allVertices.push_back(0.0f);
			allColors.push_back(dis_color(gen));
			allColors.push_back(dis_color(gen));
			allColors.push_back(dis_color(gen));
			allVertices.push_back(x - 0.15f);
			allVertices.push_back(y - 0.3f);
			allVertices.push_back(0.0f);
			allColors.push_back(dis_color(gen));
			allColors.push_back(dis_color(gen));
			allColors.push_back(dis_color(gen));

			// ������ �ﰢ��
			allVertices.push_back(x + 0.3f);
			allVertices.push_back(y + 0.05f);
			allVertices.push_back(0.0f);
			allColors.push_back(dis_color(gen));
			allColors.push_back(dis_color(gen));
			allColors.push_back(dis_color(gen));
			allVertices.push_back(x);
			allVertices.push_back(y + 0.3f);
			allVertices.push_back(0.0f);
			allColors.push_back(dis_color(gen));
			allColors.push_back(dis_color(gen));
			allColors.push_back(dis_color(gen));
			allVertices.push_back(x + 0.15f);
			allVertices.push_back(y - 0.3f);
			allVertices.push_back(0.0f);
			allColors.push_back(dis_color(gen));
			allColors.push_back(dis_color(gen));
			allColors.push_back(dis_color(gen));

			newShape4.drawMode = GL_TRIANGLES;
			newShape4.vertexCount = 9;
			newShape4.shapeType = 5; // �ﰢ�� Ÿ��

			shapes.push_back(newShape4);
			shapes.erase(shapes.begin() + i);
		}
		// ������ -> ��
		else if (command == 5 && shapes[i].shapeType == 5) {
			Shape newShape2;
			newShape2.startIndex = allVertices.size() / 3;
			newShape2.centerX = shapes[i].centerX;
			newShape2.centerY = shapes[i].centerY;
			float x = newShape2.centerX;
			float y = newShape2.centerY;

			allVertices.push_back(x + 0.25f);
			allVertices.push_back(y + 0.25f);
			allVertices.push_back(0.0f);
			allColors.push_back(dis_color(gen));
			allColors.push_back(dis_color(gen));
			allColors.push_back(dis_color(gen));
			allVertices.push_back(x - 0.25f);
			allVertices.push_back(y - 0.25f);
			allVertices.push_back(0.0f);
			allColors.push_back(dis_color(gen));
			allColors.push_back(dis_color(gen));
			allColors.push_back(dis_color(gen));
			newShape2.drawMode = GL_LINES;
			newShape2.vertexCount = 2;
			newShape2.shapeType = 2;

			shapes.push_back(newShape2);
			shapes.erase(shapes.begin() + i);
		}
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
		allVertices.clear();
		allColors.clear();
		shapes.clear();
		InitShapes();
		break;
	case 'q':
		exit(0);
		break;
	}
	glutPostRedisplay(); //--- ������ �ٲ� ������ ��� �ݹ� �Լ��� ȣ���Ͽ� ȭ���� refresh �Ѵ�
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
	glutCreateWindow("Practice 12");
	//--- GLEW �ʱ�ȭ�ϱ�
	glewExperimental = GL_TRUE;
	glewInit();
	shaderProgramID = make_shaderProgram();
	InitBuffer();
	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
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