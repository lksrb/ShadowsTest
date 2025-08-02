#pragma once

#include "Shadows.h"

#include "OpenGL.h"

struct opengl_shadows_test
{
	struct
	{
		u32 VertexArrayHandle;
		u32 VertexBufferHandle;

		opengl_index_buffer IndexBuffer;
		quad_vertex* VertexDataBase;
		quad_vertex* VertexDataPtr;
		u32 IndexCount;
	} Quad;
};

internal void OpenGL_Shadows_Initialize(opengl_shadows_test* Test, const game_window& Window);
internal void OpenGL_Shadows_InitializePipeline(opengl_shadows_test* Test, const game_window& Window);
internal void OpenGL_Shadows_UpdateAndRender(opengl_shadows_test* Test, u32 Width, u32 Height);