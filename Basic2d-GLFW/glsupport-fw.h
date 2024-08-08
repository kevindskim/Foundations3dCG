#ifndef GLSUPPORT_H
#define GLSUPPORT_H

#include <iostream>
#include <stdexcept>

#include <GL/glew.h>
#ifdef __MAC__
# include <GLUT/glut.h>
#else
# include <GL/glut.h>
#endif

#define ASSERT(x) if (!(x)) { __debugbreak(); }
#define GLCall(x) GLClearError();\
				  x;\
				  ASSERT(GLLogCall(#x, __FILE__, __LINE__))

void GLClearError();
bool GLLogCall(const char* function, const char* file, int line);

// Check if there has been an error inside OpenGL and if yes, print the error and
// through a runtime_error exception.
void checkGlErrors();

// Reads and compiles a pair of vertex shader and fragment shader files into a
// GL shader program. Throws runtime_error on error
void readAndCompileShader(GLuint programHandle,
                          const char *vertexShaderFileName, const char *fragmentShaderFileName);

// Read and compile a pair of vertex shader and fragment shader source codes from
// memory into a GL shader program. Throws runtime_error on error
void readAndCompileShaderFromMemory(GLuint programHandle,
                                    int vsSourceLength, const char *vsSource,
                                    int fsSourceLength, const char *fsSource);

// Link two compiled vertex shader and fragment shader into a GL shader program
void linkShader(GLuint programHandle, GLuint vertexShaderHandle, GLuint fragmentShaderHandle);

// Reads and compiles a single shader (vertex, fragment, etc) file into a GL
// shader. Throws runtime_error on error
void readAndCompileSingleShader(GLuint shaderHandle, const char* shaderFileName);

// Reads and compiles a single shader (vertex, fragment, etc) from memory into a GL
// shader. Throws runtime_error on error
void readAndCompileSingleShaderFromMemory(GLuint shaderHandle,
                                          int sourceLength, const char *source);


// Classes inheriting Noncopyable will not have default compiler generated copy
// constructor and assignment operator
class Noncopyable {
protected:
  Noncopyable() {}
  ~Noncopyable() {}
private:
  Noncopyable(const Noncopyable&);
  const Noncopyable& operator= (const Noncopyable&);
};

// Light wrapper around a GL shader (can be geometry/vertex/fragment shader)
// handle. Automatically allocates and deallocates. Can be casted to GLuint.
class GlShader : Noncopyable {
protected:
  GLuint handle_;

public:
  GlShader(GLenum shaderType) {
    handle_ = glCreateShader(shaderType); // create shader handle
    if (handle_ == 0)
      throw std::runtime_error("glCreateShader fails");
    checkGlErrors();
  }

  ~GlShader() {
    glDeleteShader(handle_);
  }

  // Casts to GLuint so can be used directly by glCompile etc
  operator GLuint() const {
    return handle_;
  }
};

// Light wrapper around GLSL program handle that automatically allocates
// and deallocates. Can be casted to a GLuint.
class GlProgram : Noncopyable {
protected:
  GLuint handle_;

public:
  GlProgram() {
    handle_ = glCreateProgram();
    if (handle_ == 0)
      throw std::runtime_error("glCreateProgram fails");
    checkGlErrors();
  }

  ~GlProgram() {
    glDeleteProgram(handle_);
  }

  // Casts to GLuint so can be used directly by glUseProgram and so on
  operator GLuint() const {
    return handle_;
  }
};


// Light wrapper around a GL texture object handle that automatically allocates
// and deallocates. Can be casted to a GLuint.
class GlTexture : Noncopyable {
protected:
    GLuint handle_;

public:
    GlTexture() {
        GLCall(glGenTextures(1, &handle_));
        if (handle_ == 0) {
            throw std::runtime_error("glGenTextures failed to generate texture.");
        }
        checkGlErrors();
    }

    ~GlTexture() {
        glDeleteTextures(1, &handle_);
    }

    // Cast to GLuint for direct use with OpenGL functions
    operator GLuint() const {
        return handle_;
    }
};


// Light wrapper around a GL buffer object handle that automatically allocates
// and deallocates. Can be casted to a GLuint.
class GlBufferObject : Noncopyable {
protected:
  GLuint handle_;

public:
  GlBufferObject() {
    GLCall(glGenBuffers(1, &handle_));
    checkGlErrors();
  }

  ~GlBufferObject() {
    glDeleteBuffers(1, &handle_);
  }

  // Casts to GLuint so can be used directly glBindBuffer and so on
  operator GLuint() const {
    return handle_;
  }
};

// Light wrapper around a GL Vertex Array object handle that automatically allocates
// and deallocates. Can be casted to a GLuint.
class GlVertexArrayObject : Noncopyable {
protected:
    GLuint handle_;

public:
    GlVertexArrayObject() {
        GLCall(glGenVertexArrays(1, &handle_));
        checkGlErrors();
    }

    ~GlVertexArrayObject() {
        glDeleteVertexArrays(1, &handle_);
    }

    // Casts to GLuint so can be used directly glBindBuffer and so on
    operator GLuint() const {
        return handle_;
    }
};


// Safe versions of various functions that handle GLSL shader attributes
// and variables: These mainly issue a warning when specified attributes
// and variables do not exist in the compiled GLSL program (e.g., due to
// driver optimization), and return without doing anything when the user
// tries to change these attributes or variables.
// 
// GLCall wraps the OpenGL function call and checks for errors by DS on August 6, 2024
//
inline GLint safe_glGetUniformLocation(const GLuint program, const char varname[]) {
  GLint r = glGetUniformLocation(program, varname);
  if (r < 0)
    std::cerr << "WARN: "<< varname << " cannot be bound (it either doesn't exist or has been optimized away). safe_glUniform calls will silently ignore it.\n" << std::endl;
  return r;
}

inline GLint safe_glGetAttribLocation(const GLuint program, const char varname[]) {
  GLint r = glGetAttribLocation(program, varname);
  if (r < 0)
    std::cerr << "WARN: "<< varname << " cannot be bound (it either doesn't exist or has been optimized away). safe_glAttrib calls will silently ignore it.\n" << std::endl;
  return r;
}

inline void safe_glUniformMatrix4fv(const GLint handle, const GLfloat data[]) {
  if (handle >= 0)
    GLCall(glUniformMatrix4fv(handle, 1, GL_FALSE, data));
}

inline void safe_glUniform1i(const GLint handle, const GLint a) {
  if (handle >= 0) 
    GLCall(glUniform1i(handle, a));   // GLCall is a macro that checks for errors
}

inline void safe_glUniform2i(const GLint handle, const GLint a, const GLint b) {
  if (handle >= 0)
    GLCall(glUniform2i(handle, a, b));
}

inline void safe_glUniform3i(const GLint handle, const GLint a, const GLint b, const GLint c) {
  if (handle >= 0)
    GLCall(glUniform3i(handle, a, b, c));
}

inline void safe_glUniform4i(const GLint handle, const GLint a, const GLint b, const GLint c, const GLint d) {
  if (handle >= 0)
    GLCall(glUniform4i(handle, a, b, c, d));
}

inline void safe_glUniform1f(const GLint handle, const GLfloat a) {
  if (handle >= 0)
    GLCall(glUniform1f(handle, a));
}

inline void safe_glUniform2f(const GLint handle, const GLfloat a, const GLfloat b) {
  if (handle >= 0)
    GLCall(glUniform2f(handle, a, b));
}

inline void safe_glUniform3f(const GLint handle, const GLfloat a, const GLfloat b, const GLfloat c) {
  if (handle >= 0)
    GLCall(glUniform3f(handle, a, b, c));
}

inline void safe_glUniform4f(const GLint handle, const GLfloat a, const GLfloat b, const GLfloat c, const GLfloat d) {
  if (handle >= 0)
    GLCall(glUniform4f(handle, a, b, c, d));
}

inline void safe_glEnableVertexAttribArray(const GLint handle) {
  if (handle >= 0)
    GLCall(glEnableVertexAttribArray(handle));
}

inline void safe_glDisableVertexAttribArray(const GLint handle) {
  if (handle >= 0)
    GLCall(glDisableVertexAttribArray(handle));
}

inline void safe_glVertexAttribPointer(const GLint handle, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid *pointer) {
  if (handle >= 0)
    GLCall(glVertexAttribPointer(handle, size, type, normalized, stride, pointer));
}

inline void safe_glVertexAttrib1f(const GLint handle, const GLfloat a) {
  if (handle >= 0)
    GLCall(glVertexAttrib1f(handle, a));
}

inline void safe_glVertexAttrib2f(const GLint handle, const GLfloat a, const GLfloat b) {
  if (handle >= 0)
    GLCall(glVertexAttrib2f(handle, a, b));
}

inline void safe_glVertexAttrib3f(const GLint handle, const GLfloat a, const GLfloat b, const GLfloat c) {
  if (handle >= 0)
    GLCall(glVertexAttrib3f(handle, a, b, c));
}

inline void safe_glVertexAttrib4f(const GLint handle, const GLfloat a, const GLfloat b, const GLfloat c, const GLfloat d) {
  if (handle >= 0)
    GLCall(glVertexAttrib4f(handle, a, b, c, d));
}

inline void safe_glVertexAttrib4Nub(const GLint handle, const GLubyte a, const GLubyte b, const GLubyte c, const GLubyte d) {
  if (handle >= 0)
    GLCall(glVertexAttrib4Nub(handle, a, b, c, d));
}


#endif
