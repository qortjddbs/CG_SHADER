#define _CRT_SECURE_NO_WARNINGS //--- 프로그램 맨 앞에 선언할 것
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
//--- 필요한 헤더파일 선언
#include <gl/glew.h>
#include <gl/freeglut.h>
#include <gl/freeglut_ext.h>
#include <random>
#include <vector>
#include <cmath>
//--- 아래 5개 함수는 사용자 정의 함수임
void make_vertexShaders();
void make_fragmentShaders();
GLuint make_shaderProgram();
GLvoid drawScene();
GLvoid Reshape(int w, int h);
GLvoid TimerFunction(int value);
//--- 필요한 변수 선언
GLint width, height;
GLuint shaderProgramID; //--- 세이더 프로그램 이름
GLuint vertexShader; //--- 버텍스 세이더 객체
GLuint fragmentShader; //--- 프래그먼트 세이더 객체

std::random_device rd;  // 시드값을 얻기 위한 random_device 생성.
std::mt19937 gen(rd());	// random_device 를 통해 난수 생성 엔진을 초기화 한다.
std::uniform_real_distribution<float> dis_color(0.0f, 1.0f); // 0.0f 부터 1.0f 까지 균등하게 나타나는 난수열을 생성하기 위해 균등 분포 정의.
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
int bigSpiralDirection = 0, smallSpiralDirection = 0;		// 0 : 왼쪽, 1 : 아래, 2 : 오른쪽, 3 : 위
int bigStepsInDirection = 0, smallStepsInDirection = 0;	// 현재 방향으로 이동한 스텝 수
int bigStepsToMove = 1, smallStepsToMove = 1;		// 현재 방향으로 이동해야 할 총 스텝 수
float bigStepSize = 0.02f, smallStepSize = 0.015f;	// 한 스텝당 이동거리

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

void InitShapes() {	// x,y 축하고 각 사분면마다 삼각형 하나씩
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

//--- 그리기 콜백 함수
GLvoid drawScene()
{
	//--- 변경된 배경색 설정
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//--- 렌더링 파이프라인에 세이더 불러오기

	if (allVertices.empty()) {
		glutSwapBuffers();
		return;
	}

	//--- 렌더링 파이프라인에 세이더 불러오기
	glUseProgram(shaderProgramID);
	//--- 사용할 VAO 불러오기
	glBindVertexArray(vao);

	// 점 크기 설정
	glPointSize(10.0f);

	// 각 도형별로 그리기
	for (const auto& shape : shapes) {
		glDrawArrays(GL_TRIANGLES, shape.startIndex, shape.vertexCount);
	}

	glutSwapBuffers(); //--- 화면에 출력하기
}

GLvoid Keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case '1':		// 대각선 이동
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
	glutPostRedisplay(); //--- 배경색이 바뀔 때마다 출력 콜백 함수를 호출하여 화면을 refresh 한다
}

