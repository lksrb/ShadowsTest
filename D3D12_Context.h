// DirectX 12 Header File
// This file contains everything to get DirectX12 building including some extra components like DirectXMath.h or d3dx12.h
#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3d12shader.h>
#include <dxcapi.h>
#include <DirectXMath.h>
using namespace DirectX;

#pragma comment(lib, "dxcompiler.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

// D3D12 extension
//#include "d3dx12.h"

// TODO: Better approach?
inline ID3D12InfoQueue* g_DebugInfoQueue; // Created by GameRenderer
internal void DX12DumpInfoQueue(ID3D12InfoQueue* InfoQueue);
#define DumpInfoQueue() DX12DumpInfoQueue(g_DebugInfoQueue)
#define DxAssert(x) do { HRESULT __Result = x; if (FAILED(__Result)) { DumpInfoQueue(); Assert(false, #x); } } while(0)
#define FIF 2

#if _DEBUG
#define DX12_ENABLE_DEBUG_LAYER 1
#else
#define DX12_ENABLE_DEBUG_LAYER 0
#endif

#if DX12_ENABLE_DEBUG_LAYER
#include <dxgidebug.h>
#include <d3d12sdklayers.h>

#pragma comment(lib, "dxguid.lib")
#endif

internal D3D12_RESOURCE_BARRIER DX12CmdTransition(ID3D12GraphicsCommandList* CommandList, ID3D12Resource* Resource, D3D12_RESOURCE_STATES StateBefore, D3D12_RESOURCE_STATES StateAfter, UINT Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES, D3D12_RESOURCE_BARRIER_FLAGS Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE)
{
	D3D12_RESOURCE_BARRIER Result;
	Result.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	Result.Flags = Flags;
	Result.Transition.pResource = Resource;
	Result.Transition.StateBefore = StateBefore;
	Result.Transition.StateAfter = StateAfter;
	Result.Transition.Subresource = Subresource;

	CommandList->ResourceBarrier(1, &Result);
	return Result;
};

internal void DX12CmdSetViewport(ID3D12GraphicsCommandList* CommandList, f32 TopLeftX, f32 TopLeftY, f32 Width, f32 Height, f32 MinDepth = D3D12_MIN_DEPTH, f32 MaxDepth = D3D12_MAX_DEPTH)
{
	D3D12_VIEWPORT Viewport;
	Viewport.TopLeftX = TopLeftX;
	Viewport.TopLeftY = TopLeftY;
	Viewport.Width = Width;
	Viewport.Height = Height;
	Viewport.MinDepth = MinDepth;
	Viewport.MaxDepth = MaxDepth;
	CommandList->RSSetViewports(1, &Viewport);
}

internal void DX12CmdSetScissorRect(ID3D12GraphicsCommandList* CommandList, i32 Left, i32 Top, i32 Right, i32 Bottom)
{
	D3D12_RECT ScissorRect;
	ScissorRect.left = Left;
	ScissorRect.top = Top;
	ScissorRect.right = Right;
	ScissorRect.bottom = Bottom;
	CommandList->RSSetScissorRects(1, &ScissorRect);
}

internal void DX12CmdSetIndexBuffer(ID3D12GraphicsCommandList* CommandList, ID3D12Resource* BufferHandle, u32 SizeInBytes, DXGI_FORMAT Format)
{
	// Bind index buffer
	local_persist D3D12_INDEX_BUFFER_VIEW IndexBufferView;
	IndexBufferView.BufferLocation = BufferHandle->GetGPUVirtualAddress();
	IndexBufferView.SizeInBytes = SizeInBytes;
	IndexBufferView.Format = Format;
	CommandList->IASetIndexBuffer(&IndexBufferView);
}

internal void DX12CmdSetVertexBuffer(ID3D12GraphicsCommandList* CommandList, u32 StartSlot, ID3D12Resource* BufferHandle, u32 SizeInBytes, u32 StrideInBytes)
{
	local_persist D3D12_VERTEX_BUFFER_VIEW VertexBufferView;
	VertexBufferView.BufferLocation = BufferHandle->GetGPUVirtualAddress();
	VertexBufferView.SizeInBytes = SizeInBytes;
	VertexBufferView.StrideInBytes = StrideInBytes;
	CommandList->IASetVertexBuffers(StartSlot, 1, &VertexBufferView);
}

