#include<GL/freeglut.h>

bool bIsFullScreen = false;
GLfloat angle = 0.0f;
int main(int argc, char* argv[])
{
	// Function declarations
	void initialize(void);
	void uninitialize(void);
	void reshape(int, int);
	void display(void);
	void keyboard(unsigned char, int, int);
	void mouse(int, int, int, int);
	void update(void);

	// Code
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(800, 600);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("My First OpenGL Program - Prafulla Joshi");

	/*-------------*/
	

	/*-------------*/


	initialize();

	// Callbacks
	glutIdleFunc(update);
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

	// Following calls are from native 
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glShadeModel(GL_SMOOTH);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	glutFullScreen();
}

void uninitialize(void)
{
	// Code
}

void reshape(int width, int height)
{
	// Code
	if (height == 0)
		height = 1;
	glViewport(0, 0, (GLsizei)width, (GLsizei)height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
}


void update(void)
{
	//Code
	angle = angle + 0.1f;
	if (angle >= 360.0f)					//This snippet is for continuous rotation of rectangle
		angle = 0.0f;
	glutPostRedisplay();
}


void display(void)
{
	// Code
	//Local functions declarations
	void Draw_Pyramid(void);
	void Draw_Cube(void);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);			// To make depth buffer functional here

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(-1.5f, 0.0f, -6.0f);

	glRotatef(angle, 0.0f, 1.0f, 0.0f);

	glBegin(GL_TRIANGLES);
	Draw_Pyramid();
	glEnd();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(1.5f, 0.0f, -6.0f);
	glScalef(0.75f, 0.75f, 0.75f);
	glRotatef(angle, 1.0f, 1.0f, 1.0f);

	glBegin(GL_QUADS);
	Draw_Cube();
	glEnd();

	//glFlush();
	glutSwapBuffers();
}



void Draw_Pyramid(void)
{
	/*Pyramid has total 4 surfaces. We will give 3D co-ordinates to each surface as following */

	//Front Surface
	glColor3f(1.0f, 0.0f, 0.0f);		// Red
	glVertex3f(0.0f, 1.0f, 0.0f);

	glColor3f(0.0f, 1.0f, 0.0f);		// Green
	glVertex3f(-1.0f, -1.0f, 1.0f);

	glColor3f(0.0f, 0.0f, 1.0f);		// Blue
	glVertex3f(1.0f, -1.0f, 1.0f);

	//Right Surface
	glColor3f(1.0f, 0.0f, 0.0f);		// Red
	glVertex3f(0.0f, 1.0f, 0.0f);

	glColor3f(0.0f, 0.0f, 1.0f);		// Blue
	glVertex3f(1.0f, -1.0f, 1.0f);

	glColor3f(0.0f, 1.0f, 0.0f);		// Green
	glVertex3f(1.0f, -1.0f, -1.0f);

	//Back Surface
	glColor3f(1.0f, 0.0f, 0.0f);		// Red
	glVertex3f(0.0f, 1.0f, 0.0f);

	glColor3f(0.0f, 1.0f, 0.0f);		// Green
	glVertex3f(1.0f, -1.0f, -1.0f);

	glColor3f(0.0f, 0.0f, 1.0f);		// Blue
	glVertex3f(-1.0f, -1.0f, -1.0f);

	//Left Surface
	glColor3f(1.0f, 0.0f, 0.0f);		// Red
	glVertex3f(0.0f, 1.0f, 0.0f);

	glColor3f(0.0f, 0.0f, 1.0f);		// Blue
	glVertex3f(-1.0f, -1.0f, -1.0f);

	glColor3f(0.0f, 1.0f, 0.0f);		// Green
	glVertex3f(-1.0f, -1.0f, 1.0f);
}


void Draw_Cube(void)
{
	/*Cube has total 6 surfaces. We will give 3D co-ordinates to each surface as following */

	//Top Surface
	glColor3f(1.0f, 0.0f, 0.0f);		// Red

	glVertex3f(1.0f, 1.0f, -1.0f);
	glVertex3f(-1.0f, 1.0f, -1.0f);
	glVertex3f(-1.0f, 1.0f, 1.0f);
	glVertex3f(1.0f, 1.0f, 1.0f);


	//Bottom Surface
	glColor3f(0.0f, 1.0f, 0.0f);		// Green

	glVertex3f(1.0f, -1.0f, -1.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);

	// Front Surface
	glColor3f(0.0f, 0.0f, 1.0f);		// Blue

	glVertex3f(1.0f, 1.0f, 1.0f);
	glVertex3f(-1.0f, 1.0f, 1.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);


	// Right Surface
	glColor3f(0.0f, 1.0f, 1.0f);		// Cyan

	glVertex3f(1.0f, 1.0f, -1.0f);
	glVertex3f(1.0f, 1.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, 1.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);


	// Back Surface
	glColor3f(1.0f, 0.0f, 1.0f);		// Magenta

	glVertex3f(-1.0f, 1.0f, -1.0f);
	glVertex3f(1.0f, 1.0f, -1.0f);
	glVertex3f(1.0f, -1.0f, -1.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);


	// Left Surface
	glColor3f(1.0f, 1.0f, 0.0f);		// Yellow

	glVertex3f(-1.0f, 1.0f, 1.0f);
	glVertex3f(-1.0f, 1.0f, -1.0f);
	glVertex3f(-1.0f, -1.0f, -1.0f);
	glVertex3f(-1.0f, -1.0f, 1.0f);

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
