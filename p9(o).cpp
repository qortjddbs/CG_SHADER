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
std::uniform_real_distribution<float> dis_x(0.1f, 0.9f); // -0.1f ���� 0.1f ���� �յ��ϰ� ��Ÿ���� �������� �����ϱ� ���� �յ� ���� ����.
std::uniform_real_distribution<float> dis_y(0.1f, 0.7f); // -0.1f ���� 0.1f ���� �յ��ϰ� ��Ÿ���� �������� �����ϱ� ���� �յ� ���� ����.
std::uniform_real_distribution<float> dis_size(0.05f, 0.15f); // �ﰢ�� ũ��� ���� ����

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
	LineX.shapeType = 1; // �� Ÿ��

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
	LineY.shapeType = 1; // �� Ÿ��

	shapes.push_back(LineY);
	UpdateBuffer();

	// 1��и�
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
	newShape1.shapeType = 2; // �ﰢ�� Ÿ��

	shapes.push_back(newShape1);
	UpdateBuffer();

	// 2��и�
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
	newShape2.shapeType = 2; // �ﰢ�� Ÿ��

	shapes.push_back(newShape2);
	UpdateBuffer();

	// 3��и�
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
	newShape3.shapeType = 2; // �ﰢ�� Ÿ��

	shapes.push_back(newShape3);
	UpdateBuffer();

	// 4��и�
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
	newShape4.shapeType = 2; // �ﰢ�� Ÿ��

	shapes.push_back(newShape4);
	UpdateBuffer();
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
    
    // ���ο� �����ͷ� ��ü
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
		InitShapes();
		break;
	case 'q':
		exit(0);
		break;
	}
	glutPostRedisplay(); //--- ������ �ٲ� ������ ��� �ݹ� �Լ��� ȣ���Ͽ� ȭ���� refresh �Ѵ�
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