#include<GL/freeglut.h>

bool bIsFullScreen = false;

int main(int argc, char* argv[])
{
	// Function declarations
	void initialize(void);
	void uninitialize(void);
	void reshape(int, int);
	void display(void);
	void keyboard(unsigned char, int, int);
	void mouse(int, int, int, int);

	// Code
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
	glutInitWindowSize(800, 600);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("Horizontal and Vertical Lines using GLUT");

	initialize();

	// Callbacks

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutCloseFunc(uninitialize);

	glutMainLoop();

	return 0;
}

void initialize(void)
{
	// Code
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

void uninitialize(void)
{
	// Code
}

void reshape(int width, int height)
{
	// Code
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (height == 0)
	{
		height = 1;
	}
	glViewport(0, 0, GLsizei(width), (GLsizei)height);

}

void display(void)
{
	// Code
	glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glLineWidth(3.0f);
	glBegin(GL_LINES);
		
	// For Horizintal Line
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex2f(-1.0f, 0.0f);
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex2f(1.0f, 0.0f);

	// For Vertical Line 1
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex2f(0.0f, 1.0f);
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex2f(0.0f, -1.0f);

	glEnd();
	
	glLineWidth(1.0f);
	glBegin(GL_LINES);

	float i;
	// Vertical lines
	for (i = -1.0f; i < 1.0f; i = i + 0.05f)
	{
		glColor3f(0.0f, 1.0f, 0.0f);
		glVertex2f(i, 1.0f);
		glColor3f(0.0f, 1.0f, 0.0f);
		glVertex2f(i, -1.0f);
	}

	// Horizontal lines
	
	for (i = 1.0f; i > -1.0f; i = i - 0.05f)
	{


		glColor3f(0.0f, 1.0f, 0.0f);
		glVertex2f(-1.0f, i);
		glColor3f(0.0f, 1.0f, 0.0f);
		glVertex2f(1.0f, i);
	}

	glEnd();

	glFlush();
}

void keyboard(unsigned char key, int x, int y)
{
	// Code
	switch (key)
	{
	case 27:
		glutLeaveMainLoop();
		break;
	case 'F':
	case 'f':
		if (bIsFullScreen == false)
		{
			glutFullScreen();
			bIsFullScreen = true;
		}
		else
		{
			glutLeaveFullScreen();
			bIsFullScreen = false;
		}
		break;
	}
}

void mouse(int button, int state, int x, int y)
{
	// Code
	switch (button)
	{
	case GLUT_LEFT_BUTTON:
		break;
	case GLUT_RIGHT_BUTTON:
		glutLeaveMainLoop();
		break;
	}
}
