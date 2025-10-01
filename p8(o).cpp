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
//--- �ʿ��� ���� ����
GLint width, height;
GLuint shaderProgramID; //--- ���̴� ���α׷� �̸�
GLuint vertexShader; //--- ���ؽ� ���̴� ��ü
GLuint fragmentShader; //--- �����׸�Ʈ ���̴� ��ü

std::random_device rd;  // �õ尪�� ��� ���� random_device ����.
std::mt19937 gen(rd());	// random_device �� ���� ���� ���� ������ �ʱ�ȭ �Ѵ�.
std::uniform_real_distribution<float> dis_color(0.0f, 1.0f); // 0.0f ���� 1.0f ���� �յ��ϰ� ��Ÿ���� �������� �����ϱ� ���� �յ� ���� ����.


#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

int currentShape = 0;
int selectedShapeIndex = -1;

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

bool IsMouseInShape(float x, float y, const Shape& shape) {
	float dx = x - shape.centerX;
	float dy = y - shape.centerY;

	if (dx < 0) dx = -dx;
	if (dy < 0) dy = -dy;

	switch (shape.shapeType) {
		case 0: // ��
			return (dx <= 0.05f && dy <= 0.05f); // ������ 0.05
		case 1: // ��
			return (dy <= 0.01f && dx <= 0.1f); // �� �߽� �ֺ� ����
		case 2: { // �ﰢ��
			// �ﰢ���� �� ������ (AddShape���� ���ǵ� ��ǥ ����)
			float x1 = shape.centerX;           // ���� ������
			float y1 = shape.centerY + 0.15f;
			float x2 = shape.centerX - 0.15f;   // ���� �Ʒ� ������
			float y2 = shape.centerY - 0.1f;
			float x3 = shape.centerX + 0.15f;   // ������ �Ʒ� ������
			float y3 = shape.centerY - 0.1f;

			// �����߽� ��ǥ�� �̿��� �ﰢ�� ���� �Ǻ�
			float denominator = (y2 - y3) * (x1 - x3) + (x3 - x2) * (y1 - y3);
			if (abs(denominator) < 0.0001f) return false; // �ﰢ���� �ƴ� ���

			float a = ((y2 - y3) * (x - x3) + (x3 - x2) * (y - y3)) / denominator;
			float b = ((y3 - y1) * (x - x3) + (x1 - x3) * (y - y3)) / denominator;
			float c = 1 - a - b;

			return (a >= 0 && b >= 0 && c >= 0);
		}
		case 3: // �簢��
			return abs(dx) <= 0.1f && abs(dy) <= 0.1f; // �簢�� ����
	}
	return false;
}

//void MoveShape(int shapeIndex, float deltaX, float deltaY) {
//	if (shapeIndex < 0 || shapeIndex >= shapes.size()) return;
//
//	Shape& shape = shapes[shapeIndex];
//	int startVertex = shape.startIndex * 3;
//	int endVertex
//}

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

