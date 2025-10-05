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
std::uniform_real_distribution<float> dis_x(-0.9f, 0.9f);
std::uniform_real_distribution<float> dis_y(-0.9f, 0.7f);

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

struct Shape {
	int startIndex;
	int vertexCount;
	float centerX, centerY;
};

std::vector<GLfloat> allVertices;
std::vector<GLfloat> allColors;
std::vector<Shape> shapes;
GLuint vao, vbo[2];
bool num1, num2, num3, num4 = false;
float bigDirX = 0.05f, bigDirY = 0.05f;
float smallDirX = 0.03f, smallDirY = 0.03f;
bool timerRunning = false;
float bigAngle = 0.0f, smallAngle = 0.0f;
float bigRadius = 0.1f, smallRadius = 0.05f;
float bigCenterX = 0.0f, bigCenterY = 0.0f;
float smallCenterX = 0.0f, smallCenterY = 0.0f;
int bigSpiralDirection = 0, smallSpiralDirection = 0;		// 0 : ����, 1 : �Ʒ�, 2 : ������, 3 : ��
int bigStepsInDirection = 0, smallStepsInDirection = 0;	// ���� �������� �̵��� ���� ��
int bigStepsToMove = 1, smallStepsToMove = 1;		// ���� �������� �̵��ؾ� �� �� ���� ��
float bigStepSize = 0.02f, smallStepSize = 0.015f;	// �� ���ܴ� �̵��Ÿ�

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
	Shape bigTriangle;
	bigTriangle.startIndex = allVertices.size() / 3;
	bigTriangle.vertexCount = 3;
	bigTriangle.centerX = dis_x(gen);
	bigTriangle.centerY = dis_y(gen);
	float x = bigTriangle.centerX;
	float y = bigTriangle.centerY;

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

	shapes.push_back(bigTriangle);

	Shape smallTriangle;
	smallTriangle.startIndex = allVertices.size() / 3;
	smallTriangle.vertexCount = 3;
	smallTriangle.centerX = dis_x(gen);
	smallTriangle.centerY = dis_y(gen);
	x = smallTriangle.centerX;
	y = smallTriangle.centerY;

	allVertices.push_back(x);
	allVertices.push_back(y + 0.15f);
	allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allVertices.push_back(x - 0.05f);
	allVertices.push_back(y - 0.05f);
	allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allVertices.push_back(x + 0.05f);
	allVertices.push_back(y - 0.05f);
	allVertices.push_back(0.0f);
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));
	allColors.push_back(dis_color(gen));

	shapes.push_back(smallTriangle);
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
		glDrawArrays(GL_TRIANGLES, shape.startIndex, shape.vertexCount);
	}

	glutSwapBuffers(); //--- ȭ�鿡 ����ϱ�
}

GLvoid Keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case '1':		// �밢�� �̵�
		num1 = !num1;
		if (num1) {
			num2 = num3 = num4 = false;
			bigDirX = 0.02f; bigDirY = 0.02f;
			smallDirX = 0.01f; smallDirY = 0.01f;

			if (!timerRunning) {
				timerRunning = true;
				glutTimerFunc(16, TimerFunction, 1);
			}
		}
		break;
	case '2':
		num2 = !num2;
		if (num2) {
			num1 = num3 = num4 = false;
			bigDirX = 0.02f; bigDirY = 0.2f;
			smallDirX = 0.01f; smallDirY = 0.2f;

			if (!timerRunning) {
				timerRunning = true;
				glutTimerFunc(16, TimerFunction, 1);
			}
		}
		break;
	case '3':
		num3 = !num3;
		if (num3) {
			num1 = num2 = num4 = false;
			
			bigSpiralDirection = 0;
			smallSpiralDirection = 0;
			bigStepsInDirection = 0;
			smallStepsInDirection = 0;
			bigStepsToMove = 1;
			smallStepsToMove = 1;
			bigCenterX = shapes[0].centerX;
			bigCenterY = shapes[0].centerY;
			smallCenterX = shapes[1].centerX;
			smallCenterY = shapes[1].centerY;

			if (!timerRunning) {
				timerRunning = true;
				glutTimerFunc(16, TimerFunction, 1);
			}
		}
		break;
	case '4':
		num4 = !num4;
		if (num4) {
			num1 = num2 = num3 = false;
			
			bigAngle = 0.0f;
			smallAngle = 0.0f;
			bigRadius = 0.1f;
			smallRadius = 0.05f;
			bigCenterX = shapes[0].centerX;
			bigCenterY = shapes[0].centerY;
			smallCenterX = shapes[1].centerX;
			smallCenterY = shapes[1].centerY;

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
	glutPostRedisplay(); //--- ������ �ٲ� ������ ��� �ݹ� �Լ��� ȣ���Ͽ� ȭ���� refresh �Ѵ�
}

