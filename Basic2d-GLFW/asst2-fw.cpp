/*******************************************************************************
 *
 *   Harvard Computer Science
 *   CS 175: Computer Graphics
 *   Professor Steven Gortler
 *   Students: Michael {Tingley, Traver}
 *   Emails: {michaeltingley, mtraver}@college.harvard.edu
 *
 ******************************************************************************/
 // Modified by DS to clear texture buffer activation before drawing the second object on August 5, 2024

#include <vector>
#include <string>
#include <memory>
#include <stdexcept>
#include <stdio.h>
#if __GNUG__
#   include <tr1/memory>
#endif

#include <GL/glew.h>

#include <GLFW/glfw3.h>  // GLFW helper library

#ifdef __MAC__
#   define IS_MAC true
#   include <GLUT/glut.h>
#else
#   define IS_MAC false
#   include <GL/glut.h>
#endif

#include "ppm.h"
#include "glsupport-fw.h"

 // added by ds to fix compile error C4996
#pragma warning(disable : 4996)

using namespace std;      // for string, vector, iostream and other standard C++ stuff
using namespace std::tr1; // for shared_ptr

/* G L O B A L S **************************************************/

/* !!!!!!!! IMPORTANT !!!!!!!!!!!!!!!!!!!!!!!!!!
 * Before you start working on this assignment, set the following variable properly
 * to indicate whether you want to use OpenGL 2.x with GLSL 1.0 or OpenGL 3.x+ with
 * GLSL 1.3.
 *
 * Set g_Gl2Compatible = true to use GLSL 1.0 and g_Gl2Compatible = false to use GLSL 1.3.
 * Make sure that your machine supports the version of GLSL you are using. In particular,
 * on Mac OS X currently there is no way of using OpenGL 3.x with GLSL 1.3 when
 * GLUT is used.
 *
 * If g_Gl2Compatible=true, shaders with -gl2 suffix will be loaded.
 * If g_Gl2Compatible=false, shaders with -gl3 suffix will be loaded.
 * To complete the assignment you only need to edit the shader files that get loaded
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
static const bool g_Gl2Compatible = false;
static const float g_initialWidth  = 512.0;
static const float g_initialHeight = g_initialWidth;

static int g_width             = g_initialWidth;  /** screen width */
static int g_height            = g_initialHeight; /** screen height */
static bool g_leftClicked      = false;     /** is the left mouse button down? */
static bool g_rightClicked     = false;     /** is the right mouse button down? */
static float g_objScale        = 1.0;       /** scale factor for object */
static int g_leftClickX, g_leftClickY;      /** coordinates for mouse left click event */
static int g_rightClickX, g_rightClickY;    /** coordinates for mouse right click event */

/**
 * Represents the offset of the triangle in the x-direction. This increments
 * each time the shape should move to the right, and decrements each time the
 * shape should move to the left.
 */
static int g_xOffset           = 0.0;

/** Like xOffset, but in the y direction. */
static int g_yOffset           = 0.0;

/** Global shader states */
struct SquareShaderState {
  GlProgram program;

  /** Handles to uniform variables */
  GLint h_uVertexScale;
  GLint h_uTex0, h_uTex1;
  GLint h_uXCoefficient, h_uYCoefficient;

  /** Handles to vertex attributes */
  GLint h_aPosition;
  GLint h_aTexCoord;
};

struct TriangleShaderState {
  GlProgram program;

  /** Handles to uniform variables */
  GLint h_uTex2;
  GLint h_uXCoefficient, h_uYCoefficient;
  GLint h_uXOffset, h_uYOffset;

  /** Handles to vertex attributes */
  GLint h_aPosition;
  GLint h_aTexCoord;
  GLint h_aColor;
};

static shared_ptr<SquareShaderState> g_squareShaderState;
static shared_ptr<TriangleShaderState> g_triangleShaderState;

/** Global texture instance */
static shared_ptr<GlTexture> g_tex0, g_tex1, g_tex2;

/** Global geometries to draw a triangle with indecies */ 
struct GeometryPX {
  GlBufferObject posVbo, texVbo, colorVbo, indexVbo;
  GlVertexArrayObject VAO; // VAO is vertex array object
};

static shared_ptr<GeometryPX> g_square;
static shared_ptr<GeometryPX> g_triangle;


/* C A L L B A C K S **************************************************/

