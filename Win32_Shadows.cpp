#include "Win32_Shadows.h"

#include "D3D12_Context.h"

#include "Shadows.h"
#include "Shadows.cpp"

int main()
{
	Trace("Hello, Blocky!");

	// If the application is not DPI aware, Windows will automatically scale the pixels to a DPI scale value (150% for example)
	// So if the resolution is 3840×2160, the application window client area would be 2560×1440, so Windows scales that defaultly.
	// By settings this, Windows will no longer be able to scale pixels resulting in sharper image.
	// Note that DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 is for Windows build version > 1607,
	// so we need to add something if this failes
	SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	// Creates and shows the window
	game_window Window = CreateGameWindow();
	g_ClientWidth = Window.ClientAreaWidth;
	g_ClientHeight = Window.ClientAreaHeight;

	// Get current system time and use its value to initialize seed for our random
	FILETIME FileTime;
	GetSystemTimePreciseAsFileTime(&FileTime);

	// Show window after initialization
	ShowWindow(Window.Handle, SW_SHOW);

	// NOTE: This value represent how many increments of performance counter is happening
	LARGE_INTEGER CounterFrequency;
	QueryPerformanceFrequency(&CounterFrequency);

	LARGE_INTEGER LastCounter;
	QueryPerformanceCounter(&LastCounter);

	// How many cycles per some work done by CPU elapsed?
  // Not super accurate since the OS can insert some other instructions (scheduler)
	DWORD64 LastCycleCount = __rdtsc();

	bool IsMinimized = false;
	f32 TimeStep = 0.0f;
	g_IsRunning = true;
	game_input Input = {};

	d3d12_context* D3D12Context = VmAllocArray(d3d12_context, 1);
	D3D12Context_Initialize(D3D12Context, Window);

	shadows_test* Shadows = VmAllocArray(shadows_test, 1);
	Shadows_Initialize(Shadows, D3D12Context);

	f32 TimeSinceStart = 0.0f;

	while (g_IsRunning)
	{
		//ScopedTimer timer("Frame");

		// Process events
		Win32ProcessEvents(&Input, Window);

		// Only reliable way of knowing if we are minimized or not
		IsMinimized = IsIconic(Window.Handle);

		if (g_DoResize)
		{
			g_DoResize = false;

			//GameRendererResizeSwapChain(&GameRenderer, g_ClientWidth, g_ClientHeight);
		}

		if (!IsMinimized)
		{
			Shadows_UpdateAndRender(Shadows, &Input, D3D12Context, TimeStep, TimeSinceStart);
			//ScopedTimer timer("Game update");
			//GameUpdate(&Game, &GameRenderer, &Input, TimeStep, g_ClientWidth, g_ClientHeight);
		}

		// Render stuff

		DWORD64 EndCycleCount = __rdtsc();

		// Timestep
		LARGE_INTEGER EndCounter;
		QueryPerformanceCounter(&EndCounter);

		LONGLONG CounterElapsed = EndCounter.QuadPart - LastCounter.QuadPart;
		TimeStep = CounterElapsed / (f32)CounterFrequency.QuadPart;
		LastCounter = EndCounter;

		LONGLONG FPS = CounterFrequency.QuadPart / CounterElapsed;

		DWORD64 CyclesElapsed = EndCycleCount - LastCycleCount;
		LastCycleCount = EndCycleCount;

		local_persist f32 EverySecond = 1.0f;

		// Display it on the title bar
		if (EverySecond >= 1.0f)
		{
			EverySecond = 0.0f;
			char Title[128];
			sprintf_s(Title, "Shadows | TimeStep: %.3f ms | FPS: %d | CycleCount: %d", TimeStep * 1000.0f, (i32)FPS, (i32)CyclesElapsed);

			SetWindowTextA(Window.Handle, Title);
		}
		else
		{
			EverySecond += TimeStep;
		}

		// Clamp Timestep to atleast 60 fps to preserve physics simulation and update simulation aswell.
		TimeStep = glm::clamp(TimeStep, 0.0f, 0.01666666f);

		TimeSinceStart += TimeStep;

		// D3D12 debug queue dump
		DumpInfoQueue();
	}

	D3D12Context_Shutdown(D3D12Context);
}