internal void DX12CmdSetVertexBuffers2(ID3D12GraphicsCommandList* CommandList, u32 StartSlot, ID3D12Resource* BufferHandle0, u32 SizeInBytes0, u32 StrideInBytes0, ID3D12Resource* BufferHandle1, u32 SizeInBytes1, u32 StrideInBytes1)
{
	// Bind vertex positions
	local_persist D3D12_VERTEX_BUFFER_VIEW VertexBufferViews[2];
	VertexBufferViews[0].BufferLocation = BufferHandle0->GetGPUVirtualAddress();
	VertexBufferViews[0].SizeInBytes = SizeInBytes0;
	VertexBufferViews[0].StrideInBytes = StrideInBytes0;

	// Bind transforms
	VertexBufferViews[1].BufferLocation = BufferHandle1->GetGPUVirtualAddress();
	VertexBufferViews[1].SizeInBytes = SizeInBytes1;
	VertexBufferViews[1].StrideInBytes = StrideInBytes1;

	CommandList->IASetVertexBuffers(0, 2, VertexBufferViews);
}

// Blocking API for submitting stuff to GPU
// NOTE: Inefficient
template<typename F>
internal void DX12SubmitToQueueImmidiate(ID3D12Device* Device, ID3D12CommandAllocator* CommandAllocator, ID3D12GraphicsCommandList* CommandList, ID3D12CommandQueue* CommandQueue, F&& Func)
{
	// Reset
	DxAssert(CommandAllocator->Reset());
	DxAssert(CommandList->Reset(CommandAllocator, nullptr));

	// Record stuff
	Func(CommandList);

	// Finish recording
	CommandList->Close();

	// Execute command list
	CommandQueue->ExecuteCommandLists(1, (ID3D12CommandList* const*)&CommandList);

	// Wait for completion
	{
		UINT64 FenceValue = 0;
		HANDLE FenceEvent = CreateEvent(nullptr, false, false, nullptr);

		ID3D12Fence* Fence;

		// Create a simple fence for synchronization
		DxAssert(Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fence)));

		DxAssert(CommandQueue->Signal(Fence, ++FenceValue));

		// Step 3: Wait until the GPU reaches the fence
		if (SUCCEEDED(Fence->SetEventOnCompletion(FenceValue, FenceEvent)))
		{
			WaitForSingleObject(FenceEvent, INFINITE);
		}
		CloseHandle(FenceEvent);

		Fence->Release();
	}
}

internal void DX12DumpInfoQueue(ID3D12InfoQueue* InfoQueue)
{
	if (!InfoQueue)
		return;

	for (UINT64 i = 0; i < InfoQueue->GetNumStoredMessages(); ++i)
	{
		SIZE_T MessageLength = 0;

		// Get the length of the message
		HRESULT HR = InfoQueue->GetMessage(i, nullptr, &MessageLength);
		if (FAILED(HR))
		{
			Warn("Failed to get message length: HRESULT = 0x%08X", HR);
			continue;
		}

		// Allocate memory for the message
		auto Message = static_cast<D3D12_MESSAGE*>(alloca(MessageLength));
		if (!Message)
		{
			Warn("Failed to allocate memory for message.");
			continue;
		}

		// Retrieve the message
		HR = InfoQueue->GetMessage(i, Message, &MessageLength);
		if (FAILED(HR))
		{
			Warn("Failed to get message: HRESULT = 0x%08X", HR);
			continue;
		}

		switch (Message->Severity)
		{
		case D3D12_MESSAGE_SEVERITY_MESSAGE:
		{
			Trace("[DX12]%s", Message->pDescription);
			break;
		}
		case D3D12_MESSAGE_SEVERITY_INFO:
		{
			Info("[DX12]%s", Message->pDescription);
			break;
		}
		case D3D12_MESSAGE_SEVERITY_WARNING:
		{
			Warn("[DX12]%s", Message->pDescription);
			break;
		}
		case D3D12_MESSAGE_SEVERITY_ERROR:
		case D3D12_MESSAGE_SEVERITY_CORRUPTION:
		{
			Err("[DX12]%s", Message->pDescription);
			break;
		}
		}
	}

	// Optionally, clear the messages from the queue
	InfoQueue->ClearStoredMessages();
}

