//--- �ʿ��� ������� ����
#define _CRT_SECURE_NO_WARNINGS //--- ���α׷� �� �տ� ������ ��
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <vector>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 800

//------------------------------------------------------------
// ������ ���� ���� ����
// ����������(uniform) ������ ������ �Ҵ������� �ݵ�� ����� ��.
// ������� ������ ���α׷��� ������� ���� �� ����.
//------------------------------------------------------------

struct Vertex {
	GLfloat x, y;
	GLfloat r = 0.9f, g = 0.9f, b = 0.f;
};
struct TRIANGLE {
	GLfloat centerX{}, centerY{};
	Vertex vertices[3];
	GLfloat angle{ 0.f };
	GLfloat scale{ 1.0f };
	GLfloat scaleDirection{ 1.0f };
};

TRIANGLE Triangles[4];
void SetTriangles(int type);
GLfloat pivotX = 0.f, pivotY = 0.f;
int selectedTriangle = -1;

int type = 1;
bool Stop = false;
bool CW = false;
bool CCW = false;

GLuint VBO, VAO;

float bgR = 1.f, bgG = 1.f, bgB = 1.f;

GLfloat transformx(int x) { return ((float)x / (WINDOW_WIDTH / 2)) - 1.0f; }
GLfloat transformy(int y) { return ((WINDOW_HEIGHT - (float)y) / (WINDOW_HEIGHT / 2)) - 1.0f; }
void make_vertexShaders();
void make_fragmentShaders();
GLuint make_shaderProgram();
GLvoid drawScene();
GLvoid Reshape(int w, int h);
GLvoid KeyBoard(unsigned char key, int x, int y);
GLvoid Timer(int value);
void init();
void UpdateVBO();
void RotateTriangles_CW();
void RotateTriangles_CCW();

//--- �ʿ��� ���� ����
GLuint shaderProgramID; //--- ���̴� ���α׷� �̸�
GLuint vertexShader; //--- ���ؽ� ���̴� ��ü
GLuint fragmentShader; //--- �����׸�Ʈ ���̴� ��ü

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

//--- ���� �Լ�
void main(int argc, char** argv) //--- ������ ����ϰ� �ݹ��Լ� ����
{
	//--- ������ �����ϱ�
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutCreateWindow("Example1");

	//--- GLEW �ʱ�ȭ�ϱ�
	glewExperimental = GL_TRUE;
	glewInit();

	//--- ���̴� �о�ͼ� ���̴� ���α׷� �����: ����� �����Լ� ȣ��
	make_vertexShaders(); //--- ���ؽ� ���̴� �����
	make_fragmentShaders(); //--- �����׸�Ʈ ���̴� �����
	shaderProgramID = make_shaderProgram();

	init();
	SetTriangles(type);

	//--- ���̴� ���α׷� �����
	glutDisplayFunc(drawScene); //--- ��� �ݹ� �Լ�
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(KeyBoard);
	glutTimerFunc(16, Timer, 1);
	glutMainLoop();
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
		return;
	}
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
		return;
	}
}

//--- ���̴� ���α׷� ����� ���̴� ��ü ��ũ�ϱ�
GLuint make_shaderProgram()
{
	GLint result;
	GLchar* errorLog = NULL;
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
		return false;
	}
	glUseProgram(shaderID); //--- ������� ���̴� ���α׷� ����ϱ�
	//--- ���� ���� ���̴����α׷� ���� �� �ְ�, �� �� �Ѱ��� ���α׷��� ����Ϸ���
	//--- glUseProgram �Լ��� ȣ���Ͽ� ��� �� Ư�� ���α׷��� �����Ѵ�.
	//--- ����ϱ� ������ ȣ���� �� �ִ�.
	return shaderID;
}

