#pragma once

// Using OpenGL 3.3
#include <glad/glad.h>
#include <../src/glad.c>

#include "OpenGL_Buffers.h"

internal void GLAPIENTRY OpenGLDebugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) 
{
	fprintf(stderr, "[GL DEBUG]: %s\n", message);
}