struct d3d12_context 
{
	ID3D12Device2* Device;
	IDXGIFactory4* Factory;

	ID3D12Debug3* DebugInterface; // 3 is maximum for Windows 10 build 19045

	// This is not defined in release mode
#if _DEBUG
	IDXGIDebug1* DxgiDebugInterface;
#endif

	// SWAPCHAIN
	// SWAPCHAIN
	// SWAPCHAIN
	ID3D12CommandAllocator* DirectCommandAllocators[FIF];
	ID3D12GraphicsCommandList2* DirectCommandList;
	ID3D12CommandQueue* DirectCommandQueue;

	IDXGISwapChain4* SwapChain;
	ID3D12Resource* BackBuffers[FIF];
	ID3D12DescriptorHeap* RTVDescriptorHeap;
	D3D12_CPU_DESCRIPTOR_HANDLE RTVHandles[FIF];

	u32 CurrentBackBufferIndex;

	ID3D12Resource* DepthBuffers[FIF];
	ID3D12DescriptorHeap* DSVDescriptorHeap;
	D3D12_CPU_DESCRIPTOR_HANDLE DSVHandles[FIF];

	// Fence
	ID3D12Fence* Fence;
	u64 FenceValue;
	u64 FrameFenceValues[FIF];
	HANDLE DirectFenceEvent;
};

internal void D3D12Context_Initialize(d3d12_context* Context, const game_window& Window)
{
	// Enable debug layer
	if (DX12_ENABLE_DEBUG_LAYER)
	{
		// Enable debug layer
		// This is sort of like validation layers in Vulkan
		DxAssert(D3D12GetDebugInterface(IID_PPV_ARGS(&Context->DebugInterface)));
		Context->DebugInterface->EnableDebugLayer();

#if _DEBUG
		// Enable DXGI debug - for more memory leaks checking
		DxAssert(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&Context->DxgiDebugInterface)));
		Context->DxgiDebugInterface->EnableLeakTrackingForThread();