GLvoid TimerFunction(int value) {
	if (!num1 && !num2 && !num3 && !num4) {
		timerRunning = false;
		return;
	}

	Shape& big = shapes[0];
	Shape& small = shapes[1];
	
	if (num1) {		// 대각선 이동
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

		// x 값 변경
		for (int i = 0; i < 3; ++i) {
			allVertices[(big.startIndex + i) * 3] += bigDirX;
			allVertices[(small.startIndex + i) * 3] += smallDirX;
		}

		// y값 변경
		for (int i = 0; i < 3; ++i) {
			allVertices[(big.startIndex + i) * 3 + 1] += bigDirY;
			allVertices[(small.startIndex + i) * 3 + 1] += smallDirY;
		}

		big.centerX += bigDirX;
		small.centerX += smallDirX;
		big.centerY += bigDirY;
		small.centerY += smallDirY;
	}
	else if (num2) {		// 지그재그 이동
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

		// x 값 변경
		for (int i = 0; i < 3; ++i) {
			allVertices[(big.startIndex + i) * 3] += bigDirX;
			allVertices[(small.startIndex + i) * 3] += smallDirX;
		}

		big.centerX += bigDirX;
		small.centerX += smallDirX;
	}
	else if (num3) {		// 사각 스파이럴 이동
		float bigDeltaX = 0.0f, bigDeltaY = 0.0f;
		switch (bigSpiralDirection) {
		case 0: bigDeltaX = bigStepSize; break;	// 오른쪽
		case 1: bigDeltaY = -bigStepSize; break;	// 아래쪽
		case 2: bigDeltaX = -bigStepSize; break;		// 왼쪽
		case 3: bigDeltaY = bigStepSize; break;		// 위쪽
		}

		float smallDeltaX = 0.0f, smallDeltaY = 0.0f;
		switch (smallSpiralDirection) {
		case 0: smallDeltaX = smallStepSize; break;	// 오른쪽
		case 1: smallDeltaY = -smallStepSize; break;	// 아래쪽
		case 2: smallDeltaX = -smallStepSize; break;		// 왼쪽
		case 3: smallDeltaY = smallStepSize; break;		// 위쪽
		}

		// 모든 정점 이동
		for (int i = 0; i < 3; ++i) {
			allVertices[(big.startIndex + i) * 3] += bigDeltaX;
			allVertices[(big.startIndex + i) * 3 + 1] += bigDeltaY;
			allVertices[(small.startIndex + i) * 3] += smallDeltaX;
			allVertices[(small.startIndex + i) * 3 + 1] += smallDeltaY;
		}

		// 중심점 업데이트
		big.centerX += bigDeltaX;
		big.centerY += bigDeltaY;
		small.centerX += smallDeltaX;
		small.centerY += smallDeltaY;

		// 큰 삼각형 방향 관리
		bigStepsInDirection++;
		if (bigStepsInDirection >= bigStepsToMove) {
			bigStepsInDirection = 0;
			bigSpiralDirection = (bigSpiralDirection + 1) % 4;

			// 오른쪽이나 왼쪽 방향이 끝나면 이동 거리 증가
			if (bigSpiralDirection == 2 || bigSpiralDirection == 0) {
				bigStepsToMove++;
			}
		}

		// 작은 삼각형 방향 관리 (다른 속도로)
		smallStepsInDirection++;
		if (smallStepsInDirection >= smallStepsToMove) {
			smallStepsInDirection = 0;
			smallSpiralDirection = (smallSpiralDirection + 1) % 4;

			// 오른쪽이나 왼쪽 방향이 끝나면 이동 거리 증가
			if (smallSpiralDirection == 2 || smallSpiralDirection == 0) {
				smallStepsToMove++;
			}
		}

		// 화면 경계를 벗어나면 리셋
		if (abs(big.centerX - bigCenterX) > 0.8f || abs(big.centerY - bigCenterY) > 0.8f) {
			bigSpiralDirection = 0;
			bigStepsInDirection = 0;
			bigStepsToMove = 1;
			big.centerX = bigCenterX;
			big.centerY = bigCenterY;

			// 정점들도 중심으로 리셋
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

			// 정점들도 중심으로 리셋
			allVertices[small.startIndex * 3] = smallCenterX;
			allVertices[small.startIndex * 3 + 1] = smallCenterY + 0.15f;
			allVertices[(small.startIndex + 1) * 3] = smallCenterX - 0.05f;
			allVertices[(small.startIndex + 1) * 3 + 1] = smallCenterY - 0.05f;
			allVertices[(small.startIndex + 2) * 3] = smallCenterX + 0.05f;
			allVertices[(small.startIndex + 2) * 3 + 1] = smallCenterY - 0.05f;
		}
	}
	else if (num4) {		// 원 스파이럴 이동
		// 1. 각도를 점진적으로 증가 (회전 효과)
		bigAngle += 0.1f;     // += 사용으로 누적 증가
		smallAngle += 0.15f;  // 작은 삼각형은 더 빠르게 회전

		// 2. 반지름을 점진적으로 증가 (스파이럴 효과)
		bigRadius += 0.002f;    // 눈에 보이는 정도로 증가
		smallRadius += 0.001f;

		// 3. 새로운 위치 계산 (극좌표 -> 직교좌표 변환)
		float newBigCenterX = bigCenterX + bigRadius * cos(bigAngle);
		float newBigCenterY = bigCenterY + bigRadius * sin(bigAngle);
		float newSmallCenterX = smallCenterX + smallRadius * cos(smallAngle);
		float newSmallCenterY = smallCenterY + smallRadius * sin(smallAngle);

		// 4. 이동할 거리 계산
		float bigDeltaX = newBigCenterX - big.centerX;
		float bigDeltaY = newBigCenterY - big.centerY;
		float smallDeltaX = newSmallCenterX - small.centerX;
		float smallDeltaY = newSmallCenterY - small.centerY;

		// 5. 모든 정점을 새로운 위치로 이동
		for (int i = 0; i < 3; ++i) {
			allVertices[(big.startIndex + i) * 3] += bigDeltaX;
			allVertices[(big.startIndex + i) * 3 + 1] += bigDeltaY;
			allVertices[(small.startIndex + i) * 3] += smallDeltaX;
			allVertices[(small.startIndex + i) * 3 + 1] += smallDeltaY;
		}

		// 6. 중심점 업데이트 (올바른 할당)
		big.centerX = newBigCenterX;
		big.centerY = newBigCenterY;
		small.centerX = newSmallCenterX;
		small.centerY = newSmallCenterY;

		// 7. 화면 경계를 벗어나면 리셋
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
	glutTimerFunc(16, TimerFunction, 1); // 다음 프레임을 위한 타이머 설정
}

//--- 다시그리기 콜백 함수
GLvoid Reshape(int w, int h)
{
	glViewport(0, 0, w, h);
}

//--- 버텍스 세이더 객체 만들기
void make_vertexShaders()
{
	GLchar* vertexSource;
	//--- 버텍스 세이더 읽어 저장하고 컴파일 하기
	//--- filetobuf: 사용자정의 함수로 텍스트를 읽어서 문자열에 저장하는 함수
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
		std::cerr << "ERROR: vertex shader 컴파일 실패\n" << errorLog << std::endl;
		free(vertexSource); // 셰이더 함수들에서 메모리 해제
		return;
	}
	free(vertexSource); // 셰이더 함수들에서 메모리 해제
}

