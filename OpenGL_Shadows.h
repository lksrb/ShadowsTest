#pragma once

#include "Shadows.h"

#include <gl/GL.h>

struct opengl_shadows_test 
{
	HDC WindowDC;
	HGLRC OpenGLRC;

	quad_vertex* VertexDataBase;
	quad_vertex* VertexDataPtr;
	u32 IndexCount;
};

internal void OpenGL_Shadows_Initialize(opengl_shadows_test* Test, const game_window& Window)
{
	// Create opengl context to this thread
	Test->WindowDC = GetDC(Window.Handle);

	PIXELFORMATDESCRIPTOR DesiredPixelFormat = {};
	DesiredPixelFormat.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	DesiredPixelFormat.nVersion = 1;
	DesiredPixelFormat.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
	DesiredPixelFormat.iPixelType = PFD_TYPE_RGBA;
	DesiredPixelFormat.cColorBits = 32;
	DesiredPixelFormat.cAlphaBits = 8;
	DesiredPixelFormat.iLayerType = PFD_MAIN_PLANE;
	i32 SuggestedPixelFormatIndex = ChoosePixelFormat(Test->WindowDC, &DesiredPixelFormat);

	PIXELFORMATDESCRIPTOR SuggestPixelFormat = {};
	DescribePixelFormat(Test->WindowDC, SuggestedPixelFormatIndex, sizeof(SuggestPixelFormat), &SuggestPixelFormat);
	SetPixelFormat(Test->WindowDC, SuggestedPixelFormatIndex, &SuggestPixelFormat);

	// Create context
	Test->OpenGLRC = wglCreateContext(Test->WindowDC);
	Assert(wglMakeCurrent(Test->WindowDC, Test->OpenGLRC), "");
}

internal void OpenGL_Shadows_UpdateAndRender(opengl_shadows_test* Test, u32 Width, u32 Height)
{
	glViewport(0, 0, Width, Height);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glClearColor(1.0f, 0, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glBegin(GL_TRIANGLES);
	f32 P = 0.9f;

	glVertex2f(-P, -P);
	glVertex2f(P, -P);
	glVertex2f(P, P);
			 
	glVertex2f(-P, -P);
	glVertex2f(P, P);
	glVertex2f(-P, P);
	glEnd();

	SwapBuffers(Test->WindowDC);
}