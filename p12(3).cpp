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

// �ִϸ��̼� ���� ����
bool isAnimating = false;
// int animationStep = 0;

struct Shape {
	GLenum drawMode;
	int startIndex;
	int vertexCount;
	float centerX, centerY;
	int shapeType;	// 1 : ��, 2 : ��, 3 : �ﰢ��, 4 : �簢��, 5 : ������
	bool isTransforming;
	int quadrant;	// ������ ��� ��и鿡 �ִ���
	int targetShapeType; // ��ȯ�� ��ǥ ���� Ÿ��
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
	LineY.shapeType = 0; // ��
	LineY.quadrant = 0;
	LineY.targetShapeType = 0;

	shapes.push_back(LineY);
	
	// 1��и� - �ﰢ�� (0.5, 0.5) - 5�� ������
	Shape shape1;
	shape1.startIndex = allVertices.size() / 3;
	shape1.centerX = 0.5f; shape1.centerY = 0.5f;
	// �ﰢ��: ������ 3�� + ���� �� ���� + ������ �� ����
	// ��� ������
	allVertices.push_back(0.5f); allVertices.push_back(0.8f); allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));
	// ���� ������
	allVertices.push_back(0.35f); allVertices.push_back(0.35f); allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));
	// ���� ������
	allVertices.push_back(0.65f); allVertices.push_back(0.35f); allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));
	// ���� �� ����
	allVertices.push_back(0.425f); allVertices.push_back(0.575f); allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));
	// ������ �� ����
	allVertices.push_back(0.575f); allVertices.push_back(0.575f); allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));
	
	shape1.drawMode = GL_LINE_LOOP; shape1.vertexCount = 3;  // �ﰢ���� ù 3�� ���� ���
	shape1.shapeType = 3; shape1.quadrant = 1;
	shape1.isTransforming = false;
	shape1.targetShapeType = 3;
	shapes.push_back(shape1);

	// 2��и� - �� (-0.5, 0.5) - 5�� ���� ����������
	Shape shape2;
	shape2.startIndex = allVertices.size() / 3;
	shape2.centerX = -0.5f; shape2.centerY = 0.5f;
	// ��: 5�� ���� ������ �������� ����������
	float lineStartX = -0.75f, lineStartY = 0.25f;
	float lineEndX = -0.25f, lineEndY = 0.75f;
	for (int i = 0; i < 5; ++i) {
		float t = i / 4.0f; // 0, 0.25, 0.5, 0.75, 1.0
		float x = lineStartX + t * (lineEndX - lineStartX);
		float y = lineStartY + t * (lineEndY - lineStartY);
		allVertices.push_back(x); allVertices.push_back(y); allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));
	}
	
	shape2.drawMode = GL_LINE_STRIP; shape2.vertexCount = 5;  // 5�� ���� ������ ����
	shape2.shapeType = 2; shape2.quadrant = 2;
	shape2.isTransforming = false;
	shape2.targetShapeType = 2;
	shapes.push_back(shape2);

	// 3��и� - �簢�� (-0.5, -0.5) - 4�� ������ + �߽�
	Shape shape3;
	shape3.startIndex = allVertices.size() / 3;
	shape3.centerX = -0.5f; shape3.centerY = -0.5f;
	// �簢��: 4�� ������ (�ð�������� ��ġ)
	// ���� ������
	allVertices.push_back(-0.75f); allVertices.push_back(-0.75f); allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));
	// ���� ������
	allVertices.push_back(-0.25f); allVertices.push_back(-0.75f); allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));
	// ��� ������
	allVertices.push_back(-0.25f); allVertices.push_back(-0.25f); allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));
	// �»� ������
	allVertices.push_back(-0.75f); allVertices.push_back(-0.25f); allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));
	// �߽��� (����, ������� ����)
	allVertices.push_back(-0.5f); allVertices.push_back(-0.5f); allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));
	
	shape3.drawMode = GL_LINE_LOOP; shape3.vertexCount = 4;  // �簢���� ù 4�� ���� ���
	shape3.shapeType = 4; shape3.quadrant = 3;
	shape3.isTransforming = false;
	shape3.targetShapeType = 4;
	shapes.push_back(shape3);

	// 4��и� - ������ (0.5, -0.5) - 5�� ������
	Shape shape4;
	shape4.startIndex = allVertices.size() / 3;
	shape4.centerX = 0.5f; shape4.centerY = -0.5f;
	// ������: 5�� ������ (��������)
	float radius = 0.3f;
	for (int i = 0; i < 5; ++i) {
		float angle = (i * 2.0f * 3.14159f / 5.0f) - (3.14159f / 2.0f); // -90������ ����
		float x = shape4.centerX + radius * cos(angle);
		float y = shape4.centerY + radius * sin(angle);
		allVertices.push_back(x); allVertices.push_back(y); allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));
	}
	
	shape4.drawMode = GL_LINE_LOOP; shape4.vertexCount = 5;  // 5�� ���� �����ؼ� ������
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

			// ���� ���� Ÿ�԰� ��ǥ ���� Ÿ�Կ� ���� ��ȯ ó��
			if (shape.shapeType == 2 && shape.targetShapeType == 3) { // �� -> �ﰢ��
				shape.drawMode = GL_LINE_LOOP;
				shape.vertexCount = 3;

				// �ﰢ�� ��ǥ ��ǥ: ������ 3��
				float targets[5][2] = {
					{centerX, centerY + 0.3f},         // ��� ������
					{centerX - 0.15f, centerY - 0.15f}, // ���� ������
					{centerX + 0.15f, centerY - 0.15f}, // ���� ������
					{centerX - 0.075f, centerY + 0.075f}, // ���� �� ���� (����)
					{centerX + 0.075f, centerY + 0.075f}  // ������ �� ���� (����)
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
			else if (shape.shapeType == 3 && shape.targetShapeType == 4) { // �ﰢ�� -> �簢��
				shape.drawMode = GL_LINE_LOOP;
				shape.vertexCount = 4;

				// �簢�� ��ǥ ��ǥ: 4�� ������ (�ð����)
				float targets[5][2] = {
					{centerX - 0.25f, centerY - 0.25f}, // ����
					{centerX + 0.25f, centerY - 0.25f}, // ����
					{centerX + 0.25f, centerY + 0.25f}, // ���
					{centerX - 0.25f, centerY + 0.25f}, // �»�
					{centerX, centerY}                   // �߽� (����)
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
			else if (shape.shapeType == 4 && shape.targetShapeType == 5) { // �簢�� -> ������
				shape.drawMode = GL_LINE_LOOP;
				shape.vertexCount = 5;

				// ������ ��ǥ ��ǥ: 5�� ������
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
			else if (shape.shapeType == 5 && shape.targetShapeType == 2) { // ������ -> ��
				shape.drawMode = GL_LINE_STRIP;
				shape.vertexCount = 5;

				// �� ��ǥ ��ǥ: 5�� ���� ����������
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

	// isAnimating ���� ������Ʈ
	isAnimating = anyAnimating;

	if (anyAnimating) {
		UpdateBuffer();
		glutPostRedisplay();
		glutTimerFunc(16, Timer, 1);
	}
	else {
		// �ִϸ��̼��� �Ϸ�Ǹ� Ÿ�̸Ӹ� ���߰�, �ʿ��� �� �ٽ� ����
		glutPostRedisplay();
	}
}

void changeShapes() {
	bool startTimer = false;

	for (auto& shape : shapes) {
		if (shape.quadrant == 0) continue; // ���� ����

		if (command == 2 && shape.shapeType == 2) { // l: �� -> �ﰢ��
			shape.isTransforming = true;
			shape.targetShapeType = 3;
			startTimer = true;
		}
		else if (command == 3 && shape.shapeType == 3) { // t: �ﰢ�� -> �簢��
			shape.isTransforming = true;
			shape.targetShapeType = 4;
			startTimer = true;
		}
		else if (command == 4 && shape.shapeType == 4) { // r: �簢�� -> ������
			shape.isTransforming = true;
			shape.targetShapeType = 5;
			startTimer = true;
		}
		else if (command == 5 && shape.shapeType == 5) { // p: ������ -> ��
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
		// ��� �ִϸ��̼� ����
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
	glutTimerFunc(16, Timer, 1); // Ÿ�̸� ����
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