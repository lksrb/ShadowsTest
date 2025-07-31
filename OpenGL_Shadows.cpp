#include "OpenGL_Shadows.h"

#include <glad/glad.h>

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
	Assert(wglMakeCurrent(Test->WindowDC, Test->OpenGLRC), "Error creating opengl context");
	OpenGL_Shadows_InitializePipeline(Test, Window);
}

internal void OpenGL_Shadows_InitializePipeline(opengl_shadows_test* Test, const game_window& Window)
{
	auto& Quad = Test->Quad;

	{
		// Vertex buffers and index buffers
		{
			for (u32 i = 0; i < FIF; i++)
			{
				//Test->Quad.VertexBuffers[i] = DX12VertexBufferCreate(Device, sizeof(quad_vertex) * c_MaxQuadVertices);
			}

			glGenBuffers(1, &Quad.VertexBufferHandle);
			glBindBuffer(GL_ARRAY_BUFFER, Quad.VertexBufferHandle);
			glBufferData(GL_ARRAY_BUFFER, c_MaxQuadVertices, nullptr, GL_DYNAMIC_DRAW);

			Test->Quad.VertexDataBase = VmAllocArray(quad_vertex, c_MaxQuadVertices);
			Test->Quad.VertexDataPtr = Test->Quad.VertexDataBase;

			// Quad Index buffer
			{
				u32* QuadIndices = VmAllocArray(u32, c_MaxQuadIndices);
				u32 Offset = 0;
				for (u32 i = 0; i < c_MaxQuadIndices; i += 6)
				{
					QuadIndices[i + 0] = Offset + 0;
					QuadIndices[i + 1] = Offset + 1;
					QuadIndices[i + 2] = Offset + 2;

					QuadIndices[i + 3] = Offset + 2;
					QuadIndices[i + 4] = Offset + 3;
					QuadIndices[i + 5] = Offset + 0;

					Offset += 4;
				}
				//Test->Quad.IndexBuffer = DX12IndexBufferCreate(Device, Context->DirectCommandAllocators[0], Context->DirectCommandList, Context->DirectCommandQueue, QuadIndices, c_MaxQuadIndices);
			}
		}
	}
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