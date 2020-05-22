//Headerfiles
#include<windows.h>
#include "resource1.h"
#include<gl/GL.h>
#include<stdio.h>		// For file I/O operation, FILE structure is needed
#include<gl/GLU.h>
#define _USE_MATH_DEFINES 1
#include<math.h>
#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"glu32.lib")
#pragma comment(lib,"winmm.lib")
#define WIN_WIDTH 800
#define WIN_HEIGHT 600
//#define PI 3.14


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
// Following variables are for animation of each character
GLfloat First_I_xValue = -2.2f;
GLfloat A_xValue = 2.1f;
GLfloat N_yValue = 1.9f;
GLfloat Second_I_yValue = -1.9f;

GLfloat Saffron_X = 0.0f;
GLfloat Saffron_Y = 0.0f;
GLfloat Saffron_Z = 0.0f;
GLfloat Green_X = 0.0f;
GLfloat Green_Y = 0.0f;
GLfloat Green_Z = 0.0f;

// Following variables are for update functionality of plane and text values
float xValue = 0.0f;
float yValue = 0.0f;
float zValue = 0.0f;
//float angle = 0.0f;
GLfloat leftUpperAngle = M_PI;		// These angles are needed for circular motion of plane
GLfloat leftLowerAngle = M_PI;
GLfloat rightUpperAngle = (3 * M_PI) / 2;
GLfloat rightLowerAngle = M_PI / 2;
float xValue_Plane = -4.5f;
float yValue_Plane = 0.0f;
float rightXValue = 0.0f;
float rightYValue = 0.0f;
float xPlane = 0.0f;


/* Follwing are the color values of TriColor of each plane*/
float Tri_Saffron_X = 1.0f;
float Tri_Saffron_Y = 0.6f;
float Tri_Saffron_Z = 0.2f;

float Tri_White_All = 1.0f;

float Tri_Green_X = 0.0705882f;
float Tri_Green_Y = 0.5333333f;
float Tri_Green_Z = 0.02745098f;

int flag = 0;				// for checking te condition of fading out the tricolor

float Tri_X_Value = -6.15f;		// Used for removing black line on letters which appears after fade out


GLfloat angle = 0.0f;

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
		TEXT("FFP_Dynamic_India!!"),
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
				update();				// Here call to Update() ==> Animation
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
	case WM_CREATE:
		PlaySound(MAKEINTRESOURCE(ID_SONG), NULL, SND_RESOURCE | SND_ASYNC);
		break;
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
	ToggleFullscreen();
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

	//function declaration
	void Draw_First_I(void);
	void Draw_N(void);
	void Draw_D(void);
	void Draw_Second_I(void);
	void Draw_A(void);
	void Call_Aeroplane(void);
	
	//Code

	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);

	/*glTranslatef(0.0f, 0.0f, -2.5f);*/

	Draw_First_I();
	Draw_N();
	Draw_D();
	Draw_Second_I();
	Draw_A();
	/*---- Following code is for calling aeroplane functionality after animation of D ----*/


	if (Green_X >= 0.0705882f)		// Whether D is animated and placed at its proper position of not
	{
		Call_Aeroplane();
	}
	/*---- Aeroplane functionlity code ends here ----*/


	SwapBuffers(ghdc);
}


