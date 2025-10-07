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
GLuint shaderProgramID; //--- ���̴� ���α׷� �̸�
GLuint vertexShader; //--- ���ؽ� ���̴� ��ü
GLuint fragmentShader; //--- �����׸�Ʈ ���̴� ��ü

std::random_device rd;  // �õ尪�� ��� ���� random_device ����.
std::mt19937 gen(rd());	// random_device �� ���� ���� ���� ������ �ʱ�ȭ �Ѵ�.
std::uniform_real_distribution<float> dis_color(0.0f, 0.7f); // 0.0f ���� 1.0f ���� �յ��ϰ� ��Ÿ���� �������� �����ϱ� ���� �յ� ���� ����.
std::uniform_real_distribution<float> dis_x(-0.7f, 0.4f);
std::uniform_real_distribution<float> dis_y(-0.7f, 0.7f);

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

int shapeType = 0;
bool timerRunning = false;
float bgColorR = 0.0f;
float bgColorG = 0.0f;
float bgColorB = 0.0f;

struct Shape {
	GLenum drawMode;
	int startIndex;
	int vertexCount;
};

// �����̷� �ִϸ��̼��� ���� ����ü
struct SpiralAnimation {
	float centerX, centerY;  // �����̷� �߽���
	float angle;             // ���� ����
	float radius;            // ���� ������
	bool active;             // �ִϸ��̼� Ȱ��ȭ ����
	int pointCount;          // ������� ������ ���� ����
	int maxPoints;           // �ִ� �� ����
	bool isExpanding;		// true : Ȯ�� ��, false : ���� ��
	float maxRadius;		// �ִ� ������
	bool hasPreviousPoint;	// �� �����̷��� ���� �� ���� ����
	float prevX, prevY;		// �� �����̷��� ���� �� ��ǥ
};

std::vector<GLfloat> allVertices;
std::vector<GLfloat> allColors;
std::vector<Shape> shapes;
GLuint vao, vbo[2];

// �����̷� �ִϸ��̼� ��ü
std::vector<SpiralAnimation> spirals;

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
	glClearColor(bgColorR, bgColorG, bgColorB, 1.0f);		// ���
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
	glPointSize(3.0f);

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
		shapeType = 0; // ��
		break;
	case 'l':
		shapeType = 1; // ��
		break;
	case '1':
		spirals.clear();
		spirals.push_back({
			dis_x(gen), dis_y(gen),
			0.0f,
			0.01f,
			true,
			0,
			64,
			true,
			0.3f,
			false,
			0.0f, 0.0f
			});
		bgColorR = dis_color(gen);
		bgColorG = dis_color(gen);
		bgColorB = dis_color(gen);

		if (!timerRunning) {
			timerRunning = true;
			glutTimerFunc(16, TimerFunction, 1);
		}
		break;
	case '2':
	case '3':
	case '4':
	case '5':
	{
		// ���ڸ�ŭ �����̷� ����
		int numSpirals = key - '0';
		spirals.clear();

		for (int i = 0; i < numSpirals; ++i) {
			spirals.push_back({
				dis_x(gen), dis_y(gen),  // centerX, centerY
				0.0f,                    // angle
				0.01f,                   // radius
				true,                    // active
				0,                       // pointCount
				64,                      // maxPoints
				true,                    // isExpanding
				0.3f,                    // maxRadius
				false,                   // hasPreviousPoint
				0.0f, 0.0f              // prevX, prevY
				});
		}

		bgColorR = dis_color(gen);
		bgColorG = dis_color(gen);
		bgColorB = dis_color(gen);

		if (!timerRunning) {
			timerRunning = true;
			glutTimerFunc(16, TimerFunction, 1);
		}
	}
	break;
	case 'c':
		allVertices.clear();
		allColors.clear();
		shapes.clear();
		spirals.clear();
		timerRunning = false;
		break;
	case 'q':
		exit(0);
		break;
	}
	glutPostRedisplay(); //--- ������ �ٲ� ������ ��� �ݹ� �Լ��� ȣ���Ͽ� ȭ���� refresh �Ѵ�
}

