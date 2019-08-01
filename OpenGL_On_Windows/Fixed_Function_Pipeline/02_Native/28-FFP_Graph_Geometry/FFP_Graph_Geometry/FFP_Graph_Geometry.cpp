//Headerfiles
#include<windows.h>
#include<gl/GL.h>
#include<stdio.h>		// For file I/O operation, FILE structure is needed
#include<gl/GLU.h>
#define _USE_MATH_DEFINES 1
#include<math.h>
#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"glu32.lib")
#define WIN_WIDTH 800
#define WIN_HEIGHT 600


// Global function declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// Global varibale declarations

bool bFullScreen = false;
DWORD dwStyle;
WINDOWPLACEMENT wpPrev = { sizeof(WINDOWPLACEMENT) };
HWND ghwnd = NULL;

HDC ghdc = NULL;
HGLRC ghrc = NULL;	// global handle to rendering context

bool gbActiveWindow = false;

FILE *gpFile = NULL;
// Winmain()
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hprevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	//Local function declarations
	int initialize(void);
	void uninitialize(void);
	void display(void);		// this is for double buffering

	if (fopen_s(&gpFile, "Log.txt", "w") != 0)
	{
		MessageBox(NULL, TEXT("Log file cannot be created !!"), TEXT("Error"), MB_OK);
		exit(0);
	}
	else
	{
		fprintf(gpFile, "Log file created successfully\n");
	}

	// Local variable declarations
	WNDCLASSEX wndclass;
	TCHAR szAppName[] = TEXT("MyFullscreenWindow");
	HWND hwnd;
	MSG msg;

	bool bDone = false;
	int iRet = 0;


	// code
	// initialization of WNDCLASSEX
	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.style = CS_VREDRAW | CS_HREDRAW | CS_OWNDC;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.lpfnWndProc = WndProc;
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.lpszClassName = szAppName;
	wndclass.lpszMenuName = NULL;

	// Register above class

	RegisterClassEx(&wndclass);

	// Create Window

	hwnd = CreateWindowEx(WS_EX_APPWINDOW,
		szAppName,
		TEXT("FFP_All_Geometry_On_Graph!!"),
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
		100,
		100,
		WIN_WIDTH,
		WIN_HEIGHT,
		NULL,	//Take desktop window as parent
		NULL,	// No menu
		hInstance,	// current instance of window
		NULL);	//If you want to send something to WndProc from WinMain

	ghwnd = hwnd;

	iRet = initialize();

	if (iRet == -1)
	{
		fprintf(gpFile, "ChoosePixelFormat() failed !!\n");
		DestroyWindow(hwnd);
	}
	else if (iRet == -2)
	{
		fprintf(gpFile, "SetPixelFormat() failed !!\n");
		DestroyWindow(hwnd);
	}
	else if (iRet == -3)
	{
		fprintf(gpFile, "wglCreateContext() failed !!\n");
		DestroyWindow(hwnd);
	}
	else if (iRet == -4)
	{
		fprintf(gpFile, "wglmakeCurrent() failed !!\n");
		DestroyWindow(hwnd);
	}
	else
	{
		fprintf(gpFile, "Initialization successful !!!\n");
	}

	ShowWindow(hwnd, iCmdShow);
	//UpdateWindow(hwnd);
	SetForegroundWindow(hwnd);
	SetFocus(hwnd);

	// Game Loop

	while (bDone == false)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				bDone = true;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			if (gbActiveWindow == true)
			{
				// Here call to Update() ==> Animation
			}
			display();
			// Here call to Display(); though for this application we are calling in WM_PAINT
		}
	}

	return (int)msg.wParam;
}

// Call back function body

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	// local function declarations
	void resize(int, int);
	//void display(void);		// Removed for double buffering
	void uninitialize(void);
	void ToggleFullscreen(void);
	//	Code
	switch (iMsg)
	{
	case WM_SETFOCUS:
		gbActiveWindow = true;
		break;
	case WM_KILLFOCUS:
		gbActiveWindow = false;
		break;
	case WM_SIZE:
		resize(LOWORD(lParam), HIWORD(lParam));
		break;
		//case WM_PAINT:
		//	display();	// This should be done only in single buffering
		//	break;
	case WM_ERASEBKGND:
		return 0;	// Don't go to DefWindowProc(), I have external painter, I don't want your painter
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE:
			//MessageBox(hwnd, TEXT("Exiting on Escape Key !!"), TEXT("VK_ESCAPE handled"), MB_OK);
			DestroyWindow(hwnd);
			break;
		case 0x46:
			ToggleFullscreen();
			break;
		}
		break;
	case WM_DESTROY:
		uninitialize();
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hwnd, iMsg, wParam, lParam);
}


