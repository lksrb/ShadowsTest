#pragma once

#include "Shadows.h"
#include "D3D12_Buffers.h"

#include <vector>

struct d3d12_shadows_test
{
	// Quad
	struct
	{
		ID3D12PipelineState* Pipeline;
		dx12_index_buffer IndexBuffer;
		dx12_vertex_buffer VertexBuffers[FIF];
		quad_vertex* VertexDataBase;
		quad_vertex* VertexDataPtr;
		u32 IndexCount;
		ID3D12RootSignature* RootSignature;
		quad_root_signature_constant_buffer RootSignatureBuffer;
	} Quad;

	// Light stuff
	light_environment LightEnvironment;
	dx12_constant_buffer LightEnvironmentConstantBuffers[FIF];

	// Shadows
	struct
	{
		ID3D12Resource* ShadowMaps[FIF];
		D3D12_CPU_DESCRIPTOR_HANDLE DSVHandles[FIF];
		D3D12_CPU_DESCRIPTOR_HANDLE SRVHandles[FIF];
		ID3D12PipelineState* Pipeline;
		ID3D12RootSignature* RootSignature;
		ID3D12DescriptorHeap* DSVDescriptorHeap;
		ID3D12DescriptorHeap* SRVDescriptorHeap;
		shadow_pass_root_signature_constant_buffer RootSignatureBuffer;
	} ShadowPass;
};

// Helpers
D3D12_SHADER_BYTECODE CompileVertexShader(const wchar_t* Path)
{
#if defined(_DEBUG)
	LPCWSTR Arguments[] = {
		L"-T", L"vs_6_0",  // Shader profile
		L"-E", L"VSMain", // Entry point
		L"-Zi",            // Debug info
		L"-Qembed_debug",  // Embed debug info
		L"-IResources"
	};
#else
	LPCWSTR Arguments[] = {
		L"-T", L"vs_6_0",  // Shader profile
		L"-E", L"VSMain",
		 L"-IResources" // Entry point
		 //L"-Zi",            // Debug info
		 //L"-Qembed_debug",  // Embed debug info
	};
#endif

	IDxcBlob* VertexShader = nullptr;

	// Initialize DXC
	IDxcCompiler3* Compiler;
	IDxcLibrary* Library;
	IDxcIncludeHandler* IncludeHandler;
	DxAssert(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&Compiler)));
	DxAssert(DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&Library)));
	DxAssert(Library->CreateIncludeHandler(&IncludeHandler));

	IDxcBlobEncoding* SourceShader;
	DxAssert(Library->CreateBlobFromFile(Path, nullptr, &SourceShader));

	DxcBuffer Buffer = {};

	BOOL Known;
	SourceShader->GetEncoding(&Known, &Buffer.Encoding);
	Buffer.Ptr = SourceShader->GetBufferPointer();
	Buffer.Size = SourceShader->GetBufferSize();

	IDxcResult* Result;
	DxAssert(Compiler->Compile(&Buffer, Arguments, CountOf(Arguments), IncludeHandler, IID_PPV_ARGS(&Result)));

	HRESULT ErrorCode;
	Result->GetStatus(&ErrorCode);

	if (FAILED(ErrorCode))
	{
		IDxcBlobEncoding* ErrorMessage = nullptr;

		Result->GetErrorBuffer(&ErrorMessage);

		Err("%s", (const char*)ErrorMessage->GetBufferPointer());
		Assert(false, "");
	}
	else
	{
		Result->GetResult(&VertexShader);
	}

	return { VertexShader->GetBufferPointer(), VertexShader->GetBufferSize() };
}


D3D12_SHADER_BYTECODE CompileFragmentShader(const wchar_t* Path)
{
	IDxcBlob* PixelShader = nullptr;

#if defined(_DEBUG)
	LPCWSTR Arguments[] = {
	   L"-T", L"ps_6_0",  // Shader profile
	   L"-E", L"PSMain", // Entry point
	   L"-Zi",            // Debug info
	   L"-Qembed_debug",  // Embed debug info
	   L"-IResources"
	};
#else
	LPCWSTR Arguments[] = {
	   L"-T", L"ps_6_0",  // Shader profile
	   L"-E", L"PSMain", // Entry point
	   L"-IResources"
	   //L"-Zi",            // Debug info
	   //L"-Qembed_debug",  // Embed debug info
	};
#endif
	// Initialize DXC
	IDxcCompiler3* Compiler;
	IDxcLibrary* Library;
	IDxcIncludeHandler* IncludeHandler;
	DxAssert(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&Compiler)));
	DxAssert(DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&Library)));
	DxAssert(Library->CreateIncludeHandler(&IncludeHandler));

	IDxcBlobEncoding* SourceShader;
	DxAssert(Library->CreateBlobFromFile(Path, nullptr, &SourceShader));

	DxcBuffer Buffer = {};

	BOOL Known;
	SourceShader->GetEncoding(&Known, &Buffer.Encoding);
	Buffer.Ptr = SourceShader->GetBufferPointer();
	Buffer.Size = SourceShader->GetBufferSize();

	IDxcResult* Result;
	DxAssert(Compiler->Compile(&Buffer, Arguments, CountOf(Arguments), IncludeHandler, IID_PPV_ARGS(&Result)));

	HRESULT ErrorCode;
	Result->GetStatus(&ErrorCode);

	if (FAILED(ErrorCode))
	{
		IDxcBlobEncoding* ErrorMessage = nullptr;

		Result->GetErrorBuffer(&ErrorMessage);

		Err("%s", (const char*)ErrorMessage->GetBufferPointer());
		Assert(false, "");
	}
	else
	{
		Result->GetResult(&PixelShader);
	}

	return { PixelShader->GetBufferPointer(), PixelShader->GetBufferSize() };
}

internal std::vector<glm::vec4> getFrustumCornersWorldSpace(const glm::mat4& proj, const glm::mat4& view)
{
	const auto inv = glm::inverse(proj * view);

	std::vector<glm::vec4> frustumCorners;
	for (unsigned int x = 0; x < 2; ++x)
	{
		for (unsigned int y = 0; y < 2; ++y)
		{
			for (unsigned int z = 0; z < 2; ++z)
			{
				const glm::vec4 pt =
					inv * glm::vec4(
						2.0f * x - 1.0f,
						2.0f * y - 1.0f,
						2.0f * z - 1.0f,
						1.0f);
				frustumCorners.push_back(pt / pt.w);
			}
		}
	}

	return frustumCorners;
}