void update()
{
	//Code
	if (First_I_xValue < -0.9f)
	{
		First_I_xValue = First_I_xValue + 0.0002f;		// changing 'x' value for first I in INDIA so that it can animate from left to its position
	}

	if (First_I_xValue >= -0.9f)					// whether 'I' is placed at proper place or not
	{
		if (A_xValue >= 0.7f)
		{
			A_xValue = A_xValue - 0.0002f;				// Changing 'x' value of 'A' in INDIA so that it can animate only after first 'I' is animated and positioned properly
		}
	}
	
	if (A_xValue <= 0.7f)						// whether 'A' is placed at proper place or not
	{
		if (N_yValue >= 0.0f)
		{
			N_yValue = N_yValue - 0.0002f;		// Changing 'y' value of 'N' in INDIA so that it animates only after 'A' is animated and positioned properly
		}
	}

	if (N_yValue <= 0.0f)						// whether 'N' is placed at proper place or not
	{
		if (Second_I_yValue <= 0.0f)			// Changing 'y' value of second 'I' in INDIA so that it animates only after 'N' is animated and positioned properly
		{
			Second_I_yValue = Second_I_yValue + 0.0002f;
		}
	}
	if (Second_I_yValue >= 0.0f)				// whether second 'I' is placed at proper place or not
	{
		//Below code is for fade in of Saffron color of 'D' in INDIA
		if (Saffron_X <= 1.0f)
		{
			Saffron_X = Saffron_X + 0.001f;
		}
		if(Saffron_Y <= 0.6f)
		{
			Saffron_Y = Saffron_Y + 0.001f;
		}
		if (Saffron_Z <= 0.2f)
		{
			Saffron_Z = Saffron_Z + 0.001f;
		}
		//Below code is for fade in of Green color of 'D' in INDIA
		if (Green_X <= 0.0705882f)
		{
			Green_X = Green_X + 0.001f;
		}
		if (Green_Y <= 0.5333333f)
		{
			Green_Y = Green_Y + 0.001f;
		}
		if (Green_Z <= 0.02745098f)
		{
			Green_Z = Green_Z + 0.001f;
		}
	}

	if (flag == 1)
	{
		// Saffron color strip fade out
		if (Tri_Saffron_X >= 0.0)
		{
			Tri_Saffron_X = Tri_Saffron_X - 0.001;
		}

		if (Tri_Saffron_Y >= 0.0)
		{
			Tri_Saffron_Y = Tri_Saffron_Y - 0.001;
		}

		if (Tri_Saffron_Z >= 0.0)
		{
			Tri_Saffron_Z = Tri_Saffron_Z - 0.001;
		}

		// White color strip fade out

		if (Tri_White_All >= 0.0)
		{
			Tri_White_All = Tri_White_All - 0.001;
		}

		// Green color strip fade out

		if (Tri_Green_X >= 0.0)
		{
			Tri_Green_X = Tri_Green_X - 0.001;
		}

		if (Tri_Green_Y >= 0.0)
		{
			Tri_Green_Y = Tri_Green_Y - 0.001;
		}

		if (Tri_Green_Z >= 0.0)
		{
			Tri_Green_Z = Tri_Green_Z - 0.001;
		}
	}

	if (Tri_Saffron_X < 0.0) /*|| Tri_White_All < 0.0 || Tri_Green_X < 0.0)*/	// Check whether line has been faded out or not
	{
		//Tri_X_Value = 8.3f;
		Tri_X_Value = Tri_X_Value + 0.1;
		xValue_Plane = 4.0f;
	}
}


void Draw_First_I()
{
	//Code

	glLoadIdentity();
	glTranslatef(First_I_xValue, 0.0f, -2.5f);

	// Following code is for drawing 'I' in INDIA
	glLineWidth(6.0f);
	glBegin(GL_LINES);

	// Below code is for vertical line of I
	glColor3f(1.0f, 0.6f, 0.2f);							// Saffron color, RGB values taken from wiki
	glVertex3f(0.0f, 0.7f, 0.0f);

	glColor3f(0.0705882f, 0.5333333f, 0.02745098f);			// Green color, RGB values taken from wiki
	glVertex3f(0.0f, -0.7f, 0.0f);
	// Vertical line of I ends here
	// 'I' rendered here

	glEnd();
}

void Draw_N()
{
	//Code
	// Following code is for drawing 'N' in INDIA
	glLoadIdentity();
	glTranslatef(-0.55f, N_yValue, -2.5f);
	glBegin(GL_LINES);

	glEnable(GL_BLEND);
	glEnable(GL_LINE_SMOOTH);

	// Below code for right line of N
	glColor3f(1.0f, 0.6f, 0.2f);							// Saffron color, RGB values taken from wiki 
	glVertex3f(0.15f, 0.7f, 0.0f);

	glColor3f(0.0705882f, 0.5333333f, 0.02745098f);			// Green color, RGB values taken from wiki
	glVertex3f(0.15f, -0.7f, 0.0f);
	// Right line of N ends here

	//Below code for middle line of N
	glColor3f(0.0705882f, 0.5333333f, 0.02745098f);			// Green color, RGB values taken from wiki
	glVertex3f(0.15f, -0.7f, 0.0f);

	glColor3f(1.0f, 0.6f, 0.2f);							// Saffron color, RGB values taken from wiki 
	glVertex3f(-0.15f, 0.7f, 0.0f);
	// Middle line of N ends here

	// Below code for left line of N
	glColor3f(1.0f, 0.6f, 0.2f);							// Saffron color, RGB values taken from wiki 
	glVertex3f(-0.15f, 0.7f, 0.0f);

	glColor3f(0.0705882f, 0.5333333f, 0.02745098f);			// Green color, RGB values taken from wiki
	glVertex3f(-0.15f, -0.7f, 0.0f);
	//Left line ends here
	// 'N' rendered
	glEnd();
}

