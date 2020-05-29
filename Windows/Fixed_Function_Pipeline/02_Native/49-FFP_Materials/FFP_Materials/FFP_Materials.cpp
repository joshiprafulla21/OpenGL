//Headerfiles
#include<windows.h>
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
GLfloat angle = 0.0f;

// Following parameters are spesic for light
bool bLight = false;
GLfloat LightAmbient[] = { 0.0f,0.0f,0.0f,0.0f };
GLfloat LightDiffuse[] = { 1.0f,1.0f,1.0f,1.0f };
//GLfloat LightSpecular[] = { 1.0f,1.0f,1.0f,1.0f };	
GLfloat LightPosition[] = { 0.0f,0.0f,0.0f,1.0f };

/*GLfloat MaterialAmbient[] = { 0.0f,0.0f,0.0f,1.0f };
GLfloat MaterialDiffuse[] = { 1.0f,1.0f,1.0f,1.0f };
GLfloat MaterialSpecular[] = { 1.0f,1.0f,1.0f,1.0f };
GLfloat MaterialShininess[] = { 128.0f };*/

GLfloat light_model_ambient[] = { 0.2f,0.2f,0.2f,1.0f };
GLfloat light_model_local_viewer[] = { 0.0f };

GLUquadric *quadric[24];

GLfloat angleOfXRotation = 0.0f;
GLfloat angleOfYRotation = 0.0f;
GLfloat angleOfZRotation = 0.0f;
GLint keyPressed = 0;


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
		TEXT("FFP- Material--24 Spheres!!"),
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
			DestroyWindow(hwnd);
			break;
		case 0x46:
			ToggleFullscreen();
			break;
		case 'L':
		case 'l':
			if (bLight == false)
			{
				bLight = true;
				glEnable(GL_LIGHTING);
			}
			else
			{
				bLight = false;
				glDisable(GL_LIGHTING);
			}
			break;
		case 'X':
		case 'x':
			keyPressed = 1;
			angleOfXRotation = 0.0f;
			break;
		case 'Y':
		case 'y':
			keyPressed = 2;
			angleOfYRotation = 0.0f;
			break;
		case 'Z':
		case 'z':
			keyPressed = 3;
			angleOfZRotation = 0.0f;
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
	pfd.cDepthBits = 32;
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
	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);		// To enable depth test
	glDepthFunc(GL_LEQUAL);		// Compare z value of each object with 1.0 which is the max and show only those objects whose depth value is less than or equal to 1.0
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	glEnable(GL_AUTO_NORMAL);
	glEnable(GL_NORMALIZE);

	glLightfv(GL_LIGHT0, GL_AMBIENT, LightAmbient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, LightDiffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, LightPosition);
	//glLightfv(GL_LIGHT0, GL_SPECULAR, LightSpecular);	
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, light_model_ambient);
	glLightModelfv(GL_LIGHT_MODEL_LOCAL_VIEWER, light_model_local_viewer);

	glEnable(GL_LIGHT0);

	for (int i = 0; i < 24; i++)
	{
		quadric[i] = gluNewQuadric();
	}

	glClearColor(0.25f, 0.25f, 0.25f, 1.0f);

	glClearDepth(1.0);		// To give existance to depth buffer....3D specific call  (1.0 is the maximum number we are filling depth buffer with)

	// warm-up call to resize()
	resize(WIN_WIDTH, WIN_HEIGHT);		// for size dependent resources to be adjusted according window
	ToggleFullscreen();
	return 0;
}

void resize(int width, int height)
{
	/*For this application we are using ortho projection since this we are taking this application from Green Book by Mark Kilgard*/
	if (height == 0)
	{
		height = 1;
	}
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
	if (width <= height)
	{
		glOrtho(0.0f,		// left
			15.5f,			// right
			0.0f,			// bottom
			(15.5f*(GLfloat)height / (GLfloat)width),	// top
			-10.0f,			// near
			10.0f);			// far
	}
	else
	{
		glOrtho(0.0f,
			(15.5f*(GLfloat)width / (GLfloat)height),
			0.0f,
			15.5f,
			-10.0f,
			10.0f);
	}
}