#endif
	}

	// Create device

	// This takes ~300 ms, insane
	DxAssert(D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&Context->Device)));

	// Create factory
	DxAssert(CreateDXGIFactory2(DX12_ENABLE_DEBUG_LAYER ? DXGI_CREATE_FACTORY_DEBUG : 0, IID_PPV_ARGS(&Context->Factory)));

	// Debug info queue
	if (DX12_ENABLE_DEBUG_LAYER)
	{
		DxAssert(Context->Device->QueryInterface(IID_PPV_ARGS(&g_DebugInfoQueue)));
		g_DebugInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, false);
		g_DebugInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, false);
		g_DebugInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, false);
		g_DebugInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_INFO, false);
		//InfoQueue->Release();
		//Renderer->DebugInterface->Release();

		// Suppressing some warning
		{
			// Suppress whole categories of messages
			//D3D12_MESSAGE_CATEGORY Categories[] = {};

			// Suppress messages based on their severity level
			D3D12_MESSAGE_SEVERITY Severities[] =
			{
				D3D12_MESSAGE_SEVERITY_INFO
			};

			// Suppress individual messages by their ID
			D3D12_MESSAGE_ID DenyIds[] = {
				D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,   // I'm really not sure how to avoid this message.
				D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,                         // This warning occurs when using capture frame while graphics debugging.
				D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,                       // This warning occurs when using capture frame while graphics debugging.
			};

			D3D12_INFO_QUEUE_FILTER NewFilter = {};
			//NewFilter.DenyList.NumCategories = _countof(Categories);
			//NewFilter.DenyList.pCategoryList = Categories;
			NewFilter.DenyList.NumSeverities = CountOf(Severities);
			NewFilter.DenyList.pSeverityList = Severities;
			NewFilter.DenyList.NumIDs = CountOf(DenyIds);
			NewFilter.DenyList.pIDList = DenyIds;

			DxAssert(g_DebugInfoQueue->PushStorageFilter(&NewFilter));
		}
	}

	// Create direct command queue
	{
		// Create direct command allocator
		for (u32 i = 0; i < FIF; i++)
		{
			DxAssert(Context->Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&Context->DirectCommandAllocators[i])));
		}

		D3D12_COMMAND_QUEUE_DESC Desc = {};
		Desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; // Most general - For draw, compute and copy commands
		Desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_HIGH;
		Desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		Desc.NodeMask = 0;
		DxAssert(Context->Device->CreateCommandQueue(&Desc, IID_PPV_ARGS(&Context->DirectCommandQueue)));

		// Create direct command list
		{
			DxAssert(Context->Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, Context->DirectCommandAllocators[Context->CurrentBackBufferIndex], nullptr, IID_PPV_ARGS(&Context->DirectCommandList)));

			// Command lists are created in the recording state, but there is nothing
			// to record yet. The main loop expects it to be closed, so close it now.
			DxAssert(Context->DirectCommandList->Close());
		}

		// Fence
		DxAssert(Context->Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Context->Fence)));
		Context->DirectFenceEvent = CreateEvent(nullptr, false, false, nullptr);
		Assert(Context->DirectFenceEvent != INVALID_HANDLE_VALUE, "Could not create fence event.");
	}

	// Create swapchain
	{
		DXGI_SWAP_CHAIN_DESC1 Desc = {};
		Desc.Width = Window.ClientAreaWidth;
		Desc.Height = Window.ClientAreaHeight;
		Desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		Desc.Stereo = false;
		Desc.SampleDesc = { 1, 0 }; // Anti-aliasing needs to be done manually in D3D12
		Desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		Desc.BufferCount = FIF;
		Desc.Scaling = DXGI_SCALING_STRETCH;
		Desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		Desc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
		// It is recommended to always allow tearing if tearing support is available.
		// TODO: More robustness needed
		Desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

		DXGI_SWAP_CHAIN_FULLSCREEN_DESC FullScreenDesc = {};
		FullScreenDesc.Windowed = true;

		IDXGISwapChain1* SwapChain1;
		HRESULT Result = (Context->Factory->CreateSwapChainForHwnd(Context->DirectCommandQueue, Window.Handle, &Desc, &FullScreenDesc, nullptr, &SwapChain1));

		DumpInfoQueue();
		// Disable the Alt+Enter fullscreen toggle feature. Switching to fullscreen
		// will be handled manually.
		DxAssert(Context->Factory->MakeWindowAssociation(Window.Handle, DXGI_MWA_NO_ALT_ENTER));
		SwapChain1->QueryInterface(IID_PPV_ARGS(&Context->SwapChain));
		SwapChain1->Release();

		//Renderer->SwapChain->SetMaximumFrameLatency(FIF);

		Context->CurrentBackBufferIndex = Context->SwapChain->GetCurrentBackBufferIndex();
	}

	// Views are descriptors located in the GPU memory.
	// They describe how to memory of particular resource is layed out

	// Create Render Target Views
	{
		// Create a Descriptor Heap
		D3D12_DESCRIPTOR_HEAP_DESC Desc = {};
		Desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		Desc.NumDescriptors = FIF;
		Desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		Desc.NodeMask = 0;
		DxAssert(Context->Device->CreateDescriptorHeap(&Desc, IID_PPV_ARGS(&Context->RTVDescriptorHeap)));

		// Place rtv descriptor sequentially in memory
		u32 RTVDescriptorSize = Context->Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		D3D12_CPU_DESCRIPTOR_HANDLE RtvHandle = Context->RTVDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		for (u32 i = 0; i < FIF; ++i)
		{
			DxAssert(Context->SwapChain->GetBuffer(i, IID_PPV_ARGS(&Context->BackBuffers[i])));
			Context->BackBuffers[i]->SetName(L"SwapchainRenderTargetTexture");
			Context->Device->CreateRenderTargetView(Context->BackBuffers[i], nullptr, RtvHandle);

			Context->RTVHandles[i] = RtvHandle;
			RtvHandle.ptr += RTVDescriptorSize;
		}
	}

	// Create depth resources
	{
		// Create the descriptor heap for the depth-stencil view.
		D3D12_DESCRIPTOR_HEAP_DESC DsvHeapDesc = {};
		DsvHeapDesc.NumDescriptors = FIF;
		DsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		DsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		DxAssert(Context->Device->CreateDescriptorHeap(&DsvHeapDesc, IID_PPV_ARGS(&Context->DSVDescriptorHeap)));

		D3D12_CPU_DESCRIPTOR_HANDLE DsvHandle = Context->DSVDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
		u32 DSVDescriptorSize = Context->Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

		// Create depth buffers
		for (u32 i = 0; i < FIF; i++)
		{
			D3D12_CLEAR_VALUE OptimizedClearValue = {};
			OptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
			OptimizedClearValue.DepthStencil = { 1.0f, 0 };

			D3D12_HEAP_PROPERTIES HeapProperties = {};
			HeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

			D3D12_RESOURCE_DESC DepthStencilDesc = {};
			DepthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
			DepthStencilDesc.Width = Window.ClientAreaWidth;
			DepthStencilDesc.Height = Window.ClientAreaHeight;
			DepthStencilDesc.DepthOrArraySize = 1;
			DepthStencilDesc.MipLevels = 1;
			DepthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
			DepthStencilDesc.SampleDesc = { 1, 0 };
			DepthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

			DxAssert(Context->Device->CreateCommittedResource(
				&HeapProperties,
				D3D12_HEAP_FLAG_NONE,
				&DepthStencilDesc,
				D3D12_RESOURCE_STATE_DEPTH_WRITE,
				&OptimizedClearValue,
				IID_PPV_ARGS(&Context->DepthBuffers[i])
			));

			// Create depth-stencil view
			{
				D3D12_DEPTH_STENCIL_VIEW_DESC Desc = {};
				Desc.Format = DXGI_FORMAT_D32_FLOAT;
				Desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
				Desc.Texture2D.MipSlice = 0;
				Desc.Flags = D3D12_DSV_FLAG_NONE;

				Context->Device->CreateDepthStencilView(Context->DepthBuffers[i], &Desc, DsvHandle);

				// Increment by the size of the dsv descriptor size
				Context->DSVHandles[i] = DsvHandle;
				DsvHandle.ptr += DSVDescriptorSize;
			}
		}
	}

	Trace("D3D12 Context initialized!");
}

//internal void D3D12Context_Render(d3d12_context* Render)

internal void D3D12Context_Shutdown(d3d12_context* Renderer)
{
#if DX12_ENABLE_DEBUG_LAYER
	// Report all memory leaks
	Renderer->DxgiDebugInterface->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_ALL | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
	DX12DumpInfoQueue(g_DebugInfoQueue);
#endif

	// Zero everything to make sure nothing can reference this
	memset(Renderer, 0, sizeof(Renderer));
}

internal u64 D3D12Context_Signal(ID3D12CommandQueue* CommandQueue, ID3D12Fence* Fence, u64* FenceValue)
{
	u64& RefFenceValue = *FenceValue;
	u64 FenceValueForSignal = ++RefFenceValue;
	DxAssert(CommandQueue->Signal(Fence, FenceValueForSignal));
	return FenceValueForSignal;
}

internal void D3D12Context_WaitForFenceValue(ID3D12Fence* Fence, u64 FenceValue, HANDLE FenceEvent, u32 Duration = UINT32_MAX)
{
	if (Fence->GetCompletedValue() < FenceValue)
	{
		Fence->SetEventOnCompletion(FenceValue, FenceEvent);
		WaitForSingleObject(FenceEvent, Duration);
	}
}