void Draw_D()
{
	//function declaration
	
	//Code
	// Following code is for drawing 'D' in INDIA
	glLoadIdentity();
	glTranslatef(-0.1f, 0.0f, -2.5f);
	glBegin(GL_LINES);

	//Below code for upper horizontal of D
	glColor3f(Saffron_X, Saffron_Y, Saffron_Z);							// Saffron color, RGB values taken from wiki 
	glVertex3f(-0.15f, 0.7f, 0.0f);

	glColor3f(Saffron_X, Saffron_Y, Saffron_Z);							// Saffron color, RGB values taken from wiki 
	glVertex3f(0.15f, 0.7f, 0.0f);
	// Upper horizontal line of D ends here

	// Below code for right vertical of D
	glColor3f(Saffron_X, Saffron_Y, Saffron_Z);							// Saffron color, RGB values taken from wiki 
	glVertex3f(0.15f, 0.7f, 0.0f);

	glColor3f(Green_X, Green_Y, Green_Z);			// Green color, RGB values taken from wiki
	glVertex3f(0.15f, -0.7f, 0.0f);
	// Right vertical line of D ends here

	// Below code for lower horizontal of D
	glColor3f(Green_X, Green_Y, Green_Z);			// Green color, RGB values taken from wiki
	glVertex3f(0.15f, -0.7f, 0.0f);

	glColor3f(Green_X, Green_Y, Green_Z);			// Green color, RGB values taken from wiki
	glVertex3f(-0.15f, -0.7f, 0.0f);

	//Below code is for left vertical line of D
	glColor3f(Green_X, Green_Y, Green_Z);			// Green color, RGB values taken from wiki
	glVertex3f(-0.1f, -0.7f, 0.0f);

	glColor3f(Saffron_X, Saffron_Y, Saffron_Z);							// Saffron color, RGB values taken from wiki 
	glVertex3f(-0.1f, 0.7f, 0.0f);
	//Left vertical line of D ends here
	// 'D' rendered here

	glEnd();

}

void Draw_Second_I()
{
	//Code
	// Following code is for drawing 'I' in INDIA

	glLoadIdentity();
	/*glTranslatef(0.25f, 0.0f, -2.0f);*/	
	glTranslatef(0.25f, Second_I_yValue, -2.5f);
	glLineWidth(6.0f);
	glBegin(GL_LINES);

	// Below code is for vertical line of I
	glColor3f(1.0f, 0.6f, 0.2f);							// Saffron color, RGB values taken from wiki
	glVertex3f(0.0f, 0.7f, 0.0f);

	glColor3f(0.0705882f, 0.5333333f, 0.02745098f);			// Green color, RGB values taken from wiki
	glVertex3f(0.0f, -0.7f, 0.0f);
	// Vertical line of I ends here
	// 'I' rendered here
	glEnd();
}

void Draw_A()
{
	//Code
	// Following code is for 'A' in INDIA

	glLoadIdentity();
	//glTranslatef(0.7f, 0.0f, -2.0f);
	glTranslatef(A_xValue, 0.0f, -2.5f);
	glLineWidth(6.0f);
	glBegin(GL_LINES);

	// Below code is for left slanting line of 'A'
	glColor3f(1.0f, 0.6f, 0.2f);							// Saffron color, RGB values taken from wiki 
	glVertex3f(0.0f, 0.7f, 0.0f);

	glColor3f(0.0705882f, 0.5333333f, 0.02745098f);			// Green color, RGB values taken from wiki
	glVertex3f(-0.25f, -0.7f, 0.0f);
	// left slanting line of 'A' ends here

	// Below code is for left slanting line of 'A'
	glColor3f(1.0f, 0.6f, 0.2f);							// Saffron color, RGB values taken from wiki 
	glVertex3f(0.0f, 0.7f, 0.0f);

	glColor3f(0.0705882f, 0.5333333f, 0.02745098f);			// Green color, RGB values taken from wiki
	glVertex3f(0.25f, -0.7f, 0.0f);
	// left slanting line of 'A' ends here
	glEnd();

}