GLvoid TimerFunction(int value) {
	if (!num1 && !num2 && !num3 && !num4) {
		timerRunning = false;
		return;
	}

	Shape& big = shapes[0];
	Shape& small = shapes[1];
	
	if (num1) {		// �밢�� �̵�
		if (big.centerX + 0.1f >= 1.0f || big.centerX - 0.1f <= -1.0f) {
			bigDirX *= -1;
		}
		if (big.centerY + 0.3f >= 1.0f || big.centerY - 0.1f <= -1.0f) {
			bigDirY *= -1;
		}

		if (small.centerX + 0.05f >= 1.0f || small.centerX - 0.05f <= -1.0f) {
			smallDirX *= -1;
		}
		if (small.centerY + 0.15f >= 1.0f || small.centerY - 0.05f <= -1.0f) {
			smallDirY *= -1;
		}

		// x �� ����
		for (int i = 0; i < 3; ++i) {
			allVertices[(big.startIndex + i) * 3] += bigDirX;
			allVertices[(small.startIndex + i) * 3] += smallDirX;
		}

		// y�� ����
		for (int i = 0; i < 3; ++i) {
			allVertices[(big.startIndex + i) * 3 + 1] += bigDirY;
			allVertices[(small.startIndex + i) * 3 + 1] += smallDirY;
		}

		big.centerX += bigDirX;
		small.centerX += smallDirX;
		big.centerY += bigDirY;
		small.centerY += smallDirY;
	}
	else if (num2) {		// ������� �̵�
		if (big.centerX + 0.1f >= 1.0f || big.centerX - 0.1f <= -1.0f) {
			bigDirX *= -1;
			for (int i = 0; i < 3; ++i) {
				allVertices[(big.startIndex + i) * 3 + 1] += bigDirY;
			}
			big.centerY += bigDirY;
		}
		if (big.centerY + 0.5f >= 1.0f || big.centerY - 0.2f <= -1.0f) {
			bigDirY *= -1;
		}

		if (small.centerX + 0.05f >= 1.0f || small.centerX - 0.05f <= -1.0f) {
			smallDirX *= -1;
			for (int i = 0; i < 3; ++i) {
				allVertices[(small.startIndex + i) * 3 + 1] += smallDirY;
			}
			small.centerY += smallDirY;
		}
		if (small.centerY + 0.3f >= 1.0f || small.centerY - 0.2f <= -1.0f) {
			smallDirY *= -1;
		}

		// x �� ����
		for (int i = 0; i < 3; ++i) {
			allVertices[(big.startIndex + i) * 3] += bigDirX;
			allVertices[(small.startIndex + i) * 3] += smallDirX;
		}

		big.centerX += bigDirX;
		small.centerX += smallDirX;
	}
	else if (num3) {		// �簢 �����̷� �̵�
		float bigDeltaX = 0.0f, bigDeltaY = 0.0f;
		switch (bigSpiralDirection) {
		case 0: bigDeltaX = bigStepSize; break;	// ������
		case 1: bigDeltaY = -bigStepSize; break;	// �Ʒ���
		case 2: bigDeltaX = -bigStepSize; break;		// ����
		case 3: bigDeltaY = bigStepSize; break;		// ����
		}

		float smallDeltaX = 0.0f, smallDeltaY = 0.0f;
		switch (smallSpiralDirection) {
		case 0: smallDeltaX = smallStepSize; break;	// ������
		case 1: smallDeltaY = -smallStepSize; break;	// �Ʒ���
		case 2: smallDeltaX = -smallStepSize; break;		// ����
		case 3: smallDeltaY = smallStepSize; break;		// ����
		}

		// ��� ���� �̵�
		for (int i = 0; i < 3; ++i) {
			allVertices[(big.startIndex + i) * 3] += bigDeltaX;
			allVertices[(big.startIndex + i) * 3 + 1] += bigDeltaY;
			allVertices[(small.startIndex + i) * 3] += smallDeltaX;
			allVertices[(small.startIndex + i) * 3 + 1] += smallDeltaY;
		}

		// �߽��� ������Ʈ
		big.centerX += bigDeltaX;
		big.centerY += bigDeltaY;
		small.centerX += smallDeltaX;
		small.centerY += smallDeltaY;

		// ū �ﰢ�� ���� ����
		bigStepsInDirection++;
		if (bigStepsInDirection >= bigStepsToMove) {
			bigStepsInDirection = 0;
			bigSpiralDirection = (bigSpiralDirection + 1) % 4;

			// �������̳� ���� ������ ������ �̵� �Ÿ� ����
			if (bigSpiralDirection == 2 || bigSpiralDirection == 0) {
				bigStepsToMove++;
			}
		}

		// ���� �ﰢ�� ���� ���� (�ٸ� �ӵ���)
		smallStepsInDirection++;
		if (smallStepsInDirection >= smallStepsToMove) {
			smallStepsInDirection = 0;
			smallSpiralDirection = (smallSpiralDirection + 1) % 4;

			// �������̳� ���� ������ ������ �̵� �Ÿ� ����
			if (smallSpiralDirection == 2 || smallSpiralDirection == 0) {
				smallStepsToMove++;
			}
		}

		// ȭ�� ��踦 ����� ����
		if (abs(big.centerX - bigCenterX) > 0.8f || abs(big.centerY - bigCenterY) > 0.8f) {
			bigSpiralDirection = 0;
			bigStepsInDirection = 0;
			bigStepsToMove = 1;
			big.centerX = bigCenterX;
			big.centerY = bigCenterY;

			// �����鵵 �߽����� ����
			allVertices[big.startIndex * 3] = bigCenterX;
			allVertices[big.startIndex * 3 + 1] = bigCenterY + 0.3f;
			allVertices[(big.startIndex + 1) * 3] = bigCenterX - 0.1f;
			allVertices[(big.startIndex + 1) * 3 + 1] = bigCenterY - 0.1f;
			allVertices[(big.startIndex + 2) * 3] = bigCenterX + 0.1f;
			allVertices[(big.startIndex + 2) * 3 + 1] = bigCenterY - 0.1f;
		}

		if (abs(small.centerX - smallCenterX) > 0.6f || abs(small.centerY - smallCenterY) > 0.6f) {
			smallSpiralDirection = 0;
			smallStepsInDirection = 0;
			smallStepsToMove = 1;
			small.centerX = smallCenterX;
			small.centerY = smallCenterY;

			// �����鵵 �߽����� ����
			allVertices[small.startIndex * 3] = smallCenterX;
			allVertices[small.startIndex * 3 + 1] = smallCenterY + 0.15f;
			allVertices[(small.startIndex + 1) * 3] = smallCenterX - 0.05f;
			allVertices[(small.startIndex + 1) * 3 + 1] = smallCenterY - 0.05f;
			allVertices[(small.startIndex + 2) * 3] = smallCenterX + 0.05f;
			allVertices[(small.startIndex + 2) * 3 + 1] = smallCenterY - 0.05f;
		}
	}
	else if (num4) {		// �� �����̷� �̵�
		// 1. ������ ���������� ���� (ȸ�� ȿ��)
		bigAngle += 0.1f;     // += ������� ���� ����
		smallAngle += 0.15f;  // ���� �ﰢ���� �� ������ ȸ��

		// 2. �������� ���������� ���� (�����̷� ȿ��)
		bigRadius += 0.002f;    // ���� ���̴� ������ ����
		smallRadius += 0.001f;

		// 3. ���ο� ��ġ ��� (����ǥ -> ������ǥ ��ȯ)
		float newBigCenterX = bigCenterX + bigRadius * cos(bigAngle);
		float newBigCenterY = bigCenterY + bigRadius * sin(bigAngle);
		float newSmallCenterX = smallCenterX + smallRadius * cos(smallAngle);
		float newSmallCenterY = smallCenterY + smallRadius * sin(smallAngle);

		// 4. �̵��� �Ÿ� ���
		float bigDeltaX = newBigCenterX - big.centerX;
		float bigDeltaY = newBigCenterY - big.centerY;
		float smallDeltaX = newSmallCenterX - small.centerX;
		float smallDeltaY = newSmallCenterY - small.centerY;

		// 5. ��� ������ ���ο� ��ġ�� �̵�
		for (int i = 0; i < 3; ++i) {
			allVertices[(big.startIndex + i) * 3] += bigDeltaX;
			allVertices[(big.startIndex + i) * 3 + 1] += bigDeltaY;
			allVertices[(small.startIndex + i) * 3] += smallDeltaX;
			allVertices[(small.startIndex + i) * 3 + 1] += smallDeltaY;
		}

		// 6. �߽��� ������Ʈ (�ùٸ� �Ҵ�)
		big.centerX = newBigCenterX;
		big.centerY = newBigCenterY;
		small.centerX = newSmallCenterX;
		small.centerY = newSmallCenterY;

		// 7. ȭ�� ��踦 ����� ����
		if (bigRadius > 0.8f) {
			bigRadius = 0.1f;
			bigAngle = 0.0f;
		}
		if (smallRadius > 0.6f) {
			smallRadius = 0.05f;
			smallAngle = 0.0f;
		}
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
	glutCreateWindow("Practice 8");
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