void update(void)
{
	//Code
	angleOfXRotation = angleOfXRotation + 0.25f;
	/*if (angleOfXRotation >= 360.0f)
	{
		angleOfXRotation = 0.0f;
	}*/

	angleOfYRotation = angleOfYRotation + 0.25f;
	//if (angleOfYRotation >= 360.0f)
	//{
	//	angleOfYRotation = 0.0f;
	//}

	angleOfZRotation = angleOfZRotation + 0.25f;
	//if (angleOfZRotation >= 360.0f)
	//{
	//	angleOfZRotation = 0.0f;
	//}
}

void display(void)
{
	// function declarations
	void draw24Spheres(void);

	//Code
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);			// To make depth buffer functional here
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	//glTranslatef(0.0f, 0.0f, -0.70f);		//Right

	if (keyPressed == 1)
	{
		glRotatef(angleOfXRotation, 1.0f, 0.0f, 0.0f);
		LightPosition[1] = angleOfXRotation;
	}
	else if (keyPressed == 2)
	{
		glRotatef(angleOfYRotation, 0.0f, 1.0f, 0.0f);
		LightPosition[2] = angleOfYRotation;
	}
	else if (keyPressed == 3)
	{
		glRotatef(angleOfZRotation, 0.0f, 0.0f, 1.0f);
		LightPosition[0] = angleOfZRotation;
	}

	glLightfv(GL_LIGHT0, GL_POSITION, LightPosition);

	draw24Spheres();

	/*glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	quadric = gluNewQuadric();
	gluSphere(quadric, 0.2f, 30, 30);*/

	SwapBuffers(ghdc);
}

