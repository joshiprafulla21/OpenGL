//Headerfiles
#include<windows.h>
#include<gl/glew.h>
#include<gl/GL.h>
#include<stdio.h>		// For file I/O operation, FILE structure is needed

#pragma comment(lib,"glew32.lib")
#pragma comment(lib,"opengl32.lib")
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
GLenum result;

GLuint gVertexShaderObject;
GLuint gFragmentShaderObject;
GLuint gShaderProgramObject;

GLint iShaderCompileStatus = 0;
GLint iInfoLogLen = 0;
GLchar* szInfoLog = NULL;

GLint iProgramLinkStatus = 0;

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
		TEXT("PP_BlueScreen_With_Shaders !!"),
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
	void uninitialize(void);
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

	glClearColor(0.0f, 0.0f, 1.0f, 1.0f);		// BLUE COLOR

	result = glewInit();		// To enable core profile extensions
	if (result != GLEW_OK)
	{
		fprintf(gpFile, "glewInit() failed !!\n");
		uninitialize();
		DestroyWindow(ghwnd);
	}
	/* ++++++++++++++++++++ VERTEX SHADER ++++++++++++++++++++ */
	// DEFINE VERTEX SHADER OBJECT
	gVertexShaderObject = glCreateShader(GL_VERTEX_SHADER);

	// WRITE VERTEX SHADER CODE
	const GLchar *vertexShaderSourceCode = 
		"void main(void)" \
		"{" \
		"}";

	//SPECIFY THE ABOVE SOURCE CODE TO VERTEX SHADER OBJECT
	glShaderSource(gVertexShaderObject, 1, (const GLchar **)&vertexShaderSourceCode, NULL);

	//COMPILE VERTEX SHADER
	glCompileShader(gVertexShaderObject);

	//ERROR CATCHING STEPS
	glGetShaderiv(gVertexShaderObject, GL_COMPILE_STATUS, &iShaderCompileStatus);
	if (iShaderCompileStatus == GL_FALSE)
	{
		glGetShaderiv(gVertexShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLen);
		if (iInfoLogLen > 0)
		{
			szInfoLog = (GLchar *)malloc(iInfoLogLen);
			if (szInfoLog != NULL)
			{
				GLsizei written;
				glGetShaderInfoLog(gVertexShaderObject, iInfoLogLen, &written, szInfoLog);
				fprintf(gpFile, "Vertex Shader Compilation Log :", szInfoLog);
				free(szInfoLog);
				uninitialize();
				DestroyWindow(ghwnd);
				exit(0);
			}
		}
	}

	/* ++++++++++ FRAGMENT SHADER +++++++++++ */
	//RE-INITIALIZATION OF GLOBAL VARIABLES REQUIRED FOR FRAGMENT SHADER
	iShaderCompileStatus = 0;
	iInfoLogLen = 0;
	szInfoLog = NULL;
	// DEFINE FRAGMENT SHADER OBJECT
	gFragmentShaderObject = glCreateShader(GL_FRAGMENT_SHADER);

	// WRITE FRAGMENT SHADER CODE
	const GLchar *fragmentShaderSourceCode =
		"void main(void)" \
		"{" \
		"}";

	//SPECIFY THE ABOVE SOURCE CODE TO FRAGMENT SHADER OBJECT
	glShaderSource(gFragmentShaderObject, 1, (const GLchar **)&fragmentShaderSourceCode, NULL);

	//COMPILE FRAGMENT SHADER
	glCompileShader(gFragmentShaderObject);

	// ERROR CATCHING STEPS
	glGetShaderiv(gFragmentShaderObject, GL_COMPILE_STATUS, &iShaderCompileStatus);
	if (iShaderCompileStatus == GL_FALSE)
	{
		glGetShaderiv(gFragmentShaderObject, GL_INFO_LOG_LENGTH, &iInfoLogLen);
		if (iInfoLogLen > 0)
		{
			szInfoLog = (GLchar *)malloc(iInfoLogLen);
			if (szInfoLog != NULL)
			{
				GLsizei written;
				glGetShaderInfoLog(gFragmentShaderObject, iInfoLogLen, &written, szInfoLog);
				fprintf(gpFile, "Fragment Shader Compilation Log :", szInfoLog);
				free(szInfoLog);
				uninitialize();
				DestroyWindow(ghwnd);
				exit(0);
			}
		}
	}


	/* +++++++ SHADER PROGRAM ++++++++ */
	gShaderProgramObject = glCreateProgram();

	// ATTACH VERTEX SHADER TO SHADER PROGRAM
	glAttachShader(gShaderProgramObject, gVertexShaderObject);

	// ATTACH FRAGMENT SHADER TO SHADER PROGRAM
	glAttachShader(gShaderProgramObject, gFragmentShaderObject);

	// LINK THE SHADER PROGRAM
	glLinkProgram(gShaderProgramObject);

	// ERROR CHECKING CODE
	/*glGetProgramiv(gShaderProgramObject, GL_LINK_STATUS, &iProgramLinkStatus);
	if (iProgramLinkStatus == GL_FALSE)
	{
		glGetProgramiv(gShaderProgramObject, GL_INFO_LOG_LENGTH, &iInfoLogLen);
		if (iInfoLogLen > 0)
		{
			szInfoLog = (GLchar *)malloc(iInfoLogLen);
			if (szInfoLog != NULL)
			{
				GLsizei written;
				glGetProgramInfoLog(gShaderProgramObject, iInfoLogLen, &written, szInfoLog);
				fprintf(gpFile, "Shader Program Linking Log :", szInfoLog);
				free(szInfoLog);
				uninitialize();
				DestroyWindow(ghwnd);
				exit(0);
			}
		}
	}*/

	/*------FOLLOWING CHANGES ARE 3D SPECIFIC--------*/

	glClearDepth(1.0f);				// To give existence to depth buffer.. 1.0 is the maximum number filled up in depth buffer
	glEnable(GL_DEPTH_TEST);		//  To enable despth test	
	glDepthFunc(GL_LEQUAL);			// Which test to be done is specified here

	/*---------3D SPECIFIC CALLS ENDS HERE-------------*/

	resize(WIN_WIDTH, WIN_HEIGHT);		// warm-up call to resize() for size dependent resources to be adjusted according window
	ToggleFullscreen();
	return 0;
}


void resize(int width, int height)
{
	if (height == 0)
		height = 1;
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
}


void display(void)
{
	//Code
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// To make depth buffer functional; here actually depth buffer is filled up with 1.0

	glUseProgram(gShaderProgramObject);
	/* OUR MAIN CODE RELATED TO OPENGL LIES HERE i.e., UNIFORMS LIES HERE */
	glUseProgram(0);

	SwapBuffers(ghdc);
}



void uninitialize(void)
{
	glUseProgram(gShaderProgramObject);
	glDetachShader(gShaderProgramObject, gFragmentShaderObject);
	glDetachShader(gShaderProgramObject, gVertexShaderObject);
	glDeleteShader(gFragmentShaderObject);
	gFragmentShaderObject = 0;
	glDeleteShader(gVertexShaderObject);
	gVertexShaderObject = 0;
	glDeleteProgram(gShaderProgramObject);
	gShaderProgramObject = 0;
	glUseProgram(0);



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