void ToggleFullscreen(void)
{
	MONITORINFO mi;

	if (bFullScreen == false)
	{
		dwStyle = GetWindowLong(ghwnd, GWL_STYLE);

		if (dwStyle & WS_OVERLAPPEDWINDOW)
		{
			mi = { sizeof(MONITORINFO) };

			if (GetWindowPlacement(ghwnd, &wpPrev) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY
			), &mi))
			{
				SetWindowLong(ghwnd, GWL_STYLE,
					dwStyle & ~WS_OVERLAPPEDWINDOW);

				SetWindowPos(ghwnd,
					HWND_TOP,
					mi.rcMonitor.left,
					mi.rcMonitor.top,
					mi.rcMonitor.right - mi.rcMonitor.left,
					mi.rcMonitor.bottom - mi.rcMonitor.top,
					SWP_NOZORDER | SWP_FRAMECHANGED);
			}
		}
		ShowCursor(FALSE);
		bFullScreen = true;
	}
	else
	{
		SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);

		SetWindowPlacement(ghwnd, &wpPrev);

		SetWindowPos(ghwnd,
			HWND_TOP,
			0,
			0,
			0,
			0,
			SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER);

		ShowCursor(TRUE);
		bFullScreen = false;
	}
}

int initialize(void)
{
	// Function declaration
	void resize(int, int);

	// Variable declaration

	PIXELFORMATDESCRIPTOR pfd;
	int iPixelFormatIndex;

	// Code
	// Initialize pfd structure
	ZeroMemory(&pfd, sizeof(PIXELFORMATDESCRIPTOR));

	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;	// This is openGL version supported by window.
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cRedBits = 8;
	pfd.cGreenBits = 8;
	pfd.cBlueBits = 8;
	pfd.cAlphaBits = 8;

	ghdc = GetDC(ghwnd);
	iPixelFormatIndex = ChoosePixelFormat(ghdc, &pfd);
	if (iPixelFormatIndex == 0)
	{
		return -1;
	}
	if (SetPixelFormat(ghdc, iPixelFormatIndex, &pfd) == FALSE)
	{
		return -2;
	}

	ghrc = wglCreateContext(ghdc);

	if (ghrc == NULL)
	{
		return -3;
	}
	if (wglMakeCurrent(ghdc, ghrc) == FALSE)
	{
		return -4;
	}
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	// warm-up call to resize()
	resize(WIN_WIDTH, WIN_HEIGHT);		// for size dependent resources to be adjusted according window
	return 0;
}

void resize(int width, int height)
{
	if (height == 0)
		height = 1;
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
}

void display(void)
{
	//Function declaration
	void DrawGraph(void);
	void DrawRectangle(void);
	void DrawCircle(void);
	void DrawTriangle(void);
	void DrawIncircle(void);

	//Code
	glClear(GL_COLOR_BUFFER_BIT);
	//glFlush();	// Single Buffer

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -1.3f);
	DrawGraph();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -3.0f);
	
	glLineWidth(3.0f);
	glBegin(GL_LINE_LOOP);
	glColor3f(1.0f, 1.0f, 0.0f);
	DrawRectangle();
	glEnd();


	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -3.0f);

	glLineWidth(3.0f);
	glBegin(GL_LINE_LOOP);
	glColor3f(1.0f, 1.0f, 0.0f);
	DrawTriangle();
	glEnd();

	glLineWidth(3.0f);
	glBegin(GL_LINE_LOOP);
	glColor3f(1.0f, 1.0f, 0.0f);
	DrawIncircle();
	glEnd();

	glLineWidth(3.0f);
	glBegin(GL_LINE_LOOP);
	glColor3f(1.0f, 1.0f, 0.0f);
	DrawCircle();
	glEnd();

	SwapBuffers(ghdc);
}

