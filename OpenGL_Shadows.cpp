#include "OpenGL_Shadows.h"

#define GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB					0x8242

#define WGL_CONTEXT_MAJOR_VERSION_ARB					0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB					0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB						0x2093
#define WGL_CONTEXT_FLAGS_ARB							0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB					0x9126

#define WGL_CONTEXT_DEBUG_BIT_ARB						0x0001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB			0x0002

#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB				0x0000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB		0x0000002

typedef HGLRC WINAPI FP_wglCreateContextAttribsARB(HDC hdc, HGLRC ShareContext, const int* attribList);

internal void OpenGL_Shadows_Initialize(opengl_shadows_test* Test, const game_window& Window)
{
	// Create opengl context to this thread
	HDC WindowDC = GetDC(Window.Handle);

	PIXELFORMATDESCRIPTOR DesiredPixelFormat = {};
	DesiredPixelFormat.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	DesiredPixelFormat.nVersion = 1;
	DesiredPixelFormat.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
	DesiredPixelFormat.iPixelType = PFD_TYPE_RGBA;
	DesiredPixelFormat.cColorBits = 32;
	DesiredPixelFormat.cAlphaBits = 8;
	DesiredPixelFormat.iLayerType = PFD_MAIN_PLANE;
	i32 SuggestedPixelFormatIndex = ChoosePixelFormat(WindowDC, &DesiredPixelFormat);

	PIXELFORMATDESCRIPTOR SuggestPixelFormat = {};
	DescribePixelFormat(WindowDC, SuggestedPixelFormatIndex, sizeof(SuggestPixelFormat), &SuggestPixelFormat);
	SetPixelFormat(WindowDC, SuggestedPixelFormatIndex, &SuggestPixelFormat);

	// Create context
	HGLRC LegacyOpenGLRC = wglCreateContext(WindowDC);
	Assert(wglMakeCurrent(WindowDC, LegacyOpenGLRC), "Error creating legacy opengl context");

	// Load additional stuff
	{
		FP_wglCreateContextAttribsARB* wglCreateContextAttribsARB = (FP_wglCreateContextAttribsARB*)wglGetProcAddress("wglCreateContextAttribsARB");
		HGLRC ShareContext = 0;
		int Attribs[] = {
			WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
			WGL_CONTEXT_MINOR_VERSION_ARB, 6,
			WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB | WGL_CONTEXT_DEBUG_BIT_ARB,
			WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
			0
		};
		HGLRC ModernContext = wglCreateContextAttribsARB(WindowDC, ShareContext, Attribs);
		Assert(ModernContext, "Failed to initialize modern opengl context!");
		wglMakeCurrent(WindowDC, ModernContext);
		wglDeleteContext(LegacyOpenGLRC);
	}

	// Load modern OpenGL stuff
	Assert(gladLoadGL(), "Failed to load OpenGL 4.6!");
	bool DebugOpenGL = true;
	if (DebugOpenGL)
	{
		int ContextFlags = 0;
		glGetIntegerv(GL_CONTEXT_FLAGS, &ContextFlags);
		if (ContextFlags & GL_CONTEXT_FLAG_DEBUG_BIT)
		{
			glEnable(GL_DEBUG_OUTPUT);
			glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
			glDebugMessageCallback(OpenGLDebugCallback, nullptr);
			glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE); // Enable all messages
		}

		if (GLAD_GL_KHR_debug) {
			// Set up debug callback

			int k = 0;
		}
	}

	// Initialize rendering
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

				OpenGL_IndexBuffer_Create(&Quad.IndexBuffer, QuadIndices, c_MaxQuadIndices);
			}
		}


		{

		}
	}
}

internal void OpenGL_Shadows_UpdateAndRender(opengl_shadows_test* Test, u32 Width, u32 Height)
{
	glViewport(0, 0, Width, Height);
	glClearColor(1.0f, 0, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
}