void init() {
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(2 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}


//--- ��� �ݹ� �Լ�
GLvoid drawScene() //--- �ݹ� �Լ�: �׸��� �ݹ� �Լ�
{
	glClearColor(bgR, bgG, bgB, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(shaderProgramID);
	glBindVertexArray(VAO);

	glDrawArrays(GL_TRIANGLES, 0, 12); //--- �������ϱ�: 0�� �ε������� 1���� ���ؽ��� ����Ͽ� �� �׸���

	glutSwapBuffers(); // ȭ�鿡 ����ϱ�
}

//--- �ٽñ׸��� �ݹ� �Լ�
GLvoid Reshape(int w, int h) //--- �ݹ� �Լ�: �ٽ� �׸��� �ݹ� �Լ�
{
	glViewport(0, 0, w, h);
}

GLvoid KeyBoard(unsigned char key, int x, int y) //--- �ݹ� �Լ�: Ű���� �ݹ� �Լ� 
{
	switch (key)
	{
	case '1': {
		if (type == 1) break;
		type = 1;
		SetTriangles(type);
		break;
	}
	case '2': {
		if (type == 2) break;
		type = 2;
		SetTriangles(type);
		break;
	}
	case 'c':
	case 'C': {
		CW = true;
		CCW = false;
		break;
	}
	case 't':
	case 'T': {
		CW = false;
		CCW = true;
		break;
	}
	case 's':
	case 'S': {
		Stop = !Stop;
		glutTimerFunc(16, Timer, 1);
		break;
	}
	case 'r':
	case 'R': {
		int i = selectedTriangle = (selectedTriangle + 1) % 4;
		pivotX = Triangles[i].vertices[1].x;
		pivotY = Triangles[i].vertices[1].y;
		break;
	}
	case 'Q':
	case 'q':
		exit(0);
		break;
	}
	glutPostRedisplay();
}

GLvoid Timer(int value) {
	if (CW) RotateTriangles_CW();
	else if (CCW) RotateTriangles_CCW();

	if (!Stop) glutTimerFunc(16, Timer, 1);
	glutPostRedisplay();
}


void UpdateVBO() {
	std::vector<Vertex> allVertices;
	allVertices.clear();
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 3; ++j)
			allVertices.push_back(Triangles[i].vertices[j]);
	}
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, allVertices.size() * sizeof(Vertex), allVertices.data(), GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void SetTriangles(int type) {
	auto& tri = Triangles;

	float pos[4][2] = {
		{ 0.f,  0.3f },
		{ 0.3f, 0.f  },
		{ 0.f, -0.3f },
		{-0.3f, 0.f  }
	};

	for (int i = 0; i < 4; ++i) {
		float cx = pos[i][0];
		float cy = pos[i][1];

		if (type == 1) {
			if (i == 0) { // ��
				tri[i].vertices[0] = { cx - 0.2f, cy - 0.1f };
				tri[i].vertices[1] = { cx,        cy + 0.2f };
				tri[i].vertices[2] = { cx + 0.2f, cy - 0.1f };
			}
			else if (i == 1) { // ������
				tri[i].vertices[0] = { cx - 0.1f, cy + 0.2f };
				tri[i].vertices[1] = { cx + 0.2f, cy };
				tri[i].vertices[2] = { cx - 0.1f, cy - 0.2f };
			}
			else if (i == 2) { // �Ʒ�
				tri[i].vertices[0] = { cx + 0.2f, cy + 0.1f };
				tri[i].vertices[1] = { cx,        cy - 0.2f };
				tri[i].vertices[2] = { cx - 0.2f, cy + 0.1f };
			}
			else if (i == 3) { // ����
				tri[i].vertices[0] = { cx + 0.1f, cy - 0.2f };
				tri[i].vertices[1] = { cx - 0.2f, cy };
				tri[i].vertices[2] = { cx + 0.1f, cy + 0.2f };
			}
		}
		else if (type == 2) {
			if (i == 0) { // ��
				tri[i].vertices[0] = { cx - 0.2f, cy - 0.1f };
				tri[i].vertices[2] = { cx + 0.2f, cy - 0.1f };
			}
			else if (i == 1) { // ������
				tri[i].vertices[0] = { cx - 0.1f, cy + 0.2f };
				tri[i].vertices[2] = { cx - 0.1f, cy - 0.2f };
			}
			else if (i == 2) { // �Ʒ�
				tri[i].vertices[0] = { cx + 0.2f, cy + 0.1f };
				tri[i].vertices[2] = { cx - 0.2f, cy + 0.1f };
			}
			else if (i == 3) { // ����
				tri[i].vertices[0] = { cx + 0.1f, cy - 0.2f };
				tri[i].vertices[2] = { cx + 0.1f, cy + 0.2f };
			}

			float dx = -cx;
			float dy = -cy;
			float len = sqrtf(dx * dx + dy * dy);
			dx /= len; dy /= len;
			tri[i].vertices[1] = { cx + dx * 0.25f, cy + dy * 0.25f };
		}

		tri[i].centerX = (tri[i].vertices[0].x + tri[i].vertices[2].x) / 2.0f;
		tri[i].centerY = (tri[i].vertices[0].y + tri[i].vertices[2].y) / 2.0f + 0.1f;

		for (int j = 0; j < 3; ++j) {
			float x = tri[i].vertices[j].x;
			float y = tri[i].vertices[j].y;
			float angle = 0.f;
			if (CW) angle = -tri[i].angle;
			else if (CCW) angle = tri[i].angle;
			tri[i].vertices[j].x = x * cos(angle) - y * sin(angle);
			tri[i].vertices[j].y = x * sin(angle) + y * cos(angle);
		}
	}

	UpdateVBO();
}

void RotateTriangles_CW() {
	std::cout << "Running CW Rotation" << std::endl;
	auto& tri = Triangles;
	const float delta = 0.05f;
	const float scaleDelta = 0.01f;
	const float minScale = 0.5f;
	const float maxScale = 1.5f;

	for (int i = 0; i < 4; ++i) {
		tri[i].angle += delta;
		if (tri[i].angle >= 6.2831853f) tri[i].angle = -6.2831853f;

		tri[i].scale += scaleDelta * tri[i].scaleDirection;
		if (tri[i].scale >= maxScale) {
			tri[i].scale = maxScale;
			tri[i].scaleDirection = -1.0f;
		}
		else if (tri[i].scale <= minScale) {
			tri[i].scale = minScale;
			tri[i].scaleDirection = 1.0f;
		}

		float angle = -delta;
		for (int j = 0; j < 3; ++j) {
			float x = tri[i].vertices[j].x - pivotX;
			float y = tri[i].vertices[j].y - pivotY;

			x /= (tri[i].scale - scaleDelta * tri[i].scaleDirection);
			y /= (tri[i].scale - scaleDelta * tri[i].scaleDirection);

			float rotatedX = x * cos(angle) - y * sin(angle);
			float rotatedY = x * sin(angle) + y * cos(angle);

			tri[i].vertices[j].x = pivotX + rotatedX * tri[i].scale;
			tri[i].vertices[j].y = pivotY + rotatedY * tri[i].scale;
		}
	}
	UpdateVBO();
}

void RotateTriangles_CCW() {
	std::cout << "Running CCW Rotation" << std::endl;
	auto& tri = Triangles;
	const float delta = 0.05f;
	const float scaleDelta = 0.01f;
	const float minScale = 0.5f;
	const float maxScale = 1.5f;

	for (int i = 0; i < 4; ++i) {
		tri[i].angle += delta;
		if (tri[i].angle >= 6.2831853f) tri[i].angle = -6.2831853f;

		tri[i].scale += scaleDelta * tri[i].scaleDirection;
		if (tri[i].scale >= maxScale) {
			tri[i].scale = maxScale;
			tri[i].scaleDirection = -1.0f;
		}
		else if (tri[i].scale <= minScale) {
			tri[i].scale = minScale;
			tri[i].scaleDirection = 1.0f;
		}

		float angle = delta;
		for (int j = 0; j < 3; ++j) {
			float x = tri[i].vertices[j].x - pivotX;
			float y = tri[i].vertices[j].y - pivotY;

			x /= (tri[i].scale - scaleDelta * tri[i].scaleDirection);
			y /= (tri[i].scale - scaleDelta * tri[i].scaleDirection);

			float rotatedX = x * cos(angle) - y * sin(angle);
			float rotatedY = x * sin(angle) + y * cos(angle);

			tri[i].vertices[j].x = pivotX + rotatedX * tri[i].scale;
			tri[i].vertices[j].y = pivotY + rotatedY * tri[i].scale;
		}
	}
	UpdateVBO();
}