void draw24Spheres()
{
	//Code
	GLfloat MaterialAmbient[4];
	GLfloat MaterialDiffuse[4];
	GLfloat MaterialSpecular[4];
	GLfloat MaterialShininess[1];

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	/* Apply material values for each sphere*/

	/* ------------------ COLUMN 1 STARTS HERE --------------------------*/

	/* SPHERE 1 STARTS HERE	*/
	MaterialAmbient[0] = 0.0215;	//R
	MaterialAmbient[1] = 0.1745;	//G
	MaterialAmbient[2] = 0.0215;	//B
	MaterialAmbient[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.07568;	//R
	MaterialDiffuse[1] = 0.61424;	//G
	MaterialDiffuse[2] = 0.07568;	//B
	MaterialDiffuse[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.633;	//R
	MaterialSpecular[1] = 0.727811;	//G
	MaterialSpecular[2] = 0.633;	//B
	MaterialSpecular[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.6 * 128;
	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(1.5f, 14.0f, 0.0f);	// This will change for every sphere; since we are in ortho,range of screen is from 0 to 15.5
	gluSphere(quadric[0], 1.0f, 30, 30);
	/* SPHERE 1 ENDS HERE */

	/* SPHERE 2 STARTS HERE	*/
	MaterialAmbient[0] = 0.135;	//R
	MaterialAmbient[1] = 0.2225;	//G
	MaterialAmbient[2] = 0.1575;	//B
	MaterialAmbient[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.54;	//R
	MaterialDiffuse[1] = 0.89;	//G
	MaterialDiffuse[2] = 0.63;	//B
	MaterialDiffuse[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.316228;	//R
	MaterialSpecular[1] = 0.316228;	//G
	MaterialSpecular[2] = 0.316228;	//B
	MaterialSpecular[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.1 * 128;
	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(1.5f, 11.5f, 0.0f);	// This will change for every sphere; since we are in ortho,range of screen is from 0 to 15.5
	gluSphere(quadric[1], 1.0f, 30, 30);
	/* SPHERE 2 ENDS HERE */

	/* SPHERE 3 STARTS HERE	*/
	MaterialAmbient[0] = 0.05375;	//R
	MaterialAmbient[1] = 0.05;	//G
	MaterialAmbient[2] = 0.06625;	//B
	MaterialAmbient[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.18275;	//R
	MaterialDiffuse[1] = 0.17;	//G
	MaterialDiffuse[2] = 0.22525;	//B
	MaterialDiffuse[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.332741;	//R
	MaterialSpecular[1] = 0.328634;	//G
	MaterialSpecular[2] = 0.346435;	//B
	MaterialSpecular[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.3 * 128;
	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(1.5f, 9.0f, 0.0f);	// This will change for every sphere; since we are in ortho,range of screen is from 0 to 15.5
	gluSphere(quadric[2], 1.0f, 30, 30);
	/* SPHERE 3 ENDS HERE */

	/* SPHERE 4 STARTS HERE	*/
	MaterialAmbient[0] = 0.25;	//R
	MaterialAmbient[1] = 0.20725;	//G
	MaterialAmbient[2] = 0.20725;	//B
	MaterialAmbient[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 1.0;	//R
	MaterialDiffuse[1] = 0.829;	//G
	MaterialDiffuse[2] = 0.829;	//B
	MaterialDiffuse[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.296648;	//R
	MaterialSpecular[1] = 0.296648;	//G
	MaterialSpecular[2] = 0.296648;	//B
	MaterialSpecular[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.088 * 128;
	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(1.5f, 6.5f, 0.0f);	// This will change for every sphere; since we are in ortho,range of screen is from 0 to 15.5
	gluSphere(quadric[3], 1.0f, 30, 30);
	/* SPHERE 4 ENDS HERE */

	/* SPHERE 5 STARTS HERE	*/
	MaterialAmbient[0] = 0.1745;	//R
	MaterialAmbient[1] = 0.01175;	//G
	MaterialAmbient[2] = 0.01175;	//B
	MaterialAmbient[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.61424;	//R
	MaterialDiffuse[1] = 0.04136;	//G
	MaterialDiffuse[2] = 0.04136;	//B
	MaterialDiffuse[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.727811;	//R
	MaterialSpecular[1] = 0.626959;	//G
	MaterialSpecular[2] = 0.626959;	//B
	MaterialSpecular[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.6 * 128;
	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(1.5f, 4.0f, 0.0f);	// This will change for every sphere; since we are in ortho,range of screen is from 0 to 15.5
	gluSphere(quadric[4], 1.0f, 30, 30);
	/* SPHERE 5 ENDS HERE */

	/* SPHERE 6 STARTS HERE	*/
	MaterialAmbient[0] = 0.1;		//R
	MaterialAmbient[1] = 0.18725;	//G
	MaterialAmbient[2] = 0.1745;	//B
	MaterialAmbient[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.396;	//R
	MaterialDiffuse[1] = 0.74151;	//G
	MaterialDiffuse[2] = 0.69102;	//B
	MaterialDiffuse[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.297254;	//R
	MaterialSpecular[1] = 0.30829;	//G
	MaterialSpecular[2] = 0.306678;	//B
	MaterialSpecular[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.6 * 128;
	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(1.5f, 1.5f, 0.0f);	// This will change for every sphere; since we are in ortho,range of screen is from 0 to 15.5
	gluSphere(quadric[5], 1.0f, 30, 30);
	/* SPHERE 6 ENDS HERE */
	/*----------------- COLUMN 1 ENDS HERE-----------------*/


	/* ------------------ COLUMN 2 STARTS HERE --------------------------*/

	/* SPHERE 1 STARTS HERE	*/
	MaterialAmbient[0] = 0.329412;	//R
	MaterialAmbient[1] = 0.223529;	//G
	MaterialAmbient[2] = 0.027451;	//B
	MaterialAmbient[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.780392;	//R
	MaterialDiffuse[1] = 0.568627;	//G
	MaterialDiffuse[2] = 0.113725;	//B
	MaterialDiffuse[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.992157;	//R
	MaterialSpecular[1] = 0.941176;	//G
	MaterialSpecular[2] = 0.807843;	//B
	MaterialSpecular[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.21797842 * 128;
	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(9.0f, 14.0f, 0.0f);	// This will change for every sphere; since we are in ortho,range of screen is from 0 to 15.5
	gluSphere(quadric[6], 1.0f, 30, 30);
	/* SPHERE 1 ENDS HERE */

	/* SPHERE 2 STARTS HERE	*/
	MaterialAmbient[0] = 0.2125;	//R
	MaterialAmbient[1] = 0.1275;	//G
	MaterialAmbient[2] = 0.054;	//B
	MaterialAmbient[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.714;	//R
	MaterialDiffuse[1] = 0.4284;	//G
	MaterialDiffuse[2] = 0.18144;	//B
	MaterialDiffuse[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.393548;	//R
	MaterialSpecular[1] = 0.271906;	//G
	MaterialSpecular[2] = 0.166721;	//B
	MaterialSpecular[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.2 * 128;
	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(9.0f, 11.5f, 0.0f);	// This will change for every sphere; since we are in ortho,range of screen is from 0 to 15.5
	gluSphere(quadric[7], 1.0f, 30, 30);
	/* SPHERE 2 ENDS HERE */

	/* SPHERE 3 STARTS HERE	*/
	MaterialAmbient[0] = 0.25;	//R
	MaterialAmbient[1] = 0.25;	//G
	MaterialAmbient[2] = 0.25;	//B
	MaterialAmbient[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.4;	//R
	MaterialDiffuse[1] = 0.4;	//G
	MaterialDiffuse[2] = 0.4;	//B
	MaterialDiffuse[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.774597;	//R
	MaterialSpecular[1] = 0.774597;	//G
	MaterialSpecular[2] = 0.774597;	//B
	MaterialSpecular[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.6 * 128;
	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(9.0f, 9.0f, 0.0f);	// This will change for every sphere; since we are in ortho,range of screen is from 0 to 15.5
	gluSphere(quadric[8], 1.0f, 30, 30);
	/* SPHERE 3 ENDS HERE */

	/* SPHERE 4 STARTS HERE	*/
	MaterialAmbient[0] = 0.19125;	//R
	MaterialAmbient[1] = 0.0735;	//G
	MaterialAmbient[2] = 0.0225;	//B
	MaterialAmbient[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.7038;	//R
	MaterialDiffuse[1] = 0.27048;	//G
	MaterialDiffuse[2] = 0.0828;	//B
	MaterialDiffuse[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.256777;	//R
	MaterialSpecular[1] = 0.137622;	//G
	MaterialSpecular[2] = 0.086014;	//B
	MaterialSpecular[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.1 * 128;
	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(9.0f, 6.5f, 0.0f);	// This will change for every sphere; since we are in ortho,range of screen is from 0 to 15.5
	gluSphere(quadric[9], 1.0f, 30, 30);
	/* SPHERE 4 ENDS HERE */

	/* SPHERE 5 STARTS HERE	*/
	MaterialAmbient[0] = 0.24725;	//R
	MaterialAmbient[1] = 0.1995;	//G
	MaterialAmbient[2] = 0.0745;	//B
	MaterialAmbient[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.75164;	//R
	MaterialDiffuse[1] = 0.60648;	//G
	MaterialDiffuse[2] = 0.22648;	//B
	MaterialDiffuse[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.628281;	//R
	MaterialSpecular[1] = 0.555802;	//G
	MaterialSpecular[2] = 0.366065;	//B
	MaterialSpecular[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.4 * 128;
	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(9.0f, 4.0f, 0.0f);	// This will change for every sphere; since we are in ortho,range of screen is from 0 to 15.5
	gluSphere(quadric[10], 1.0f, 30, 30);
	/* SPHERE 5 ENDS HERE */

	/* SPHERE 6 STARTS HERE	*/
	MaterialAmbient[0] = 0.19225;		//R
	MaterialAmbient[1] = 0.19225;	//G
	MaterialAmbient[2] = 0.19225;	//B
	MaterialAmbient[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.50754;	//R
	MaterialDiffuse[1] = 0.50754;	//G
	MaterialDiffuse[2] = 0.50754;	//B
	MaterialDiffuse[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.508273;	//R
	MaterialSpecular[1] = 0.508273;	//G
	MaterialSpecular[2] = 0.508273;	//B
	MaterialSpecular[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.4 * 128;
	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(9.0f, 1.5f, 0.0f);	// This will change for every sphere; since we are in ortho,range of screen is from 0 to 15.5
	gluSphere(quadric[11], 1.0f, 30, 30);
	/* SPHERE 6 ENDS HERE */
	/*----------------- COLUMN 2 ENDS HERE-----------------*/

	/* ------------------ COLUMN 3 STARTS HERE --------------------------*/

	/* SPHERE 1 STARTS HERE	*/
	MaterialAmbient[0] = 0.0;	//R
	MaterialAmbient[1] = 0.0;	//G
	MaterialAmbient[2] = 0.0;	//B
	MaterialAmbient[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.01;	//R
	MaterialDiffuse[1] = 0.01;	//G
	MaterialDiffuse[2] = 0.01;	//B
	MaterialDiffuse[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.50;	//R
	MaterialSpecular[1] = 0.50;	//G
	MaterialSpecular[2] = 0.50;	//B
	MaterialSpecular[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.25 * 128;
	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(16.5f, 14.0f, 0.0f);	// This will change for every sphere; since we are in ortho,range of screen is from 0 to 15.5
	gluSphere(quadric[12], 1.0f, 30, 30);
	/* SPHERE 1 ENDS HERE */

	/* SPHERE 2 STARTS HERE	*/
	MaterialAmbient[0] = 0.0;	//R
	MaterialAmbient[1] = 0.1;	//G
	MaterialAmbient[2] = 0.06;	//B
	MaterialAmbient[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.0;	//R
	MaterialDiffuse[1] = 0.50980392;	//G
	MaterialDiffuse[2] = 0.50980392;	//B
	MaterialDiffuse[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.50196078;	//R
	MaterialSpecular[1] = 0.50196078;	//G
	MaterialSpecular[2] = 0.50196078;	//B
	MaterialSpecular[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.25 * 128;
	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(16.5f, 11.5f, 0.0f);	// This will change for every sphere; since we are in ortho,range of screen is from 0 to 15.5
	gluSphere(quadric[13], 1.0f, 30, 30);
	/* SPHERE 2 ENDS HERE */

	/* SPHERE 3 STARTS HERE	*/
	MaterialAmbient[0] = 0.0;	//R
	MaterialAmbient[1] = 0.0;	//G
	MaterialAmbient[2] = 0.0;	//B
	MaterialAmbient[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.1;	//R
	MaterialDiffuse[1] = 0.35;	//G
	MaterialDiffuse[2] = 0.1;	//B
	MaterialDiffuse[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.45;	//R
	MaterialSpecular[1] = 0.55;	//G
	MaterialSpecular[2] = 0.45;	//B
	MaterialSpecular[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.25 * 128;
	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(16.5f, 9.0f, 0.0f);	// This will change for every sphere; since we are in ortho,range of screen is from 0 to 15.5
	gluSphere(quadric[14], 1.0f, 30, 30);
	/* SPHERE 3 ENDS HERE */

	/* SPHERE 4 STARTS HERE	*/
	MaterialAmbient[0] = 0.0;	//R
	MaterialAmbient[1] = 0.0;	//G
	MaterialAmbient[2] = 0.0;	//B
	MaterialAmbient[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.5;	//R
	MaterialDiffuse[1] = 0.0;	//G
	MaterialDiffuse[2] = 0.0;	//B
	MaterialDiffuse[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.7;	//R
	MaterialSpecular[1] = 0.6;	//G
	MaterialSpecular[2] = 0.6;	//B
	MaterialSpecular[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.25 * 128;
	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(16.5f, 6.5f, 0.0f);	// This will change for every sphere; since we are in ortho,range of screen is from 0 to 15.5
	gluSphere(quadric[15], 1.0f, 30, 30);
	/* SPHERE 4 ENDS HERE */

	/* SPHERE 5 STARTS HERE	*/
	MaterialAmbient[0] = 0.0;	//R
	MaterialAmbient[1] = 0.0;	//G
	MaterialAmbient[2] = 0.0;	//B
	MaterialAmbient[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.55;	//R
	MaterialDiffuse[1] = 0.55;	//G
	MaterialDiffuse[2] = 0.55;	//B
	MaterialDiffuse[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.70;	//R
	MaterialSpecular[1] = 0.70;	//G
	MaterialSpecular[2] = 0.70;	//B
	MaterialSpecular[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.25 * 128;
	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(16.5f, 4.0f, 0.0f);	// This will change for every sphere; since we are in ortho,range of screen is from 0 to 15.5
	gluSphere(quadric[16], 1.0f, 30, 30);
	/* SPHERE 5 ENDS HERE */

	/* SPHERE 6 STARTS HERE	*/
	MaterialAmbient[0] = 0.0;		//R
	MaterialAmbient[1] = 0.0;	//G
	MaterialAmbient[2] = 0.0;	//B
	MaterialAmbient[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.5;	//R
	MaterialDiffuse[1] = 0.5;	//G
	MaterialDiffuse[2] = 0.0;	//B
	MaterialDiffuse[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.60;	//R
	MaterialSpecular[1] = 0.60;	//G
	MaterialSpecular[2] = 0.50;	//B
	MaterialSpecular[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.25 * 128;
	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(16.5f, 1.5f, 0.0f);	// This will change for every sphere; since we are in ortho,range of screen is from 0 to 15.5
	gluSphere(quadric[17], 1.0f, 30, 30);
	/* SPHERE 6 ENDS HERE */
	/*----------------- COLUMN 3 ENDS HERE-----------------*/

	/* ------------------ COLUMN 4 STARTS HERE --------------------------*/

	/* SPHERE 1 STARTS HERE	*/
	MaterialAmbient[0] = 0.02;	//R
	MaterialAmbient[1] = 0.02;	//G
	MaterialAmbient[2] = 0.02;	//B
	MaterialAmbient[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.01;	//R
	MaterialDiffuse[1] = 0.01;	//G
	MaterialDiffuse[2] = 0.01;	//B
	MaterialDiffuse[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.4;	//R
	MaterialSpecular[1] = 0.4;	//G
	MaterialSpecular[2] = 0.4;	//B
	MaterialSpecular[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.078125 * 128;
	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(24.0f, 14.0f, 0.0f);	// This will change for every sphere; since we are in ortho,range of screen is from 0 to 15.5
	gluSphere(quadric[18], 1.0f, 30, 30);
	/* SPHERE 1 ENDS HERE */

	/* SPHERE 2 STARTS HERE	*/
	MaterialAmbient[0] = 0.0;	//R
	MaterialAmbient[1] = 0.05;	//G
	MaterialAmbient[2] = 0.05;	//B
	MaterialAmbient[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.4;	//R
	MaterialDiffuse[1] = 0.5;	//G
	MaterialDiffuse[2] = 0.5;	//B
	MaterialDiffuse[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.04;	//R
	MaterialSpecular[1] = 0.7;	//G
	MaterialSpecular[2] = 0.7;	//B
	MaterialSpecular[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.078125 * 128;
	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(24.0f, 11.5f, 0.0f);	// This will change for every sphere; since we are in ortho,range of screen is from 0 to 15.5
	gluSphere(quadric[19], 1.0f, 30, 30);
	/* SPHERE 2 ENDS HERE */

	/* SPHERE 3 STARTS HERE	*/
	MaterialAmbient[0] = 0.0;	//R
	MaterialAmbient[1] = 0.05;	//G
	MaterialAmbient[2] = 0.0;	//B
	MaterialAmbient[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.4;	//R
	MaterialDiffuse[1] = 0.5;	//G
	MaterialDiffuse[2] = 0.4;	//B
	MaterialDiffuse[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.04;	//R
	MaterialSpecular[1] = 0.7;	//G
	MaterialSpecular[2] = 0.04;	//B
	MaterialSpecular[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.078125 * 128;
	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(24.0f, 9.0f, 0.0f);	// This will change for every sphere; since we are in ortho,range of screen is from 0 to 15.5
	gluSphere(quadric[20], 1.0f, 30, 30);
	/* SPHERE 3 ENDS HERE */

	/* SPHERE 4 STARTS HERE	*/
	MaterialAmbient[0] = 0.05;	//R
	MaterialAmbient[1] = 0.0;	//G
	MaterialAmbient[2] = 0.0;	//B
	MaterialAmbient[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.5;	//R
	MaterialDiffuse[1] = 0.4;	//G
	MaterialDiffuse[2] = 0.4;	//B
	MaterialDiffuse[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.7;	//R
	MaterialSpecular[1] = 0.04;	//G
	MaterialSpecular[2] = 0.04;	//B
	MaterialSpecular[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.078125 * 128;
	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(24.0f, 6.5f, 0.0f);	// This will change for every sphere; since we are in ortho,range of screen is from 0 to 15.5
	gluSphere(quadric[21], 1.0f, 30, 30);
	/* SPHERE 4 ENDS HERE */

	/* SPHERE 5 STARTS HERE	*/
	MaterialAmbient[0] = 0.05;	//R
	MaterialAmbient[1] = 0.05;	//G
	MaterialAmbient[2] = 0.05;	//B
	MaterialAmbient[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.5;	//R
	MaterialDiffuse[1] = 0.5;	//G
	MaterialDiffuse[2] = 0.5;	//B
	MaterialDiffuse[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.7;	//R
	MaterialSpecular[1] = 0.7;	//G
	MaterialSpecular[2] = 0.7;	//B
	MaterialSpecular[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.078125 * 128;
	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(24.0f, 4.0f, 0.0f);	// This will change for every sphere; since we are in ortho,range of screen is from 0 to 15.5
	gluSphere(quadric[22], 1.0f, 30, 30);
	/* SPHERE 5 ENDS HERE */

	/* SPHERE 6 STARTS HERE	*/
	MaterialAmbient[0] = 0.05;		//R
	MaterialAmbient[1] = 0.05;	//G
	MaterialAmbient[2] = 0.0;	//B
	MaterialAmbient[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_AMBIENT, MaterialAmbient);

	MaterialDiffuse[0] = 0.5;	//R
	MaterialDiffuse[1] = 0.5;	//G
	MaterialDiffuse[2] = 0.4;	//B
	MaterialDiffuse[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_DIFFUSE, MaterialDiffuse);

	MaterialSpecular[0] = 0.7;	//R
	MaterialSpecular[1] = 0.7;	//G
	MaterialSpecular[2] = 0.04;	//B
	MaterialSpecular[3] = 1.0f;		//A
	glMaterialfv(GL_FRONT, GL_SPECULAR, MaterialSpecular);

	MaterialShininess[0] = 0.078125 * 128;
	glMaterialfv(GL_FRONT, GL_SHININESS, MaterialShininess);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(24.0f, 1.5f, 0.0f);	// This will change for every sphere; since we are in ortho,range of screen is from 0 to 15.5
	gluSphere(quadric[23], 1.0f, 30, 30);
	/* SPHERE 6 ENDS HERE */
	/*----------------- COLUMN 4 ENDS HERE-----------------*/

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
	for (int i = 0; i < 24; i++)
	{
		gluDeleteQuadric(quadric[i]);
	}
}