void DrawGraph(void)
{
	float i;

	// Horizontal lines ---- Parallel to X-Axis
	glLineWidth(1.0f);
	glBegin(GL_LINES);
	for (i = 1.0f; i > -1.0f; i = i - 0.05f)
	{
		glColor3f(1.0f, 0.0f, 0.0f);
		glVertex2f(-1.0f, i);
		glColor3f(1.0f, 0.0f, 0.0f);
		glVertex2f(1.0f, i);
	}

	glEnd();

	// Vertical lines ---- Parallel to Y-Axis
	glLineWidth(1.0f);
	glBegin(GL_LINES);
	for (i = -1.0f; i < 1.0f; i = i + 0.05f)
	{
		glColor3f(1.0f, 0.0f, 0.0f);
		glVertex2f(i, 1.0f);
		glColor3f(1.0f, 0.0f, 0.0f);
		glVertex2f(i, -1.0f);
	}

	glEnd();

	// For X-Axis
	glLineWidth(3.0f);
	glBegin(GL_LINES);
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex2f(-1.0f, 0.0f);
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex2f(1.0f, 0.0f);

	glEnd();


	// For Y-Axis
	glLineWidth(3.0f);
	glBegin(GL_LINES);

	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex2f(0.0f, 1.0f);
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex2f(0.0f, -1.0f);

	glEnd();
}


void DrawRectangle(void)
{
	//Code
	glVertex3f(1.0f, 1.0f, 0.0f);
	
	glVertex3f(-1.0f, 1.0f, 0.0f);

	glVertex3f(-1.0f, -1.0f, 0.0f);

	glVertex3f(1.0f, -1.0f, 0.0f);
}

void DrawTriangle(void)
{
	//Code
	glVertex3f(0.0f, 1.0f, 0.0f);
		
	glVertex3f(-1.0f, -1.0f, 0.0f);

	glVertex3f(1.0f, -1.0f, 0.0f);

}

void DrawIncircle()
{
	//Variable declaration
	double dSideRight, dSideLeft, dSideBase;
	double Perimeter, AreaOfTriangle;
	double Ox, Oy;
	double RadiusOfIncircle;
	int NumberOfPoints = 10000;

	//Code
	dSideRight = sqrt(pow((-1 - 0), 2) + pow((-1 - 1), 2));		// Right Side of triangle using Distance formula 

	dSideLeft = sqrt(pow((1 - 0), 2) + pow((-1 - 1), 2));		// Left Side of triangle using Distance formula

	dSideBase = sqrt(pow((1 - (-1)), 2) + pow((-1 - (-1)), 2));	// Base of triangle using Distance formula

	Perimeter = (dSideRight + dSideLeft + dSideBase);		// Perimeter of Triangle
	double SemiPerimeter = Perimeter / 2;
	AreaOfTriangle = 0.5 * 2.0 * 1.0;						// Area of Triangle

	RadiusOfIncircle = (2 * AreaOfTriangle) / SemiPerimeter;		// Radius of Circle..Formula

	Ox = ((dSideBase * 0.0f) + (dSideRight * -1.0f) + (dSideLeft * 1.0f)) / Perimeter;		// Origin X.....Formula 

	Oy = ((dSideBase * 1.0f) + (dSideRight * -1.0f) + (dSideLeft * -1.0f)) / Perimeter;		// Origin Y.....Formula


	for (float angle = 0.0f; angle < 2.0f*M_PI; angle = angle + 0.001f)
	{

		glVertex3f(RadiusOfIncircle * cos(angle) + Ox, RadiusOfIncircle * sin(angle) + Oy, 0.0f);
	}
}

void DrawCircle(void)
{
	for (float angle = 0.0f; angle < 2.0f*M_PI; angle = angle + 0.001f)
	{
		glVertex3f(cos(angle),sin(angle), 0.0f);
	}
}

void uninitialize(void)
{
	if (bFullScreen == true)
	{
		SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(ghwnd, &wpPrev);
		SetWindowPos(ghwnd,
			HWND_TOP,
			0,
			0,
			0,
			0,
			SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER);
		ShowCursor(TRUE);
		//bFullScreen = false;
	}

	if (wglGetCurrentContext() == ghrc)
	{
		wglMakeCurrent(NULL, NULL);
	}

	if (ghrc)
	{
		wglDeleteContext(ghrc);
		ghrc = NULL;
	}

	if (ghdc)
	{
		ReleaseDC(ghwnd, ghdc);
		ghdc = NULL;
	}

	if (gpFile)
	{
		fprintf(gpFile, "Log file closed successfully !!\n");
		fclose(gpFile);
		gpFile = NULL;
	}
}