static void drawSquare() {
  /* Activate the glsl program */
  glUseProgram(g_squareShaderState->program);

  /* Bind textures */
  // Activate the texture unit 0 and binding texture of g_tex0
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, *g_tex0);

  // Activate the texture unit 1 and binding texture of g_tex1
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, *g_tex1);

  /* Compute coefficients for maintaining aspect ratio */
  float scaleCoefficient = min(g_width / g_initialWidth, g_height / g_initialHeight);

  /* Set glsl uniform variables */
  safe_glUniform1i(g_squareShaderState->h_uTex0, 0); /* 0 meams GL_TEXTURE0 */
  safe_glUniform1i(g_squareShaderState->h_uTex1, 1); /* 1 meams GL_TEXTURE1 */
  safe_glUniform1f(g_squareShaderState->h_uVertexScale, g_objScale);
  safe_glUniform1f(g_squareShaderState->h_uXCoefficient, g_initialWidth / g_width * scaleCoefficient);
  safe_glUniform1f(g_squareShaderState->h_uYCoefficient, g_initialHeight / g_height * scaleCoefficient);

  /* Bind vertex buffers */
  GLCall(glBindVertexArray(g_square->VAO));  // VAO is vertex array object added by DS August 8, 2024

  GLCall(glBindBuffer(GL_ARRAY_BUFFER, g_square->posVbo));

  safe_glVertexAttribPointer(g_squareShaderState->h_aPosition,
                             2, GL_FLOAT, GL_FALSE, 0, 0);

  glBindBuffer(GL_ARRAY_BUFFER, g_square->texVbo);
  safe_glVertexAttribPointer(g_squareShaderState->h_aTexCoord,
                             2, GL_FLOAT, GL_FALSE, 0, 0);

  safe_glEnableVertexAttribArray(g_squareShaderState->h_aPosition);
  safe_glEnableVertexAttribArray(g_squareShaderState->h_aTexCoord);

  // Bind the index buffer and draw elements
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_square->indexVbo);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

  safe_glDisableVertexAttribArray(g_squareShaderState->h_aPosition);
  safe_glDisableVertexAttribArray(g_squareShaderState->h_aTexCoord);

  /* Check for errors */
  checkGlErrors();
}

static void drawTriangle() {

  /* Activate the glsl program */
  glUseProgram(g_triangleShaderState->program);

  /* Bind textures */
  // Activate the texture unit 2 and binding texture of g_tex2
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, *g_tex2);

  /* Compute coefficients for maintaining aspect ratio */
  float scaleCoefficient = min(g_width / g_initialWidth, g_height / g_initialHeight);

  /* Set glsl uniform variables */
  safe_glUniform1i(g_triangleShaderState->h_uTex2, 2); /* 2 meams GL_TEXTURE2 */
  safe_glUniform1f(g_triangleShaderState->h_uXCoefficient, g_initialWidth / g_width * scaleCoefficient);
  safe_glUniform1f(g_triangleShaderState->h_uYCoefficient, g_initialHeight / g_height * scaleCoefficient);

  /* Uniform variables used to move the triangle around the screen */
  safe_glUniform1f(g_triangleShaderState->h_uXOffset, g_xOffset * .05);
  safe_glUniform1f(g_triangleShaderState->h_uYOffset, g_yOffset * .05);

  /* Bind vertex buffers */

  GLCall(glBindVertexArray(g_triangle->VAO));  // VAO is vertex array object added by DS August 8, 2024

  glBindBuffer(GL_ARRAY_BUFFER, g_triangle->posVbo);

  safe_glVertexAttribPointer(g_triangleShaderState->h_aPosition,
                             2, GL_FLOAT, GL_FALSE, 0, 0);

  glBindBuffer(GL_ARRAY_BUFFER, g_triangle->texVbo);
  safe_glVertexAttribPointer(g_triangleShaderState->h_aTexCoord,
                             2, GL_FLOAT, GL_FALSE, 0, 0);

  glBindBuffer(GL_ARRAY_BUFFER, g_triangle->colorVbo);
  safe_glVertexAttribPointer(g_triangleShaderState->h_aColor,
                             3, GL_FLOAT, GL_FALSE, 0, 0);

  safe_glEnableVertexAttribArray(g_triangleShaderState->h_aPosition);
  safe_glEnableVertexAttribArray(g_triangleShaderState->h_aTexCoord);
  safe_glEnableVertexAttribArray(g_triangleShaderState->h_aColor);

  // Bind the index buffer and draw elements
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_triangle->indexVbo);
  glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);


  safe_glDisableVertexAttribArray(g_triangleShaderState->h_aPosition);
  safe_glDisableVertexAttribArray(g_triangleShaderState->h_aTexCoord);
  safe_glDisableVertexAttribArray(g_triangleShaderState->h_aColor);

  /* Check for errors */
  checkGlErrors();
}

