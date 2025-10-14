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
std::uniform_real_distribution<float> dis_x(-0.7f, 0.7f); // -0.1f ���� 0.1f ���� �յ��ϰ� ��Ÿ���� �������� �����ϱ� ���� �յ� ���� ����.
std::uniform_real_distribution<float> dis_y(-0.7f, 0.7f); // -0.1f ���� 0.1f ���� �յ��ϰ� ��Ÿ���� �������� �����ϱ� ���� �յ� ���� ����.
std::uniform_real_distribution<float> dis_size(0.05f, 0.15f); // �ﰢ�� ũ��� ���� ����

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

int command = 0;

// �ִϸ��̼� ���� ����
bool isAnimating = false;

struct Shape {
	GLenum drawMode;
	int startIndex;
	int vertexCount;
	float centerX, centerY, size;
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

void InitShapes() {

	// ������
	for (int i = 0; i < 3; ++i) {
		Shape shape5;
		shape5.startIndex = allVertices.size() / 3;
		shape5.centerX = dis_x(gen); shape5.centerY = dis_y(gen); shape5.size = dis_size(gen);
		float x = shape5.centerX; float y = shape5.centerY;	float size = shape5.size;

		// �߾� �ﰢ��: (0, +0.3), (-0.15, -0.3), (+0.15, -0.3)
		allVertices.push_back(x); allVertices.push_back(y + size * 2); allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));

		allVertices.push_back(x - size); allVertices.push_back(y - size * 2); allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));

		allVertices.push_back(x + size); allVertices.push_back(y - size * 2); allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));

		// ���� �ﰢ��: (-0.3, +0.05), (0, +0.3), (-0.15, -0.3)
		allVertices.push_back(x - size * 2); allVertices.push_back(y); allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));

		allVertices.push_back(x); allVertices.push_back(y + size * 2); allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));

		allVertices.push_back(x - size); allVertices.push_back(y - size * 2); allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));

		// ������ �ﰢ��: (+0.3, +0.05), (0, +0.3), (+0.15, -0.3)
		allVertices.push_back(x + size * 2); allVertices.push_back(y); allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));

		allVertices.push_back(x); allVertices.push_back(y + size * 2); allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));

		allVertices.push_back(x + size); allVertices.push_back(y - size * 2); allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));

		shape5.drawMode = GL_TRIANGLES;
		shape5.vertexCount = 9;
		shape5.shapeType = 5;
		shapes.push_back(shape5);
	}
	
	// �簢��
	for (int i = 0; i < 3; ++i) {
		Shape shape4;
		shape4.startIndex = allVertices.size() / 3;
		shape4.centerX = dis_x(gen); shape4.centerY = dis_y(gen); shape4.size = dis_size(gen);
		float x = shape4.centerX; float y = shape4.centerY;	float size = shape4.size;

		// �»��
		allVertices.push_back(x - size); allVertices.push_back(y + size); allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));

		// ���ϴ�
		allVertices.push_back(x - size); allVertices.push_back(y - size); allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));

		// ���ϴ�
		allVertices.push_back(x + size); allVertices.push_back(y - size); allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));

		// �»��
		allVertices.push_back(x - size); allVertices.push_back(y + size); allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));

		// ���ϴ�
		allVertices.push_back(x + size); allVertices.push_back(y - size); allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));

		// ����
		allVertices.push_back(x + size); allVertices.push_back(y + size); allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));

		shape4.drawMode = GL_TRIANGLES;
		shape4.vertexCount = 6;
		shape4.shapeType = 4;
		shapes.push_back(shape4);
	}
	
	// �ﰢ��
	for (int i = 0; i < 3; ++i) {
		Shape shape3;
		shape3.startIndex = allVertices.size() / 3;
		shape3.centerX = dis_x(gen); shape3.centerY = dis_y(gen); shape3.size = dis_size(gen);
		float x = shape3.centerX; float y = shape3.centerY;	float size = shape3.size;

		allVertices.push_back(x); allVertices.push_back(y + size * 2); allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));

		allVertices.push_back(x - size); allVertices.push_back(y - size); allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));

		allVertices.push_back(x + size); allVertices.push_back(y - size); allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));

		shape3.drawMode = GL_TRIANGLES;
		shape3.vertexCount = 3;
		shape3.shapeType = 3;
		shapes.push_back(shape3);
	}
	

	// ��
	for (int i = 0; i < 3; ++i) {
		Shape shape2;
		shape2.startIndex = allVertices.size() / 3;
		shape2.centerX = dis_x(gen); shape2.centerY = dis_y(gen);  shape2.size = dis_size(gen);
		float x = shape2.centerX;
		float y = shape2.centerY;
		float size = shape2.size;
		allVertices.push_back(x - size); allVertices.push_back(y - size); allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));
		allVertices.push_back(x + size); allVertices.push_back(y + size); allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));

		shape2.drawMode = GL_LINES;
		shape2.vertexCount = 2;
		shape2.shapeType = 2;
		shapes.push_back(shape2);
	}

	// �� (������ �ﰢ��)
	for (int i = 0; i < 3; ++i) {
		Shape shape1;
		shape1.startIndex = allVertices.size() / 3;
		shape1.centerX = dis_x(gen); shape1.centerY = dis_y(gen);  shape1.size = dis_size(gen);
		float x = shape1.centerX;
		float y = shape1.centerY;
		float size = shape1.size;

		allVertices.push_back(x); allVertices.push_back(y + 0.02f); allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));

		allVertices.push_back(x - 0.01f); allVertices.push_back(y - 0.01f); allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));

		allVertices.push_back(x + 0.01f); allVertices.push_back(y - 0.01f); allVertices.push_back(0.0f);
		allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen)); allColors.push_back(dis_color(gen));

		shape1.drawMode = GL_TRIANGLES;
		shape1.vertexCount = 3;
		shape1.shapeType = 1;
		shapes.push_back(shape1);
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