/*---- This function is for Calling aeroplane after Dynamic INDIA animation ----*/

void Call_Aeroplane()
{
	//Function declaration
	void Draw_Circle(void);
	void Draw_Text(void);
	void Draw_Aeroplane(void);
	void Draw_A_Strip(void);
	void Draw_Tricolor();
	//Code

	// For Upper left plane which is required in Dynamic India Assignment
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(-1.5, 2.0f, -2.5f);
	//glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
	if (leftUpperAngle < (3.0f*(M_PI / 2.0f)))
	{
		xValue = 2.0f * cos(leftUpperAngle);
		yValue = 2.0f * sin(leftUpperAngle);
		glTranslatef(xValue, yValue, -2.5f);
		//glRotatef(angle, 0.0f, 0.0f, 1.0f);

	}
	//glTranslatef(xValue, yValue, -2.5f);	// Commented to remove the plane once it is on X-axis
	Draw_Aeroplane();
	Draw_Text();
	Draw_Tricolor();
	//leftUpperAngle = leftUpperAngle + 0.00031f;
	leftUpperAngle = leftUpperAngle + 0.00030f;
	angle = angle - 0.01f;

	/*--------Lower Plane------------*/

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(-1.5f, -2.0f, -2.5f);
	if (leftLowerAngle > (M_PI / 2.0f))
	{
		xValue = 2.0f * cos(leftLowerAngle);
		yValue = 2.0f * sin(leftLowerAngle);
		glTranslatef(xValue, yValue, -2.5f);
	}
	/*glTranslatef(xValue, yValue, -2.5f);*/		// Commented to remove the plane once it is on X-axis
	Draw_Aeroplane();
	Draw_Text();
	Draw_Tricolor();
	//leftLowerAngle = leftLowerAngle - 0.00031f;		//0.0006f;
	leftLowerAngle = leftLowerAngle - 0.00030f;
	/*--------Middle Plane------------*/

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(xValue_Plane, 0.0f, -2.5f);
	if (xValue_Plane <= 2.15f)		// hard-coded value, after which 3 planes separates
	{
		//xValue_Plane = xValue_Plane + 0.00074f;		//0.00095f;
		xValue_Plane = xValue_Plane + 0.000715f;
		//glTranslatef(xValue_Plane, 0.0f, -2.5f);
	}
	
	glTranslatef(xValue_Plane, 0.0f, -2.5f);
	Draw_Aeroplane();
	Draw_Text();
	Draw_Tricolor();
	if (xValue_Plane > 2.15f)				// Keep the tricolor after plane gone
	{
		//xValue_Plane = 14.0f;
		//glTranslatef(4.0f, 0.0f, -2.5f);		// sustain tricolor; therefore it is given after if block
		//Draw_Tricolor();
		flag = 1;
	}
	

	if (xValue_Plane > 2.15f)
	{
		Draw_A_Strip();
	}

	/*---- Upper Right Plane ----*/
	if (xValue_Plane > 1.15f)		// 1.15 is the value of X where plane separates,hardcoded value after many trials
	{
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glTranslatef(2.3f, 1.6f, -2.5f);		// here values of X and Y are center point of that arc

		if (rightUpperAngle <= (2.0f*M_PI) + 0.5f)
		{
			rightXValue = 1.5f * cos(rightUpperAngle);
			rightYValue = 1.5f * sin(rightUpperAngle);
			glTranslatef(rightXValue, rightYValue, -2.5f);
			Draw_Aeroplane();
			Draw_Text();
			rightUpperAngle = rightUpperAngle + 0.0009f;
		}
	}

	/*---- Lower Right Plane ----*/

	if (xValue_Plane > 1.15f)		// 1.15 is the value of X where plane separates,hardcoded value after many trials
	{
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glTranslatef(2.3f, -1.6f, -2.5f);		// here values of X and Y are center point of that arc

		if (rightLowerAngle >= 0.0f)
		{
			rightXValue = 1.5f * cos(rightLowerAngle);
			rightYValue = 1.5f * sin(rightLowerAngle);
			glTranslatef(rightXValue, rightYValue, -2.5f);
			Draw_Aeroplane();
			Draw_Text();
			rightLowerAngle = rightLowerAngle - 0.0009f;
		}
	}

}


