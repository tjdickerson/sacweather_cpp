//

#ifndef __tjd_gl_h__

#include <windows.h>
#include "gl/gl.h"

#define WGL_CONTEXT_MAJOR_VERSION_ARB     0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB     0x2092

#define WGL_CONTEXT_PROFILE_MASK_ARB      0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB  0x00000001

#define GL_ARRAY_BUFFER                   0x8892
#define GL_STATIC_DRAW                    0x88E4
#define GL_DYNAMIC_DRAW                   0x88E8

#define GL_FRAGMENT_SHADER                0x8B30
#define GL_VERTEX_SHADER                  0x8B31

// OpenGL Types
typedef char GLchar;

typedef signed long long int GLsizeiptr;
typedef signed long long int GLintptr;


// extensions
// verify this type of thing works @todo
#if defined(_WIN32)
#define GLGetProcAddress wglGetProcAddress
#else
#define GLGetProcAddress 0
#endif

// proc declarations
typedef HGLRC (WINAPI* PFNWGLCREATECONTEXTATTRIBSARBPROC) 
(HDC hDC, HGLRC hShareContext, const int* attribList);
HGLRC WINAPI wglCreateContextAttribsARB (HDC hDC, HGLRC hShareContext, const int* attribList);

typedef void (APIENTRY* PFNGLGENVERTEXARRAYSPROC) 
(GLsizei n, GLuint* arrays);

typedef void (APIENTRY* PFNGLBINDVERTEXARRAYPROC) 
(GLuint array);

typedef void (APIENTRY* PFNGLGENBUFFERSPROC) 
(GLsizei n, GLuint* buffers);

typedef void (APIENTRY* PFNGLBINDBUFFERPROC) 
(GLenum target, GLuint buffer);

typedef void (APIENTRY* PFNGLBUFFERDATAPROC) 
(GLenum target, GLsizeiptr size, const void* data, GLenum usage);

typedef GLint (APIENTRY* PFNGLGETATTRIBLOCATIONPROC) 
(GLuint program, const GLchar *name);

typedef void (APIENTRY* PFNGLENABLEVERTEXATTRIBARRAYPROC) 
(GLuint index);

typedef void (APIENTRY* PFNGLVERTEXATTRIBPOINTERPROC) 
(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer);

typedef GLint (APIENTRY* PFNGLGETUNIFORMLOCATIONPROC) 
(GLuint program, const GLchar* name);

typedef GLuint (APIENTRY* PFNGLCREATESHADERPROC) 
(GLenum type);

typedef void (APIENTRY* PFNGLSHADERSOURCEPROC) 
(GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length);

typedef void (APIENTRY* PFNGLCOMPILESHADERPROC) 
(GLuint shader);

typedef void (APIENTRY* PFNGLGETSHADERINFOLOGPROC) 
(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog);

typedef GLuint (APIENTRY* PFNGLCREATEPROGRAMPROC) 
(void);

typedef void (APIENTRY* PFNGLATTACHSHADERPROC) 
(GLuint program, GLuint shader);

typedef void (APIENTRY* PFNGLBINDFRAGDATALOCATIONPROC) 
(GLuint program, GLuint color, const GLchar* name);

typedef void (APIENTRY* PFNGLLINKPROGRAMPROC) 
(GLuint program);

typedef void (APIENTRY* PFNGLUSEPROGRAMPROC) 
(GLuint program);

typedef void (APIENTRY* PFNGLMULTIDRAWARRAYSPROC) 
(GLenum mode, const GLint *first, const GLsizei *count, GLsizei drawcount);

typedef void (APIENTRY* PFNGLUNIFORMMATRIX4FVPROC) 
(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);

typedef void (APIENTRY* PFNGLBUFFERSUBDATAPROC) 
(GLenum target, GLintptr offset, GLsizeiptr size, const void *data);

typedef void (APIENTRY* PFNGLUNIFORM3FPROC) 
(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);

typedef void (APIENTRY* PFNGLUNIFORM4FPROC) 
(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);

typedef void (APIENTRY* PFNGLACTIVETEXTUREPROC) 
(GLenum texture);



#define __tjd_gl_h__
#endif