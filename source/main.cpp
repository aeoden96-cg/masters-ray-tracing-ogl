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


// this is the quad's fragment shader in an ugly C string

GLuint create_quad_vao() {
    GLuint vao = 0, vbo = 0;
    float verts[] =
            { -1.0f, -1.0f, 0.0f, 0.0f,
              -1.0f, 1.0f, 0.0f, 1.0f,
              1.0f, -1.0f, 1.0f, 0.0f,
              1.0f, 1.0f, 1.0f, 1.0f
            };
    glGenBuffers( 1, &vbo );
    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    glBufferData( GL_ARRAY_BUFFER, 16 * sizeof( float ), verts, GL_STATIC_DRAW );
    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );
    glEnableVertexAttribArray( 0 );
    GLintptr stride = 4 * sizeof( float );
    glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, stride, NULL );
    glEnableVertexAttribArray( 1 );
    GLintptr offset = 2 * sizeof( float );
    glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, stride, (GLvoid*)offset );
    return vao;
}


//compute shader
int tex_w = 512, tex_h = 512;
GLuint tex_output;

GLuint ray_program;

GLuint quad_vao     ;
GLuint quad_program;
glm::vec2 mousePos(0,0);

Renderer simpleRenderer(true,GL_TRIANGLES);
Shader simpleShader;

Shader quadShader;


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

void setupComputeShader(){
    //setup the compute shader

    // texture handle and dimensions
    tex_output = 0;
    // create the texture
    glGenTextures( 1, &tex_output );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, tex_output );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    // linear allows us to scale the window up retaining reasonable quality
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    // same internal format as compute shader input
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA32F, tex_w, tex_h, 0, GL_RGBA, GL_FLOAT, NULL );
    // bind to image unit so can write to specific pixels from the shader
    glBindImageTexture( 0, tex_output, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F );

    int work_grp_cnt[3];

    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &work_grp_cnt[0]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &work_grp_cnt[1]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &work_grp_cnt[2]);

    printf("max global (total) work group counts x:%i y:%i z:%i\n",
           work_grp_cnt[0], work_grp_cnt[1], work_grp_cnt[2]);


    int work_grp_size[3];

    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &work_grp_size[0]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &work_grp_size[1]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &work_grp_size[2]);

    printf("max local (in one shader) work group sizes x:%i y:%i z:%i\n",
           work_grp_size[0], work_grp_size[1], work_grp_size[2]);

    int work_grp_inv;

    glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &work_grp_inv);
    printf("max local work group invocations %i\n", work_grp_inv);

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

    setupComputeShader();
	init_data();

	// Omogući uporabu Z-spremnika
	glEnable(GL_DEPTH_TEST);


	glutMainLoop();
    return 0;
}



bool init_data()
{
	glClearColor( 1.0f, 1.0f, 1.0f, 1.0f );

    quad_vao = create_quad_vao();
    quad_program = quadShader.create_quad_program();

    ray_program = quadShader.create_compute_program();

    return true;
}

void draw_quad()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(quad_program);
    glBindVertexArray(quad_vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex_output);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void myDisplay() {

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // launch compute shaders!
    glUseProgram(ray_program);
    glDispatchCompute((GLuint)tex_w, (GLuint)tex_h, 1);

    // make sure writing to image has finished before read
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    // normal drawing pass
    draw_quad();

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