/*---- This function is for Drawing Aeroplane after Dynamic INDIA animation ----*/
void Draw_Aeroplane()
{
	//Code
	// Middle rectangle of Aeroplane which is required in Dynamic India Assignment

	glBegin(GL_QUADS);
	glColor3f(0.7294117f, 0.8862745f, 0.9333333f);
	glVertex3f(0.45f, 0.18f, 0.0f);

	glColor3f(0.7294117f, 0.8862745f, 0.9333333f);
	glVertex3f(-0.45f, 0.18f, 0.0f);

	glColor3f(0.7294117f, 0.8862745f, 0.9333333f);
	glVertex3f(-0.45f, -0.18f, 0.0f);

	glColor3f(0.7294117f, 0.8862745f, 0.9333333f);
	glVertex3f(0.45f, -0.18f, 0.0f);
	glEnd();
	/*----------Tricolor behind plane starts here-----------*/

	glBegin(GL_LINES);
	//Saffron line
	glColor3f(1.0f, 0.6f, 0.2f);		// Saffron color, RGBs taken from wikipedia
	glVertex3f(-0.45f, 0.05f, 0.0f);

	glColor3f(1.0f, 0.6f, 0.2f);		// Saffron color, RGBs taken from wikipedia
	glVertex3f(-2.5f, 0.05f, 0.0f);

	//White line
	glColor3f(1.0f, 1.0f, 1.0f);		// White color, RGBs taken from wikipedia
	glVertex3f(-0.45f, 0.03f, 0.0f);

	glColor3f(1.0f, 1.0f, 1.0f);		// White color, RGBs taken from wikipedia
	glVertex3f(-2.5f, 0.03f, 0.0f);

	//Green line
	glColor3f(0.0705882f, 0.5333333f, 0.02745098f);		// Green color, RGBs taken from wikipedia
	glVertex3f(-0.45f, 0.0004f, 0.0f);

	glColor3f(0.0705882f, 0.5333333f, 0.02745098f);		// Green color, RGBs taken from wikipedia
	glVertex3f(-2.5f, 0.0004f, 0.0f);

	glEnd();

	/*----------Tricolor behind plane ends here-----------*/
	// Front Triangle of Aeroplane which is required in Dynamic India Assignment

	glBegin(GL_TRIANGLES);
	glColor3f(0.7294117f, 0.8862745f, 0.9333333f);
	glVertex3f(0.45f, 0.18f, 0.0f);

	glColor3f(0.7294117f, 0.8862745f, 0.9333333f);
	glVertex3f(0.45f, -0.18f, 0.0f);

	glColor3f(0.7294117f, 0.8862745f, 0.9333333f);
	glVertex3f(0.8f, -0.18f, 0.0f);

	glEnd();


	// Upper Triangle of Aeroplane which is required in Dynamic India Assignment

	glBegin(GL_TRIANGLES);

	glColor3f(0.7294117f, 0.8862745f, 0.9333333f);
	glVertex3f(-0.45f, 0.18f, 0.0f);

	glColor3f(0.7294117f, 0.8862745f, 0.9333333f);
	glVertex3f(-0.20f, 0.18f, 0.0f);

	glColor3f(0.7294117f, 0.8862745f, 0.9333333f);
	glVertex3f(-0.45f, 0.45f, 0.0f);

	glEnd();

	// Side Triangle of Aeroplane which is required in Dynamic India Assignment

	glBegin(GL_TRIANGLES);

	glColor3f(0.7294117f, 0.8862745f, 0.9333333f);
	glVertex3f(-0.25f, -0.13f, 0.0f);

	glColor3f(0.7294117f, 0.8862745f, 0.9333333f);
	glVertex3f(-0.25f, -0.40f, 0.0f);

	glColor3f(0.7294117f, 0.8862745f, 0.9333333f);
	glVertex3f(0.45f, -0.13f, 0.0f);

	glEnd();

}


