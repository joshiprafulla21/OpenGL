//Headerfiles
#include<windows.h>
#include"Template.h"
#include<gl/GL.h>
#include<stdio.h>		// For file I/O operation, FILE structure is needed
#include<gl/GLU.h>
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

GLdouble pyramidAngle = 0.0;		// angle for rotation of pyramid about Y axis i.e., Spinning
GLdouble cubeAngle = 0.0;			// angle for arbitary rotation of cube 

GLuint texture_stone;
GLuint texture_kundali;

// Winmain()
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hprevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	//Local function declarations
	int initialize(void);
	void uninitialize(void);
	void display(void);		// this is for double buffering
	void update(void);

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
		TEXT("FFP_Pyramid_Spinning!!"),
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
				update();		// Here call to Update() ==> Animation
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
	BOOL loadTexture(GLuint*, TCHAR[]);

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
	pfd.cDepthBits = 32;	// Take all bits of dept from PFD structure..for 3D ONLY

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

	/*------BEAUTIFICATION CALLS IN 3D--------*/

	glShadeModel(GL_SMOOTH);	// Use smooth lighting model as max as possible (using INTERPOLATION Technique)..this is due some drawbacks of perspective

	/*------BEAUTIFICATION CALLS IN 3D ENDS HERE--------*/

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	/*------FOLLOWING CHANGES ARE 3D SPECIFIC--------*/

	glClearDepth(1.0f);				// To give existence to depth buffer.. 1.0 is the maximum number filled up in depth buffer
	glEnable(GL_DEPTH_TEST);		//  To enable despth test	
	glDepthFunc(GL_LEQUAL);			// Which test to be done is specified here

	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);		// Give hint of where to use smooth lighting model

	/*---------3D SPECIFIC CALLS ENDS HERE-------------*/

	glEnable(GL_TEXTURE_2D);
	loadTexture(&texture_stone, MAKEINTRESOURCE(IDBITMAP_STONE));
	loadTexture(&texture_kundali, MAKEINTRESOURCE(IDBITMAP_KUNDALI));

	resize(WIN_WIDTH, WIN_HEIGHT);		// warm-up call to resize() for size dependent resources to be adjusted according window
	ToggleFullscreen();
	return 0;
}


BOOL loadTexture(GLuint *texture, TCHAR imageResourceID[])
{
	//Variable declaration
	HBITMAP hBitmap = NULL;
	BITMAP bmp;
	BOOL bStatus = FALSE;

	//Code

	hBitmap = (HBITMAP)LoadImage(GetModuleHandle(NULL),
		imageResourceID,
		IMAGE_BITMAP,
		0,
		0,
		LR_CREATEDIBSECTION);

	if (hBitmap)
	{
		bStatus = TRUE;
		GetObject(hBitmap, sizeof(BITMAP), &bmp);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
		glGenTextures(1, texture);
		glBindTexture(GL_TEXTURE_2D, *texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		gluBuild2DMipmaps(GL_TEXTURE_2D,
			3,
			bmp.bmWidth,
			bmp.bmHeight,
			GL_BGR_EXT,
			GL_UNSIGNED_BYTE,
			bmp.bmBits);
		DeleteObject(hBitmap);
	}
	return bStatus;
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


void update(void)
{
	//code
	pyramidAngle = pyramidAngle + 0.1;
	if (pyramidAngle >= 360.0)
		pyramidAngle = 0.0;

	cubeAngle = cubeAngle + 0.1;
	if (cubeAngle >= 360.0)
		cubeAngle = 0.0;
}

void display(void)
{
	//Local functions declarations
	void DrawPyramid(void);
	void Draw_Cube(void);

	//Code
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// To make depth buffer functional; here actually depth buffer is filled up with 1.0
	// For Pyramid
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(-1.5f, 0.0f, -6.0f);
	glRotatef(pyramidAngle, 0.0f, 1.0f, 0.0f);
	glBindTexture(GL_TEXTURE_2D, texture_stone);
	glBegin(GL_TRIANGLES);
	DrawPyramid();
	glEnd();

	// For Cube
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(1.5f, 0.0f, -6.0f);
	glScalef(0.75f, 0.75f, 0.75f);
	glRotatef(cubeAngle, 1.0f, 1.0f, 1.0f);
	glBindTexture(GL_TEXTURE_2D, texture_kundali);
	glBegin(GL_QUADS);
	Draw_Cube();
	glEnd();

	SwapBuffers(ghdc);
}


void DrawPyramid(void)
{
	/* PYRAMID WILL HAVE 4 TRIANGUALR FACES..FOLLOWING ARE THE CO-ORIDNATES FOR EACH FACE */

	// FRONT FACE
	//glColor3f(1.0f, 0.0f, 0.0f);		// RED
	glTexCoord2f(0.5f, 1.0f);
	glVertex3f(0.0f, 1.0f, 0.0f);

	//glColor3f(0.0f, 1.0f, 0.0f);		// GREEN
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);

	//glColor3f(0.0f, 0.0f, 1.0f);		// BLUE
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);

	// RIGHT FACE
	//glColor3f(1.0f, 0.0f, 0.0f);		// RED
	glTexCoord2f(0.5f, 1.0f);
	glVertex3f(0.0f, 1.0f, 0.0f);

	//glColor3f(0.0f, 0.0f, 1.0f);		// BLUE
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);

	//glColor3f(0.0f, 1.0f, 0.0f);		// GREEN
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);

	// BACK FACE
	//glColor3f(1.0f, 0.0f, 0.0f);		// RED
	glTexCoord2f(0.5f, 1.0f);
	glVertex3f(0.0f, 1.0f, 0.0f);

	//glColor3f(0.0f, 1.0f, 0.0f);		// GREEN
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);

	//glColor3f(0.0f, 0.0f, 1.0f);		// BLUE
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);

	// LEFT FACE
	//glColor3f(1.0f, 0.0f, 0.0f);		// RED
	glTexCoord2f(0.5f, 1.0f);
	glVertex3f(0.0f, 1.0f, 0.0f);

	//glColor3f(0.0f, 0.0f, 1.0f);		// BLUE
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);

	//glColor3f(0.0f, 1.0f, 0.0f);		// GREEN
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);
}


void Draw_Cube(void)
{
	/*Cube has total 6 surfaces. We will give 3D co-ordinates to each surface as following */

	//Top Surface
	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(1.0f, 1.0f, -1.0f);

	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-1.0f, 1.0f, -1.0f);

	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(-1.0f, 1.0f, 1.0f);

	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(1.0f, 1.0f, 1.0f);


	//Bottom Surface
	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);

	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);

	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);

	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);

	// Front Surface
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(1.0f, 1.0f, 1.0f);

	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(-1.0f, 1.0f, 1.0f);

	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);

	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);


	// Right Surface
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(1.0f, 1.0f, -1.0f);

	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(1.0f, 1.0f, 1.0f);

	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);

	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);


	// Back Surface
	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(-1.0f, 1.0f, -1.0f);

	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(1.0f, 1.0f, -1.0f);

	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);

	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);


	// Left Surface
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-1.0f, 1.0f, 1.0f);

	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(-1.0f, 1.0f, -1.0f);

	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);

	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);
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
	glDeleteTextures(1, &texture_stone);
}
