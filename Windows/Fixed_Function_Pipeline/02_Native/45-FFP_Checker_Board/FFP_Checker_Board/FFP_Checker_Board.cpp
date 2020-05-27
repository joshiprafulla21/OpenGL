//Headerfiles
#include<windows.h>
#include<gl/GL.h>
#include<stdio.h>		// For file I/O operation, FILE structure is needed
#include<gl/GLU.h>
#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"glu32.lib")
#define WIN_WIDTH 800
#define WIN_HEIGHT 600
#define CHECK_IMAGE_WIDTH 64
#define CHECK_IMAGE_HEIGHT 64


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

GLubyte check_image[CHECK_IMAGE_WIDTH][CHECK_IMAGE_HEIGHT][4];
GLuint TexImage;
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
		TEXT("FFP_Checker_Board !!"),
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
	case WM_ERASEBKGND:
		return 0;	// Don't go to DefWindowProc(), I have external painter, I don't want your painter
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE:
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
	void loadTexture(void);
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

	glEnable(GL_TEXTURE_2D);
	loadTexture();

	// warm-up call to resize()
	resize(WIN_WIDTH, WIN_HEIGHT);		// for size dependent resources to be adjusted according window
	ToggleFullscreen();
	return 0;
}

void loadTexture()
{
	//Function declaration
	void MakeCheckImage(void);
	fprintf(gpFile, "loadTexture invoked !!\n");
	//Code
	MakeCheckImage();
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &TexImage);
	glBindTexture(GL_TEXTURE_2D, TexImage);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D,
		0,
		GL_RGBA,
		CHECK_IMAGE_WIDTH,
		CHECK_IMAGE_HEIGHT,
		0,
		GL_RGBA,
		GL_UNSIGNED_BYTE,
		check_image);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}


void MakeCheckImage()
{
	//Variable declarations
	int i, j, c;

	fprintf(gpFile, "MakeCheckImage invoked !!\n");
	//Code
	for ( i = 0; i < CHECK_IMAGE_HEIGHT; i++)
	{
		for ( j = 0; j < CHECK_IMAGE_WIDTH; j++)
		{
			c = (((i & 0x8) == 0) ^ ((j & 0x8) == 0)) * 255;
			check_image[i][j][0] = (GLubyte)c;
			check_image[i][j][1] = (GLubyte)c;
			check_image[i][j][2] = (GLubyte)c;
			check_image[i][j][3] = 255;
		}
	}
}

void resize(int width, int height)
{
	if (height == 0)
		height = 1;
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0f, (GLfloat)width / (GLfloat)height, 1.0f, 30.0f);
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0f, 0.0f, -3.6f);
	
	//glBindTexture(GL_TEXTURE_2D, TexImage);
	glBegin(GL_QUADS);
	
	/*REDBOOK EXPECTS VERTICES TO COUNT FROM LEFT BOTTOM TO RIGHT BOTTOM CLOCKWISE FOR THIS EXAMPLE*/
	
	//1st Quad
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(-2.0f, -1.0f, 0.0f);

	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(-2.0f, 1.0f, 0.0f);

	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(0.0f, 1.0f, 0.0f);

	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(0.0f, -1.0f, 0.0f);
	//glEnd();

	//2nd Quad
	
	glTexCoord2f(0.0f, 0.0f);
	glVertex3f(1.0f, -1.0f, 0.0f);

	glTexCoord2f(0.0f, 1.0f);
	glVertex3f(1.0f, 1.0f, 0.0f);

	glTexCoord2f(1.0f, 1.0f);
	glVertex3f(2.41421f, 1.0f, -1.41421f);

	glTexCoord2f(1.0f, 0.0f);
	glVertex3f(2.41421f, -1.0f, -1.41421f);

	glEnd();

	SwapBuffers(ghdc);
	//glFlush();
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
	glBindTexture(GL_TEXTURE_2D, 0);
	glDeleteTextures(1, &TexImage);
}
