// Kompajliranje:
// g++ -o SimpleAnim SimpleAnim.cpp util.cpp -lGLEW -lGL -lGLU -lglut -lpthread

#include <iostream>
#include <numeric>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Renderer.hpp"
#include "main.hpp"


#ifdef _WIN32
    #include <windows.h>
#endif

#ifndef _WIN32
    #define LINUX_UBUNTU_SEGFAULT
#endif

//#ifdef LINUX_UBUNTU_SEGFAULT
//    #include <pthread.h>
//#endif


[[maybe_unused]] GLuint window;
GLuint sub_width = 500, sub_height = 500;
int WindowHeight;
int WindowWidth;
const double Xmin = 0.0, Xmax = 3.0;
const double Ymin = 0.0, Ymax = 3.0;



glm::vec2 mousePos(0,0);

Renderer simpleRenderer(true,GL_TRIANGLE_STRIP);
Shader simpleShader;



void glutPassiveMotionFunc(int x, int y ) {

    float xPos = ((float)x)/((float)(WindowWidth-1));
    float yPos = ((float)y)/((float)(WindowHeight-1));
    xPos =2*xPos-1;
    yPos=-2*yPos+1;

    mousePos.x = xPos;
    mousePos.y = yPos;


    glutPostRedisplay();

}

//void myKeyboardFunc( unsigned char key, int x, int y ){}

void mySpecialKeyFunc( int key, int x, int y )
{
	switch ( key ) {
        case GLUT_KEY_UP:
            //snake.add();
            break;
        case GLUT_KEY_DOWN:
            //snake.cutTail();
            break;

        case GLUT_KEY_LEFT:
            //std::cout << snake.toString();
            throw std::exception();
        default:
            break;
    }
}

int main(int argc, char ** argv)
{

	// Sljedeci blok sluzi kao bugfix koji je opisan gore
	#ifdef LINUX_UBUNTU_SEGFAULT
        //int i=pthread_getconcurrency();
	#endif

	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutInitContextFlags(GLUT_DEBUG);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );
	glutInitWindowSize((int)sub_width,(int)sub_height);
	glutInitWindowPosition(100,100);
	glutInit(&argc, argv);

	window = glutCreateWindow("Heads tart" );
	glutReshapeFunc(resizeWindow);
	glutDisplayFunc(myDisplay);
//	glutKeyboardFunc( myKeyboardFunc );			// Handles "normal" ascii symbols
    //glutMouseFunc(myMouseFunc);
    glutPassiveMotionFunc(glutPassiveMotionFunc);
	glutSpecialFunc( mySpecialKeyFunc );		// Handles "special" keyboard keys

	glewExperimental = GL_TRUE;
	glewInit();

	init_data();

	// Omogući uporabu Z-spremnika
	glEnable(GL_DEPTH_TEST);

	glutMainLoop();
    return 0;
}




GLuint quad_vao;
bool init_data()
{
	glClearColor( 1.0f, 1.0f, 1.0f, 1.0f );

    simpleShader.loadShaders({"TempShader.vert", "TempShader.frag", "","",""});

    return true;
}



void myDisplay() {


    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);



    glutSwapBuffers();
    glutPostRedisplay();
}

void resizeWindow(int w, int h)
{
	double scale, center;
	double windowXmin, windowXmax, windowYmin, windowYmax;

	glViewport( 0, 0, w, h );	// View port uses whole window

	w = (w==0) ? 1 : w;
	h = (h==0) ? 1 : h;
    WindowHeight = (h>1) ? h : 2;
    WindowWidth = (w>1) ? w : 2;
	if ( (Xmax-Xmin)/w < (Ymax-Ymin)/h ) {
		scale = ((Ymax-Ymin)/h)/((Xmax-Xmin)/w);
		center = (Xmax+Xmin)/2;
		windowXmin = center - (center-Xmin)*scale;
		windowXmax = center + (Xmax-center)*scale;
		windowYmin = Ymin;
		windowYmax = Ymax;
	}
	else {
		scale = ((Xmax-Xmin)/w)/((Ymax-Ymin)/h);
		center = (Ymax+Ymin)/2;
		windowYmin = center - (center-Ymin)*scale;
		windowYmax = center + (Ymax-center)*scale;
		windowXmin = Xmin;
		windowXmax = Xmax;
	}



}