/**
 * Display
 *
 * Whenever OpenGL requires a screen refresh it will call display() to draw the
 * scene. We specify that this is the correct function to call with the
 * glutDisplayFunc() function during initialization.
 */
static void display(GLFWwindow* window) {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    drawSquare();
    drawTriangle();

    glfwSwapBuffers(window);

  /* check for errors */
  checkGlErrors();
}


/* H E L P E R    F U N C T I O N S ***********************************/
static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    g_width = width;
    g_height = height;
    glViewport(0, 0, width, height);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    // Handle keyboard input
}

static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    // Handle mouse input
}

static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos) {
    // Handle cursor position
}
static void initGLFW(int argc, char** argv) {
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW");
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(g_width, g_height, "CS 175: Hello World", NULL, NULL);
    if (!window) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }
    glfwMakeContextCurrent(window);

    glfwSwapInterval(1); //1이면 vsync rate와 같은 속도로 화면 갱신

    // glfwMakeContextCurrent가 호출된 후에 glewInit이 수행되어야 함
    if (glewInit() != GLEW_OK)
    {
        std::cout << "Error\n";
    }

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);
   
}

static void initGLState() {
    glClearColor(128.0f / 255, 200.0f / 255, 1.0f, 0.0f);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    if (!g_Gl2Compatible) {
        glEnable(GL_FRAMEBUFFER_SRGB);
    }
}

static void loadSquareShader(SquareShaderState& ss) {
  const GLuint h = ss.program; /* Short hand */

  readAndCompileShader(ss.program, "shaders/asst2-sq-gl3.vshader", "shaders/asst2-sq-gl3.fshader");
 

  /* Retrieve handles to uniform variables */
  ss.h_uVertexScale = safe_glGetUniformLocation(h, "uVertexScale");
  ss.h_uTex0 = safe_glGetUniformLocation(h, "uTex0");
  ss.h_uTex1 = safe_glGetUniformLocation(h, "uTex1");
  ss.h_uXCoefficient = safe_glGetUniformLocation(h, "uXCoefficient");
  ss.h_uYCoefficient = safe_glGetUniformLocation(h, "uYCoefficient");

  /* Retrieve handles to vertex attributes */
  ss.h_aPosition = safe_glGetAttribLocation(h, "aPosition");
  ss.h_aTexCoord = safe_glGetAttribLocation(h, "aTexCoord");

  if (!g_Gl2Compatible)
    glBindFragDataLocation(h, 0, "fragColor");
  checkGlErrors();
}

static void loadTriangleShader(TriangleShaderState& ss) {
  const GLuint h = ss.program; /* Short hand */

  readAndCompileShader(ss.program, "shaders/asst2-tr-gl3.vshader", "shaders/asst2-tr-gl3.fshader");


  /* Retrieve handles to uniform variables */
  ss.h_uTex2 = safe_glGetUniformLocation(h, "uTex2");
  ss.h_uXCoefficient = safe_glGetUniformLocation(h, "uXCoefficient");
  ss.h_uYCoefficient = safe_glGetUniformLocation(h, "uYCoefficient");
  ss.h_uXOffset = safe_glGetUniformLocation(h, "uXOffset");
  ss.h_uYOffset = safe_glGetUniformLocation(h, "uYOffset");

  /* Retrieve handles to vertex attributes */
  ss.h_aPosition = safe_glGetAttribLocation(h, "aPosition");
  ss.h_aTexCoord = safe_glGetAttribLocation(h, "aTexCoord");
  ss.h_aColor = safe_glGetAttribLocation(h, "aColor");

  if (!g_Gl2Compatible)
    glBindFragDataLocation(h, 0, "fragColor");
  checkGlErrors();
}

static void initShaders() {
  g_squareShaderState.reset(new SquareShaderState);
  loadSquareShader(*g_squareShaderState);

  g_triangleShaderState.reset(new TriangleShaderState);
  loadTriangleShader(*g_triangleShaderState);
}