void AddShape(float x, float y, int shapeType) {
	Shape newShape;
	newShape.startIndex = allVertices.size() / 3;

	switch (shapeType) {
	case 0: // ��
		allVertices.push_back(x);
		allVertices.push_back(y);
		allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen));
		allColors.push_back(dis_color(gen));
		allColors.push_back(dis_color(gen));

		newShape.drawMode = GL_POINTS;
		newShape.vertexCount = 1;
		newShape.centerX = x;
		newShape.centerY = y;
		newShape.shapeType = 0;
		break;

	case 1: // ��
		allVertices.push_back(x - 0.1f);
		allVertices.push_back(y);
		allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen));
		allColors.push_back(dis_color(gen));
		allColors.push_back(dis_color(gen));
		allVertices.push_back(x + 0.1f);
		allVertices.push_back(y);
		allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen));
		allColors.push_back(dis_color(gen));
		allColors.push_back(dis_color(gen));
		newShape.drawMode = GL_LINES;
		newShape.vertexCount = 2;
		newShape.centerX = x;
		newShape.centerY = y;
		newShape.shapeType = 1;
		break;

	case 2: // �ﰢ��
		allVertices.push_back(x);
		allVertices.push_back(y + 0.15f);
		allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen));
		allColors.push_back(dis_color(gen));
		allColors.push_back(dis_color(gen));
		allVertices.push_back(x - 0.15f);
		allVertices.push_back(y - 0.1f);
		allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen));
		allColors.push_back(dis_color(gen));
		allColors.push_back(dis_color(gen));
		allVertices.push_back(x + 0.15f);
		allVertices.push_back(y - 0.1f);
		allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen));
		allColors.push_back(dis_color(gen));
		allColors.push_back(dis_color(gen));
		newShape.drawMode = GL_TRIANGLES;
		newShape.vertexCount = 3;
		newShape.centerX = x;
		newShape.centerY = y;
		newShape.shapeType = 2;
		break;

	case 3: // �簢��
		// ���� ��
		allVertices.push_back(x - 0.1f);
		allVertices.push_back(y + 0.1f);
		allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen));
		allColors.push_back(dis_color(gen));
		allColors.push_back(dis_color(gen));
		// ������ ��
		allVertices.push_back(x + 0.1f);
		allVertices.push_back(y + 0.1f);
		allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen));
		allColors.push_back(dis_color(gen));
		allColors.push_back(dis_color(gen));
		// ������ �Ʒ�
		allVertices.push_back(x + 0.1f);
		allVertices.push_back(y - 0.1f);
		allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen));
		allColors.push_back(dis_color(gen));
		allColors.push_back(dis_color(gen));
		// ���� ��
		allVertices.push_back(x - 0.1f);
		allVertices.push_back(y + 0.1f);
		allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen));
		allColors.push_back(dis_color(gen));
		allColors.push_back(dis_color(gen));
		// ���� �Ʒ�
		allVertices.push_back(x - 0.1f);
		allVertices.push_back(y - 0.1f);
		allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen));
		allColors.push_back(dis_color(gen));
		allColors.push_back(dis_color(gen));
		// ������ �Ʒ�
		allVertices.push_back(x + 0.1f);
		allVertices.push_back(y - 0.1f);
		allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen));
		allColors.push_back(dis_color(gen));
		allColors.push_back(dis_color(gen));
		newShape.drawMode = GL_TRIANGLES;
		newShape.vertexCount = 6;
		newShape.centerX = x;
		newShape.centerY = y;
		newShape.shapeType = 3;
		break;
	}

	shapes.push_back(newShape);
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
	case 'p':
		currentShape = 0; // ��
		break;
	case 'l':	// ������
		currentShape = 1; // ��
		if (selectedShapeIndex != -1) {
			Shape& shape = shapes[selectedShapeIndex];
			for (int i = 0; i < shape.vertexCount; ++i) {
				allVertices[(shape.startIndex + i) * 3 + 1] -= 0.05f; // y ��ǥ ����
				allVertices[(shape.startIndex + i) * 3] += 0.05f; // x ��ǥ ����
			}
			UpdateBuffer();
		}
		break;
	case 't':
		currentShape = 2; // �ﰢ��
		break;
	case 'r':
		currentShape = 3; // �簢��
		break;
	case 'w':
		if (selectedShapeIndex != -1) {
			Shape& shape = shapes[selectedShapeIndex];
			for (int i = 0; i < shape.vertexCount; ++i) {
				allVertices[(shape.startIndex + i) * 3 + 1] += 0.05f; // y ��ǥ ����
			}
			UpdateBuffer();
		}
		break;
	case 's':
		if (selectedShapeIndex != -1) {
			Shape& shape = shapes[selectedShapeIndex];
			for (int i = 0; i < shape.vertexCount; ++i) {
				allVertices[(shape.startIndex + i) * 3 + 1] -= 0.05f; // y ��ǥ ����
			}
			UpdateBuffer();
		}
		break;
	case 'd':
		if (selectedShapeIndex != -1) {
			Shape& shape = shapes[selectedShapeIndex];
			for (int i = 0; i < shape.vertexCount; ++i) {
				allVertices[(shape.startIndex + i) * 3] += 0.05f; // x ��ǥ ����
			}
			UpdateBuffer();
		}
		break;
	case 'a':
		if (selectedShapeIndex != -1) {
			Shape& shape = shapes[selectedShapeIndex];
			for (int i = 0; i < shape.vertexCount; ++i) {
				allVertices[(shape.startIndex + i) * 3] -= 0.05f; // x ��ǥ ����
			}
			UpdateBuffer();
		}
		break;
	case 'i':	// �»���
		if (selectedShapeIndex != -1) {
			Shape& shape = shapes[selectedShapeIndex];
			for (int i = 0; i < shape.vertexCount; ++i) {
				allVertices[(shape.startIndex + i) * 3] -= 0.05f; // x ��ǥ ����
				allVertices[(shape.startIndex + i) * 3 + 1] += 0.05f; // y ��ǥ ����
			}
			UpdateBuffer();
		}
		break;
	case 'j':	// �����
		if (selectedShapeIndex != -1) {
			Shape& shape = shapes[selectedShapeIndex];
			for (int i = 0; i < shape.vertexCount; ++i) {
				allVertices[(shape.startIndex + i) * 3] += 0.05f; // x ��ǥ ����
				allVertices[(shape.startIndex + i) * 3 + 1] += 0.05f; // y ��ǥ ����
			}
			UpdateBuffer();
		}
		break;
	case 'k':	// ������
		if (selectedShapeIndex != -1) {
			Shape& shape = shapes[selectedShapeIndex];
			for (int i = 0; i < shape.vertexCount; ++i) {
				allVertices[(shape.startIndex + i) * 3] -= 0.05f; // x ��ǥ ����
				allVertices[(shape.startIndex + i) * 3 + 1] -= 0.05f; // y ��ǥ ����
			}
			UpdateBuffer();
		}
		break;
	case 'c':
		allVertices.clear();
		allColors.clear();
		shapes.clear();
		selectedShapeIndex = -1;
		break;
	}
	glutPostRedisplay(); //--- ������ �ٲ� ������ ��� �ݹ� �Լ��� ȣ���Ͽ� ȭ���� refresh �Ѵ�
}

GLvoid Mouse(int button, int state, int x, int y)
{
	float Mouse_x = mapToGLCoordX(x);
	float Mouse_y = mapToGLCoordY(y);
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		if (shapes.size() < 10) AddShape(Mouse_x, Mouse_y, currentShape);
		else {
			selectedShapeIndex = -1;

			for (int i = shapes.size() - 1; i >= 0; --i) {
				if (IsMouseInShape(Mouse_x, Mouse_y, shapes[i])) {
					selectedShapeIndex = i;
					break;
				}
			}
		}
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
	glutCreateWindow("Practice 8");
	//--- GLEW �ʱ�ȭ�ϱ�
	glewExperimental = GL_TRUE;
	glewInit();
	shaderProgramID = make_shaderProgram();
	InitBuffer();
	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
	glutMouseFunc(Mouse);
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