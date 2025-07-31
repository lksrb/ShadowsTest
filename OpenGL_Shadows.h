#pragma once

#include "Shadows.h"

// Using OpenGL 3.3
#include <glad/glad.h>
#include <../src/glad.c>

struct opengl_shadows_test
{
	HDC WindowDC;
	HGLRC OpenGLRC;

	struct
	{
		u32 VertexArrayHandle;
		u32 VertexBufferHandle;
		u32 IndexBufferHandle;
		quad_vertex* VertexDataBase;
		quad_vertex* VertexDataPtr;
		u32 IndexCount;
	} Quad;
};

internal void OpenGL_Shadows_Initialize(opengl_shadows_test* Test, const game_window& Window);
internal void OpenGL_Shadows_InitializePipeline(opengl_shadows_test* Test, const game_window& Window);
internal void OpenGL_Shadows_UpdateAndRender(opengl_shadows_test* Test, u32 Width, u32 Height);