static void loadSquareGeometry(const GeometryPX& g) {
	const int vdim = 4; /* 4 vertices */
	const int idim = 6; /* 6 indices */
  GLfloat pos[2 * vdim] = {
    -.5, -.5,
    .5,  .5,
    .5,  -.5,
    -.5, .5,
  };

  GLfloat tex[2 * vdim] = {
    0, 0,
    1, 1,
    1, 0,
    0, 1,
  };

  GLuint indices[idim] = { 0, 2, 1, 0, 1, 3 };

  /* Bind vertex buffers */
  GLCall(glBindVertexArray(g.VAO));   // VAO is vertex array object added by DS August 8, 2024

  GLCall(glBindBuffer(GL_ARRAY_BUFFER, g.posVbo));
  GLCall(glBufferData(
    GL_ARRAY_BUFFER,
    2*vdim*sizeof(GLfloat),
    pos,
    GL_STATIC_DRAW));
  checkGlErrors();

  glBindBuffer(GL_ARRAY_BUFFER, g.texVbo);
  glBufferData(
    GL_ARRAY_BUFFER,
    2*vdim*sizeof(GLfloat),
    tex,
    GL_STATIC_DRAW);
  checkGlErrors();

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g.indexVbo);
  glBufferData(
	  GL_ELEMENT_ARRAY_BUFFER,
	  idim * sizeof(GLuint),
	  indices,
	  GL_STATIC_DRAW);
  
  checkGlErrors();
}

static void loadTriangleGeometry(const GeometryPX& g) {
	const int vdim = 3; /* 3 vertices */
	const int idim = 3; /* 3 indices */

  GLfloat pos[2 * vdim] = {
    0.0, -0.45,
    -0.45, 0.45,
    0.45, 0.45
  };

  /* Center shield in triangle */
  GLfloat tex[2 * vdim] = {
    0.5, -.60,
    -.35, 1.1,
    1.35, 1.1
  };

  /* Give each vertex a different color */
  GLfloat color[3 * vdim] = {
    1, 0, 0,
    0, 1, 0,
    0, 0, 1
  };

  GLuint indices[idim] = { 0, 2, 1};

  GLCall(glBindVertexArray(g.VAO));   // VAO is vertex array object added by DS August 8, 2024

  glBindBuffer(GL_ARRAY_BUFFER, g.posVbo);
  glBufferData(
    GL_ARRAY_BUFFER,
    2*vdim*sizeof(GLfloat),
    pos,
    GL_STATIC_DRAW);
  checkGlErrors();

  glBindBuffer(GL_ARRAY_BUFFER, g.texVbo);
  glBufferData(
    GL_ARRAY_BUFFER,
    2*vdim*sizeof(GLfloat),
    tex,
    GL_STATIC_DRAW);
  checkGlErrors();

  glBindBuffer(GL_ARRAY_BUFFER, g.colorVbo);
  glBufferData(
    GL_ARRAY_BUFFER,
    3*vdim*sizeof(GLfloat),
    color,
    GL_STATIC_DRAW);
  checkGlErrors();

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g.indexVbo);
  glBufferData(
      GL_ELEMENT_ARRAY_BUFFER,
      idim * sizeof(GLuint),
      indices,
      GL_STATIC_DRAW);
  checkGlErrors();

}

static void initGeometry() {
  g_square.reset(new GeometryPX());
  loadSquareGeometry(*g_square);

  g_triangle.reset(new GeometryPX());
  loadTriangleGeometry(*g_triangle);
}

static void loadTexture(GLuint texHandle, const char* ppmFilename) {
    int texWidth, texHeight;
    vector<PackedPixel> pixData;

    ppmRead(ppmFilename, texWidth, texHeight, pixData);

    // GLCall(glActiveTexture(GL_TEXTURE0));
    GLCall(glBindTexture(GL_TEXTURE_2D, texHandle));
    GLCall(glTexImage2D(GL_TEXTURE_2D, 0, g_Gl2Compatible ? GL_RGB : GL_SRGB, texWidth, texHeight,
        0, GL_RGB, GL_UNSIGNED_BYTE, &pixData[0]));

    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));

    checkGlErrors();
}

static void initTextures() {
    g_tex0.reset(new GlTexture());
    g_tex1.reset(new GlTexture());
    g_tex2.reset(new GlTexture());

    loadTexture(*g_tex0, "smiley.ppm");
    loadTexture(*g_tex1, "reachup.ppm");
    loadTexture(*g_tex2, "shield.ppm");
}


/* M A I N ************************************************************/

/**
 * Main
 *
 * The main entry-point for the HelloWorld example application.
 */
int main(int argc, char** argv) {
    try {
        initGLFW(argc, argv);

        glewInit(); // load the OpenGL extensions

        initGLState();
        initShaders();
        initGeometry();
        initTextures();

        GLFWwindow* window = glfwGetCurrentContext();
        while (!glfwWindowShouldClose(window)) {


            display(window);

            glfwPollEvents();
        }

        glfwDestroyWindow(window);
        glfwTerminate();
    }
    catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