//--- 프래그먼트 세이더 객체 만들기
void make_fragmentShaders()
{
	GLchar* fragmentSource;
	//--- 프래그먼트 세이더 읽어 저장하고 컴파일하기
	fragmentSource = filetobuf("fragment.glsl"); // 프래그세이더 읽어오기
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);
	GLint result;
	GLchar errorLog[512];
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, errorLog);
		std::cerr << "ERROR: frag_shader 컴파일 실패\n" << errorLog << std::endl;
		free(fragmentSource); // 셰이더 함수들에서 메모리 해제
		return;
	}
	free(fragmentSource); // 셰이더 함수들에서 메모리 해제
}

void InitBuffer()
{
	glGenVertexArrays(1, &vao); //--- VAO 를 지정하고 할당하기
	glBindVertexArray(vao); //--- VAO를 바인드하기
	glGenBuffers(2, vbo); //--- 2개의 VBO를 지정하고 할당하기

	// 초기에는 빈 버퍼로 설정
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
	glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
}

void main(int argc, char** argv) //--- 윈도우 출력하고 콜백함수 설정
{
	//--- 윈도우 생성하기
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Practice 8");
	//--- GLEW 초기화하기
	glewExperimental = GL_TRUE;
	glewInit();
	shaderProgramID = make_shaderProgram();
	InitBuffer();
	glutDisplayFunc(drawScene);
	glutReshapeFunc(Reshape);
	InitShapes();
	glutKeyboardFunc(Keyboard); //--- 키보드 입력 콜백함수 지정
	glutMainLoop(); // 이벤트 처리 시작
}

//--- 세이더 프로그램 만들고 세이더 객체 링크하기
GLuint make_shaderProgram()
{
	// 먼저 셰이더들을 컴파일
	make_vertexShaders();
	make_fragmentShaders();

	GLint result;
	GLchar errorLog[512];
	GLuint shaderID;

	shaderID = glCreateProgram(); //--- 세이더 프로그램 만들기
	glAttachShader(shaderID, vertexShader); //--- 세이더 프로그램에 버텍스 세이더 붙이기
	glAttachShader(shaderID, fragmentShader); //--- 세이더 프로그램에 프래그먼트 세이더 붙이기
	glLinkProgram(shaderID); //--- 세이더 프로그램 링크하기

	glDeleteShader(vertexShader); //--- 세이더 객체를 세이더 프로그램에 링크했음으로, 세이더 객체 자체는 삭제 가능
	glDeleteShader(fragmentShader);

	glGetProgramiv(shaderID, GL_LINK_STATUS, &result); // ---세이더가 잘 연결되었는지 체크하기
	if (!result) {
		glGetProgramInfoLog(shaderID, 512, NULL, errorLog);
		std::cerr << "ERROR: shader program 연결 실패\n" << errorLog << std::endl;
		return 0;
	}

	return shaderID;
}