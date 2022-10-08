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

// this is the quad's vertex shader in an ugly C string
const char* vert_shader_str =
        "#version 430\n                                                               \
layout (location = 0) in vec2 vp;\n                                           \
layout (location = 1) in vec2 vt;\n                                           \
out vec2 st;\n                                                                \
\n                                                                            \
void main () {\n                                                              \
  st = vt;\n                                                                  \
  gl_Position = vec4 (vp, 0.0, 1.0);\n                                        \
}\n";

// this is the quad's fragment shader in an ugly C string
const char* frag_shader_str =
        "#version 430\n                                                               \
in vec2 st;\n                                                                 \
uniform sampler2D img;\n                                                      \
out vec4 fc;\n                                                                \
\n                                                                            \
void main () {\n                                                              \
  fc = texture (img, st);\n                                                 \
}\n";
GLuint create_quad_vao() {
    GLuint vao = 0, vbo = 0;
    float verts[] = { -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f };
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
GLuint create_quad_program() {
    GLuint program     = glCreateProgram();
    GLuint vert_shader = glCreateShader( GL_VERTEX_SHADER );
    glShaderSource( vert_shader, 1, &vert_shader_str, NULL );
    glCompileShader( vert_shader );
    glAttachShader( program, vert_shader );
    GLuint frag_shader = glCreateShader( GL_FRAGMENT_SHADER );
    glShaderSource( frag_shader, 1, &frag_shader_str, NULL );
    glCompileShader( frag_shader );
    glAttachShader( program, frag_shader );
    glLinkProgram( program );
    return program;
}

//compute shader
int tex_w = 512, tex_h = 512;
GLuint tex_output;
GLuint ray_shader;
GLuint ray_program;

GLuint quad_vao     ;
GLuint quad_program;
glm::vec2 mousePos(0,0);

Renderer simpleRenderer(true,GL_TRIANGLES);
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

void setupComputeShader(){
//    //setup the compute shader


    glGenTextures(1, &tex_output);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex_output);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tex_w, tex_h, 0, GL_RGBA, GL_FLOAT,
                 NULL);
    glBindImageTexture(0, tex_output, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

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
    quad_program = create_quad_program();

//    simpleShader.loadShaders({"TempShader.vert", "TempShader.frag", "","",""});
//
//    const std::vector<GLfloat> data{
//            0.0f, 0.0f,
//            0.0f, 0.5f,
//            0.5f, 0.5f,
//    };

//    simpleRenderer._setup_data(data,std::vector<int>({2}));

    //shader string
    const char* compute_shader_str =
            "#version 430\n                                                               \
layout (local_size_x = 1, local_size_y = 1) in;\n                             \
layout (rgba32f, binding = 0) uniform image2D img_output;\n                   \
\n                                                                            \
void main () {\n                                                              \
  vec4 pixel = vec4 (0.0, 0.0, 0.0, 1.0);\n                                   \
  ivec2 pixel_coords = ivec2 (gl_GlobalInvocationID.xy);\n                    \
\n                                                                            \
float max_x = 5.0;\n                                                          \
float max_y = 5.0;\n                                                          \
ivec2 dims = imageSize (img_output);\n                                        \
float x = (float(pixel_coords.x * 2 - dims.x) / dims.x);\n                    \
float y = (float(pixel_coords.y * 2 - dims.y) / dims.y);\n                    \
vec3 ray_o = vec3 (x * max_x, y * max_y, 0.0);\n                              \
vec3 ray_d = vec3 (0.0, 0.0, -1.0); // ortho\n                                \
\n                                                                            \
vec3 sphere_c = vec3 (0.0, 0.0, -10.0);                                       \
float sphere_r = 1.0;                                                         \
\n                                                                            \
vec3 omc = ray_o - sphere_c;\n                                                \
float b = dot (ray_d, omc);\n                                                 \
float c = dot (omc, omc) - sphere_r * sphere_r;\n                             \
float bsqmc = b * b - c;\n                                                    \
float t = 10000.0;\n                                                          \
// hit one or both sides\n                                                    \
if (bsqmc >= 0.0) {\n                                                         \
  pixel = vec4 (0.4, 0.4, 1.0, 1.0);\n                                        \
}\n                                                                           \
\n                                                                            \
  imageStore (img_output, pixel_coords, pixel);\n                             \
}\n";

    ray_shader = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(ray_shader, 1, &compute_shader_str, NULL);
    glCompileShader(ray_shader);
// check for compilation errors as per normal here

    ray_program = glCreateProgram();
    glAttachShader(ray_program, ray_shader);
    glLinkProgram(ray_program);
// check for linking errors and validate program as per normal here

// check for linking errors and validate program as per normal here



    // texture handle and dimensions
    tex_output = 0;
    int tex_w = 512, tex_h = 512;
    { // create the texture
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
    }

    { // query up the workgroups
        int work_grp_size[3], work_grp_inv;
        // maximum global work group (total work in a dispatch)
        glGetIntegeri_v( GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &work_grp_size[0] );
        glGetIntegeri_v( GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &work_grp_size[1] );
        glGetIntegeri_v( GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &work_grp_size[2] );
        printf( "max global (total) work group size x:%i y:%i z:%i\n", work_grp_size[0], work_grp_size[1], work_grp_size[2] );
        // maximum local work group (one shader's slice)
        glGetIntegeri_v( GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &work_grp_size[0] );
        glGetIntegeri_v( GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &work_grp_size[1] );
        glGetIntegeri_v( GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &work_grp_size[2] );
        printf( "max local (in one shader) work group sizes x:%i y:%i z:%i\n", work_grp_size[0], work_grp_size[1], work_grp_size[2] );
        // maximum compute shader invocations (x * y * z)
        glGetIntegerv( GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &work_grp_inv );
        printf( "max computer shader invocations %i\n", work_grp_inv );
    }




    return true;
}



void myDisplay() {

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//    std::vector<glm::mat4> MVPs;
//    MVPs.push_back(glm::mat4(1.0f));
//
//    simpleRenderer.render_static(simpleShader,MVPs);

    // launch compute shaders!
    glUseProgram(ray_program);
    glDispatchCompute((GLuint)tex_w, (GLuint)tex_h, 1);

    // make sure writing to image has finished before read
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    // normal drawing pass
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(quad_program);
    glBindVertexArray(quad_vao);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex_output);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

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