void AddSpiralPoint(SpiralAnimation& spiral) {
	if (!spiral.active) return;

	// �����̷� ��ġ ���
	float x = spiral.centerX + spiral.radius * cos(spiral.angle);
	float y = spiral.centerY + spiral.radius * sin(spiral.angle);

	Shape newShape;
	// �� �߰�
	if (shapeType == 0) {
		newShape.drawMode = GL_POINTS;
		newShape.startIndex = allVertices.size() / 3;
		newShape.vertexCount = 1;

		allVertices.push_back(x);
		allVertices.push_back(y);
		allVertices.push_back(0.0f);

		for (int i = 0; i < 3; ++i) {
			allColors.push_back(1.0f);
		}

		shapes.push_back(newShape);
	}
	else if (shapeType == 1) {
		if (spiral.hasPreviousPoint) {
			Shape newShape;
			newShape.drawMode = GL_LINES;
			newShape.startIndex = allVertices.size() / 3;
			newShape.vertexCount = 2;
			
			allVertices.push_back(spiral.prevX);
			allVertices.push_back(spiral.prevY);
			allVertices.push_back(0.0f);

			allVertices.push_back(x);
			allVertices.push_back(y);
			allVertices.push_back(0.0f);

			for (int i = 0; i < 6; ++i) {
				allColors.push_back(1.0f);
			}

			shapes.push_back(newShape);
		}

		spiral.prevX = x;
		spiral.prevY = y;
		spiral.hasPreviousPoint = true;
	}

		if (spiral.isExpanding) {
			spiral.angle += 0.3f;
			spiral.radius += 0.005f;

			if (spiral.pointCount >= spiral.maxPoints - 2) {
				spiral.isExpanding = false;

				float finalX = spiral.centerX + spiral.radius * cos(spiral.angle + 3.141592f);
				float finalY = spiral.centerY + spiral.radius * sin(spiral.angle + 3.141592f);

				//currentSpiral.centerX = finalX  * cos(currentSpiral.angle + 3.141592f) + 0.001f;
				//currentSpiral.centerY = finalY  * sin(currentSpiral.angle + 3.141592f) - 0.2f;

				spiral.centerX += 0.3145864306 * 2;
				spiral.centerY += 0.016387239 * 2;

				spiral.angle += 3.141592f;
			}
		}
		else {
			spiral.angle -= 0.3f;
			spiral.radius -= 0.005f;

			if (spiral.radius <= 0.01f) {
				spiral.active = false;
				spiral.hasPreviousPoint = false;
			}
		}

		spiral.pointCount++;

	UpdateBuffer();
}

GLvoid TimerFunction(int value) {
	bool hasActiveSpiral = false;

	for (auto& spiral : spirals) {
		if (spiral.active) {
			AddSpiralPoint(spiral);
			hasActiveSpiral = true;
		}
	}

	if (hasActiveSpiral) {
		UpdateBuffer();
		glutPostRedisplay();
		glutTimerFunc(16, TimerFunction, 1); // ���� �������� ���� Ÿ�̸� ����
	}
	else {
		timerRunning = false;
	}
}

GLvoid Mouse(int button, int state, int x, int y)
{
	float Mouse_x = mapToGLCoordX(x);
	float Mouse_y = mapToGLCoordY(y);
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
		// ���ο� �����̷� �ϳ� �߰�
		spirals.push_back({
			Mouse_x, Mouse_y,        // centerX, centerY
			0.0f,                    // angle
			0.01f,                   // radius
			true,                    // active
			0,                       // pointCount
			64,                      // maxPoints
			true,                    // isExpanding
			0.3f,                    // maxRadius
			false,                   // hasPreviousPoint
			0.0f, 0.0f              // prevX, prevY
			});
		bgColorR = dis_color(gen);
		bgColorG = dis_color(gen);
		bgColorB = dis_color(gen);

		if (!timerRunning) {
			timerRunning = true;
			glutTimerFunc(16, TimerFunction, 1);
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
	glutCreateWindow("Practice 11");
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