/*---- This function is for Drawing Text on Aeroplane after Dynamic INDIA animation ----*/
void Draw_Text()
{
	glLineWidth(5.0f);
	glBegin(GL_LINES);
	// For 'I' in IAF
	glColor3f(0.0f, 0.0f, 0.0f);
	glVertex3f(-0.2f, 0.15f, 0.0f);

	glColor3f(0.0f, 0.0f, 0.0f);
	glVertex3f(-0.2f, -0.15f, 0.0f);

	// For 'A' in IAF
	//Left line of 'A'
	glColor3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.15f, 0.0f);

	glColor3f(0.0f, 0.0f, 0.0f);
	glVertex3f(-0.12f, -0.15f, 0.0f);
	//Right line of 'A'
	glColor3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.15f, 0.0f);

	glColor3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.12f, -0.15f, 0.0f);

	//Middle strip of 'A'
	glColor3f(0.0f, 0.0f, 0.0f);
	glVertex3f(-0.07f, 0.0f, 0.0f);

	glColor3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.07f, 0.0f, 0.0f);

	// For 'F' in IAF // vertical line of F
	glColor3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.2f, 0.15f, 0.0f);

	glColor3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.2f, -0.15f, 0.0f);

	//Upper Line of 'F'
	glColor3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.2f, 0.15f, 0.0f);

	glColor3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.35f, 0.15f, 0.0f);

	//Lower Line of 'F'
	glColor3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.2f, 0.02f, 0.0f);

	glColor3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.3f, 0.02f, 0.0f);

	glEnd();
}

/* THIS FUNCTION DRAWS STRIP OF A */

void Draw_A_Strip()
{
	//Code
	//Following code is for tri-color strip in 'A'


	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.7f, 0.014f, -2.5f);
	glLineWidth(5.0f);
	glBegin(GL_LINES);

	// Below code for saffron colorkk
	glColor3f(1.0f, 0.6f, 0.2f);							// Saffron color, RGB values taken from wiki 
	glVertex3f(-0.123f, 0.012f, 0.0f);

	glColor3f(1.0f, 0.6f, 0.2f);							// Saffron color, RGB values taken from wiki 
	glVertex3f(0.123f, 0.012f, 0.0f);
	//saffron color ends here

	// Below code for white color
	glColor3f(1.0f, 1.0f, 1.0f);							// White color, RGB values taken from wiki 
	glVertex3f(-0.123f, 0.0f, 0.0f);

	glColor3f(1.0f, 1.0f, 1.0f);							// White color, RGB values taken from wiki 
	glVertex3f(0.123f, 0.0f, 0.0f);
	//white color ends here

	// Below code for green color
	glColor3f(0.0705882f, 0.5333333f, 0.02745098f);			// Green color, RGB values taken from wiki 
	glVertex3f(-0.123f, -0.012f, 0.0f);

	glColor3f(0.0705882f, 0.5333333f, 0.02745098f);			// Green color, RGB values taken from wiki 
	glVertex3f(0.123f, -0.012f, 0.0f);
	//green color ends here

	glEnd();
}

/* THIS FUNCTION DRAWS TRI-COLOR BEHIND THE PLANE */

void Draw_Tricolor()
{
	//Code

	/*----------Tricolor behind plane starts here-----------*/

	glBegin(GL_LINES);
	//Saffron line
	glColor3f(Tri_Saffron_X, Tri_Saffron_Y, Tri_Saffron_Z);		// Saffron color, RGBs taken from wikipedia
	glVertex3f(-0.45f, 0.05f, 0.0f);

	glColor3f(Tri_Saffron_X, Tri_Saffron_Y, Tri_Saffron_Z);		// Saffron color, RGBs taken from wikipedia
	glVertex3f(Tri_X_Value, 0.05f, 0.0f);

	//White line
	glColor3f(Tri_White_All, Tri_White_All, Tri_White_All);		// White color, RGBs taken from wikipedia
	glVertex3f(-0.45f, 0.03f, 0.0f);

	glColor3f(Tri_White_All, Tri_White_All, Tri_White_All);		// White color, RGBs taken from wikipedia
	glVertex3f(Tri_X_Value, 0.03f, 0.0f);

	//Green line
	glColor3f(Tri_Green_X, Tri_Green_Y, Tri_Green_Z);		// Green color, RGBs taken from wikipedia
	glVertex3f(-0.45f, 0.0004f, 0.0f);

	glColor3f(Tri_Green_X, Tri_Green_Y, Tri_Green_Z);		// Green color, RGBs taken from wikipedia
	glVertex3f(Tri_X_Value, 0.0004f, 0.0f);

	glEnd();

	/*----------Tricolor behind plane ends here-----------*/

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