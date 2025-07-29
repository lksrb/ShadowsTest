#include "Shadows.h"

#include <vector>

internal void Shadows_Initialize(shadows_test* Test, d3d12_context* Context)
{
	auto Device = Context->Device;

	// Quad
	{
		//  Root Signature
		{
			D3D12_STATIC_SAMPLER_DESC Samplers[1] = {};

			// Sampler
			Samplers[0].Filter = D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
			Samplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
			Samplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
			Samplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
			Samplers[0].MipLODBias = 0;
			Samplers[0].MaxAnisotropy = 16;
			Samplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
			Samplers[0].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
			Samplers[0].MinLOD = 0.0f;
			Samplers[0].MaxLOD = D3D12_FLOAT32_MAX;
			Samplers[0].ShaderRegister = 0;
			Samplers[0].RegisterSpace = 0;
			Samplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

			D3D12_DESCRIPTOR_RANGE Ranges[1] = {};
			Ranges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
			Ranges[0].NumDescriptors = 1;
			Ranges[0].BaseShaderRegister = 0;
			Ranges[0].RegisterSpace = 0;
			Ranges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

			D3D12_ROOT_PARAMETER Parameters[3] = {};
			Parameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
			Parameters[0].Constants.Num32BitValues = sizeof(quad_root_signature_constant_buffer) / 4;
			Parameters[0].Constants.ShaderRegister = 0;  // b0
			Parameters[0].Constants.RegisterSpace = 0;
			Parameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

			// Light environment
			Parameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
			Parameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
			Parameters[1].Descriptor.ShaderRegister = 1; // b1
			Parameters[1].Descriptor.RegisterSpace = 0;

			// Descriptor table
			Parameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			Parameters[2].DescriptorTable.NumDescriptorRanges = CountOf(Ranges);
			Parameters[2].DescriptorTable.pDescriptorRanges = Ranges;
			Parameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

			D3D12_ROOT_SIGNATURE_DESC Desc = {};
			Desc.pParameters = Parameters;
			Desc.NumParameters = CountOf(Parameters);
			Desc.pStaticSamplers = Samplers;
			Desc.NumStaticSamplers = CountOf(Samplers);
			Desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

			// Root signature
			ID3DBlob* Error;
			ID3DBlob* Signature;
			DxAssert(D3D12SerializeRootSignature(&Desc, D3D_ROOT_SIGNATURE_VERSION_1, &Signature, &Error));
			DxAssert(Device->CreateRootSignature(0, Signature->GetBufferPointer(), Signature->GetBufferSize(), IID_PPV_ARGS(&Test->Quad.RootSignature)));
		}

		// Quad Pipeline
		{
			const wchar_t* ShaderPath = L"Quad.hlsl";
			// Define the vertex input layout.
			D3D12_INPUT_ELEMENT_DESC InputElementDescs[] =
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			};

			D3D12_GRAPHICS_PIPELINE_STATE_DESC PipelineDesc = {};

			// Rasterizer state
			PipelineDesc.RasterizerState = {};
			PipelineDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
			PipelineDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
			PipelineDesc.RasterizerState.FrontCounterClockwise = true;
			PipelineDesc.RasterizerState.DepthBias = 100;
			PipelineDesc.RasterizerState.DepthBiasClamp = 0;
			PipelineDesc.RasterizerState.SlopeScaledDepthBias = 2;
			PipelineDesc.RasterizerState.DepthClipEnable = true;
			PipelineDesc.RasterizerState.MultisampleEnable = FALSE;
			PipelineDesc.RasterizerState.AntialiasedLineEnable = FALSE;
			PipelineDesc.RasterizerState.ForcedSampleCount = 0;
			PipelineDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

			// Depth and stencil state
			PipelineDesc.DepthStencilState.DepthEnable = true;
			PipelineDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
			PipelineDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;  // Closer pixels are drawn
			PipelineDesc.DepthStencilState.StencilEnable = FALSE;  // Stencil disabled for now
			PipelineDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;  // Must match depth buffer format

			// Blend state
			PipelineDesc.BlendState = {};
			PipelineDesc.BlendState.AlphaToCoverageEnable = FALSE;
			PipelineDesc.BlendState.IndependentBlendEnable = FALSE;
			for (i32 i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
			{
				auto& Desc = PipelineDesc.BlendState.RenderTarget[i];
				Desc.BlendEnable = TRUE;
				Desc.SrcBlend = D3D12_BLEND_SRC_ALPHA;       // Use source alpha
				Desc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;  // 1 - source alpha
				Desc.BlendOp = D3D12_BLEND_OP_ADD;
				Desc.SrcBlendAlpha = D3D12_BLEND_ONE;
				Desc.DestBlendAlpha = D3D12_BLEND_ZERO;
				Desc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
				Desc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
			}

			PipelineDesc.InputLayout = { InputElementDescs, CountOf(InputElementDescs) };
			PipelineDesc.pRootSignature = Test->Quad.RootSignature;
			PipelineDesc.VS = CompileVertexShader(ShaderPath);
			PipelineDesc.PS = CompileFragmentShader(ShaderPath);
			PipelineDesc.SampleMask = UINT_MAX;
			PipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

			PipelineDesc.NumRenderTargets = 1;
			PipelineDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
			PipelineDesc.SampleDesc.Count = 1;

			DxAssert(Device->CreateGraphicsPipelineState(&PipelineDesc, IID_PPV_ARGS(&Test->Quad.Pipeline)));
		}

		// Vertex buffers and index buffers
		{
			for (u32 i = 0; i < FIF; i++)
			{
				Test->Quad.VertexBuffers[i] = DX12VertexBufferCreate(Device, sizeof(quad_vertex) * c_MaxQuadVertices);
			}

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
				Test->Quad.IndexBuffer = DX12IndexBufferCreate(Device, Context->DirectCommandAllocators[0], Context->DirectCommandList, Context->DirectCommandQueue, QuadIndices, c_MaxQuadIndices);
			}
		}
	}

	// Light environment
	{
		// Create light environment constant buffer for each frame
		for (u32 i = 0; i < FIF; i++)
		{
			Test->LightEnvironmentConstantBuffers[i] = DX12ConstantBufferCreate(Device, sizeof(light_environment));
		}
	}

	// Shadow Pass
	{
		// Root Signature
		{
			D3D12_ROOT_PARAMETER Parameters[1] = {};
			Parameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
			Parameters[0].Constants.Num32BitValues = sizeof(shadow_pass_root_signature_constant_buffer) / 4;
			Parameters[0].Constants.ShaderRegister = 0;  // b0
			Parameters[0].Constants.RegisterSpace = 0;
			Parameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

			D3D12_ROOT_SIGNATURE_DESC Desc = {};
			Desc.NumParameters = CountOf(Parameters);
			Desc.pParameters = Parameters;
			Desc.NumStaticSamplers = 0;
			Desc.pStaticSamplers = nullptr;
			Desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

			// Root signature
			ID3DBlob* Error;
			ID3DBlob* Signature;
			DxAssert(D3D12SerializeRootSignature(&Desc, D3D_ROOT_SIGNATURE_VERSION_1, &Signature, &Error));
			DxAssert(Device->CreateRootSignature(0, Signature->GetBufferPointer(), Signature->GetBufferSize(), IID_PPV_ARGS(&Test->ShadowPass.RootSignature)));
		}

		// ShadowPass Pipeline
		{
			const wchar_t* ShaderPath = L"Shadow.hlsl";
			// Define the vertex input layout.
			D3D12_INPUT_ELEMENT_DESC InputElementDescs[] =
			{
				{ "POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			};

			D3D12_GRAPHICS_PIPELINE_STATE_DESC PipelineDesc = {};

			// Rasterizer state
			PipelineDesc.RasterizerState = {};
			PipelineDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
			PipelineDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
			PipelineDesc.RasterizerState.FrontCounterClockwise = TRUE;
			PipelineDesc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
			PipelineDesc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
			PipelineDesc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
			PipelineDesc.RasterizerState.DepthClipEnable = true;
			PipelineDesc.RasterizerState.MultisampleEnable = FALSE;
			PipelineDesc.RasterizerState.AntialiasedLineEnable = FALSE;
			PipelineDesc.RasterizerState.ForcedSampleCount = 0;
			PipelineDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

			// Depth and stencil state
			PipelineDesc.DepthStencilState.DepthEnable = true;
			PipelineDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
			PipelineDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;  // Closer pixels are drawn
			PipelineDesc.DepthStencilState.StencilEnable = FALSE;  // Stencil disabled for now
			PipelineDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;  // Must match depth buffer format

			PipelineDesc.InputLayout = { InputElementDescs, CountOf(InputElementDescs) };
			PipelineDesc.pRootSignature = Test->ShadowPass.RootSignature;
			PipelineDesc.VS = CompileVertexShader(ShaderPath);
			PipelineDesc.PS = CompileFragmentShader(ShaderPath);
			PipelineDesc.SampleMask = UINT_MAX;
			PipelineDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			PipelineDesc.NumRenderTargets = 0;
			PipelineDesc.SampleDesc.Count = 1;

			DxAssert(Device->CreateGraphicsPipelineState(&PipelineDesc, IID_PPV_ARGS(&Test->ShadowPass.Pipeline)));
		}

		// Create resources
		// Create resources
		// Create resources

		// Our very own descriptor heaps
		{
			// Create the descriptor heap for the depth-stencil view.
			D3D12_DESCRIPTOR_HEAP_DESC HeapDesc = {};
			HeapDesc.NumDescriptors = FIF;
			HeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
			HeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
			DxAssert(Context->Device->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&Test->ShadowPass.DSVDescriptorHeap)));

			// SRV
			HeapDesc = {};
			HeapDesc.NumDescriptors = FIF;
			HeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
			HeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
			DxAssert(Context->Device->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&Test->ShadowPass.SRVDescriptorHeap)));
		}

		// Descriptor views
		{
			D3D12_CPU_DESCRIPTOR_HANDLE DsvHandle = Test->ShadowPass.DSVDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
			u32 DSVDescriptorSize = Context->Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

			// Create shadowmap buffers
			for (u32 i = 0; i < FIF; i++)
			{
				D3D12_CLEAR_VALUE OptimizedClearValue = {};
				OptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
				OptimizedClearValue.DepthStencil = { 1.0f, 0 };

				D3D12_RESOURCE_DESC DepthStencilDesc = {};
				DepthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
				DepthStencilDesc.Width = 1024;
				DepthStencilDesc.Height = 1024;
				DepthStencilDesc.DepthOrArraySize = 1;
				DepthStencilDesc.MipLevels = 1;
				DepthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
				DepthStencilDesc.SampleDesc.Count = 1;  // No MSAA
				DepthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

				D3D12_HEAP_PROPERTIES HeapProperties = {};
				HeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
				HeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
				HeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
				HeapProperties.CreationNodeMask = 1;
				HeapProperties.VisibleNodeMask = 1;
				DxAssert(Context->Device->CreateCommittedResource(
					&HeapProperties,
					D3D12_HEAP_FLAG_NONE,
					&DepthStencilDesc,
					D3D12_RESOURCE_STATE_GENERIC_READ,
					&OptimizedClearValue,
					IID_PPV_ARGS(&Test->ShadowPass.ShadowMaps[i])
				));

				const wchar_t* DebugNames[]{
					L"ShadowMap0",
					L"ShadowMap1"
				};

				Test->ShadowPass.ShadowMaps[i]->SetName(DebugNames[i]);

				// Update the depth-stencil view.
				D3D12_DEPTH_STENCIL_VIEW_DESC DSV = {};
				DSV.Format = DXGI_FORMAT_D32_FLOAT;
				DSV.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
				DSV.Texture2D.MipSlice = 0;
				DSV.Flags = D3D12_DSV_FLAG_NONE;

				Context->Device->CreateDepthStencilView(Test->ShadowPass.ShadowMaps[i], &DSV, DsvHandle);
				Test->ShadowPass.DSVHandles[i] = DsvHandle;
				DsvHandle.ptr += DSVDescriptorSize;
			}

			// Shadow map
			for (u32 i = 0; i < FIF; i++)
			{
				auto SRVDescriptor = Test->ShadowPass.SRVDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
				auto DescriptorSize = Context->Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
				{
					D3D12_SHADER_RESOURCE_VIEW_DESC Desc = {};
					Desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
					Desc.Format = DXGI_FORMAT_R32_FLOAT;
					Desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
					Desc.Texture2D.MipLevels = 1;
					Desc.Texture2D.MostDetailedMip = 0;
					Desc.Texture2D.PlaneSlice = 0;
					Desc.Texture2D.ResourceMinLODClamp = 0.0f;

					Context->Device->CreateShaderResourceView(Test->ShadowPass.ShadowMaps[i], &Desc, SRVDescriptor);
					Test->ShadowPass.SRVHandles[i] = SRVDescriptor;
					SRVDescriptor.ptr += DescriptorSize;
				}
			}
		}
	}
}

internal void PushCube(shadows_test* Shadows, const v3& Translation, const v3& Rotation, const v3& Scale)
{
	Assert(Shadows->Quad.IndexCount < c_MaxQuadIndices, "Shadows->QuadIndexCount < c_MaxQuadIndices");

	m4 Transform = glm::translate(m4(1.0f), Translation)
		* glm::toMat4(qtn(Rotation))
		* glm::scale(m4(1.0f), Scale);

	for (u32 i = 0; i < CountOf(c_CuboidVerticesPositions); i++)
	{
		Shadows->Quad.VertexDataPtr->Position = Transform * c_CuboidVerticesPositions[i];
		Shadows->Quad.VertexDataPtr->Color = c_CuboidVerticesColor[i];
		Shadows->Quad.VertexDataPtr->Normal = m3(glm::transpose(glm::inverse(Transform))) * c_CuboidVerticesNormals[i];
		Shadows->Quad.VertexDataPtr++;
	}

	Shadows->Quad.IndexCount += 36;
}

internal void PushDirectionalLight(shadows_test* Shadows, const v3& Direction, f32 Intensity, const v3& Radiance)
{
	directional_light& DirLight = Shadows->LightEnvironment.EmplaceDirectionalLight();
	DirLight.Direction = Direction;
	DirLight.Intensity = Intensity;
	DirLight.Radiance = Radiance;
}

internal void PushPointLight(shadows_test* Shadows, const v3& Position, f32 Radius, f32 FallOff, const v3& Radiance, f32 Intensity)
{
	point_light& Light = Shadows->LightEnvironment.EmplacePointLight();
	Light.Position = Position;
	Light.Radius = Radius;
	Light.FallOff = FallOff;
	Light.Radiance = Radiance;
	Light.Intensity = Intensity;
}

internal void CameraMovement(game_input* Input, v3* CameraPosition, v3* CameraRotation, v3* CameraForward, f32 TimeStep)
{
	// Rotating
	local_persist bool TPressed = false;

	bool JustPressed = false;
	bool JumpKeyPressed = false;

	if (Input->IsKeyPressed(key::T))
	{
		TPressed = !TPressed;

		JustPressed = true;
	}

	if (TPressed)
	{
		local_persist v2i OldMousePos;

		v2i MousePos = Input->GetRawMouseInput();

		// Avoids teleporting with camera rotation due to large delta
		if (JustPressed)
			OldMousePos = MousePos;

		v2i MouseDelta = MousePos - OldMousePos;

		//Trace("%i %i", MouseDelta.x, MouseDelta.y);
		OldMousePos = MousePos;

		f32 MouseSensitivity = 0.35f;

		// Update rotation based on mouse input
		CameraRotation->y += (f32)MouseDelta.x * MouseSensitivity * TimeStep; // Yaw
		CameraRotation->x += (f32)MouseDelta.y * MouseSensitivity * TimeStep; // Pitch
	}

	// Clamp pitch to avoid gimbal lock
	if (CameraRotation->x > glm::radians(89.0f))
		CameraRotation->x = glm::radians(89.0f);
	if (CameraRotation->x < glm::radians(-89.0f))
		CameraRotation->x = glm::radians(-89.0f);

	// Calculate the forward and right direction vectors
	v3 Up = qtn(v3(CameraRotation->x, CameraRotation->y, 0.0f)) * v3(0.0f, 1.0f, 0.0f);
	v3 Right = qtn(v3(CameraRotation->x, CameraRotation->y, 0.0f)) * v3(1.0f, 0.0f, 0.0f);
	v3 Forward = qtn(v3(CameraRotation->x, CameraRotation->y, 0.0f)) * v3(0.0f, 0.0f, 1.0f);
	f32 Speed = 10.0f;

	// Movement
	v3 Direction = {};
	if (TPressed)
	{
		if (Input->IsKeyDown(key::W))
		{
			Direction += v3(Forward.x, 0.0f, Forward.z);
		}

		if (Input->IsKeyDown(key::S))
		{
			Direction -= v3(Forward.x, 0.0f, Forward.z);
		}

		if (Input->IsKeyDown(key::A))
		{
			Direction -= Right;
		}

		if (Input->IsKeyDown(key::D))
		{
			Direction += Right;
		}

		if (Input->IsKeyPressed(key::BackSpace) || Input->IsKeyDown(key::Space))
		{
			JumpKeyPressed = true;
		}

		if (Input->IsKeyDown(key::Q) || Input->IsKeyDown(key::BackSpace) || Input->IsKeyDown(key::Space))
		{
			Direction += v3(0.0f, 1.0f, 0.0f);
		}
		else if (Input->IsKeyDown(key::E) || Input->IsKeyDown(key::Control) || Input->IsKeyDown(key::Shift))
		{
			Direction -= v3(0.0f, 1.0f, 0.0f);
		}

		if (glm::length(Direction) > 0.0f)
			Direction = glm::normalize(Direction);
	}

	*CameraPosition += Direction * Speed * TimeStep;
	*CameraForward = Forward;
}

internal void Shadows_Update(shadows_test* Shadows, game_input* Input, d3d12_context* Context, f32 TimeStep, f32 TimeSinceStart)
{
	local_persist v3 CameraPosition(0, 6, -10);
	local_persist v3 CameraRotation(glm::pi<f32>() / 4, 0, 0);
	local_persist v3 CameraForward;
	local_persist v3 Eye = v3(-2, 6, -2);
	//Eye.x += TimeStep;

	// Camera
	{
		CameraMovement(Input, &CameraPosition, &CameraRotation, &CameraForward, TimeStep);

		m4 LightSpaceMatrix(1.0f);

		// Shadows
		{
			f32 Size = 15;
			m4 LightSpaceProjection = glm::orthoLH_ZO(-Size, Size, -Size, Size, 0.1f, 40.5f);
			m4 LightSpaceView = glm::lookAtLH(Eye, v3(0, 0, 0), v3(0, 1, 0));

			LightSpaceMatrix = LightSpaceProjection * LightSpaceView;
		}

		//CameraPosition.x = 6 * bkm::Sin(TimeSinceStart);

		camera Camera;
		m4 InverseView = glm::translate(m4(1.0f), CameraPosition) * glm::toMat4(qtn(CameraRotation));
		Camera.View = glm::inverse(InverseView);
		Camera.RecalculateProjectionPerspective(2160, 1185);

		Shadows->Quad.RootSignatureBuffer.ViewProjection = Camera.GetViewProjection();
		Shadows->Quad.RootSignatureBuffer.View = Camera.View;

		Shadows->Quad.RootSignatureBuffer.LightSpaceMatrix = LightSpaceMatrix;

		// Copy
		Shadows->ShadowPass.RootSignatureBuffer.LightSpaceMatrix = Shadows->Quad.RootSignatureBuffer.LightSpaceMatrix;
	}

	// LIGHT
	PushDirectionalLight(Shadows, glm::normalize(v3(Eye.x, -Eye.y, Eye.z)), 1.0f, v3(1.0f));

	//PushPointLight(Shadows, v3(5.0f * bkm::Sin(0 * 5.0f), 1.0f, 0), 10.0, 1.0f, v3(1.0f), 2.0f);

	struct block
	{
		v3 Position;
	};

	local_persist std::vector<block> Blocks;

	if (Input->IsMousePressed(mouse::Left))
	{
		f32 Range = 5;
		auto& Block = Blocks.emplace_back();
		Block.Position = CameraPosition + CameraForward * Range;
	}

	for (size_t i = 0; i < Blocks.size(); i++)
	{
		PushCube(Shadows, Blocks[i].Position, v3(0, 0, 0), v3(1.0f, 1.0f, 1.0f));
	}

	// CUBE
	PushCube(Shadows, v3(0, 5, 0), v3(0, TimeSinceStart, 0), v3(1.0f, 1.0f, 1.0f));

	// GROUND
	PushCube(Shadows, v3(0, 0, 0), v3(0, 0, 0), v3(20.0f, 1.0f, 20.0f));
	//PushCube(Shadows, v3(10, 10, 0), v3(0, 0, glm::pi<f32>() / 2), v3(40.0f, 1.0f, 40.0f));
}

internal void Shadows_UpdateAndRender(shadows_test* Test, game_input* Input, d3d12_context* Context, f32 TimeStep, f32 TimeSinceStart)
{
	// Update game
	Shadows_Update(Test, Input, Context, TimeStep, TimeSinceStart);

	// Get current frame stuff
	auto CommandList = Context->DirectCommandList;
	auto CurrentBackBufferIndex = Context->CurrentBackBufferIndex;

	auto DirectCommandAllocator = Context->DirectCommandAllocators[CurrentBackBufferIndex];
	auto BackBuffer = Context->BackBuffers[CurrentBackBufferIndex];
	auto RTV = Context->RTVHandles[CurrentBackBufferIndex];
	auto DSV = Context->DSVHandles[CurrentBackBufferIndex];
	auto& FrameFenceValue = Context->FrameFenceValues[CurrentBackBufferIndex];

	// TODO: Figure out if this function needs to be called every frame.
	DXGI_SWAP_CHAIN_DESC SwapChainDesc;
	DxAssert(Context->SwapChain->GetDesc(&SwapChainDesc));

	// Reset state
	DxAssert(DirectCommandAllocator->Reset());

	DxAssert(CommandList->Reset(DirectCommandAllocator, nullptr));

	{
		// Set light environment data
		DX12ConstantBufferSetData(&Test->LightEnvironmentConstantBuffers[CurrentBackBufferIndex], &Test->LightEnvironment, sizeof(light_environment));

		// Send vertex data
		DX12VertexBufferSendData(&Test->Quad.VertexBuffers[CurrentBackBufferIndex], Context->DirectCommandList, Test->Quad.VertexDataBase, sizeof(quad_vertex) * Test->Quad.IndexCount);
	}

	// Shadow Pass
	if (1)
	{
		auto& ShadowPass = Test->ShadowPass;
		auto ShadowMap = ShadowPass.ShadowMaps[CurrentBackBufferIndex];

		// From resource to depth write
		DX12CmdTransition(CommandList, ShadowMap, D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_DEPTH_WRITE);
		DX12CmdSetViewport(CommandList, 0, 0, 1024.0f, 1024.0f);
		DX12CmdSetScissorRect(CommandList, 0, 0, 1024, 1024);

		auto ShadowPassDSV = ShadowPass.DSVHandles[CurrentBackBufferIndex];
		CommandList->ClearDepthStencilView(ShadowPassDSV, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
		CommandList->OMSetRenderTargets(0, nullptr, false, &ShadowPassDSV);
		CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Render instanced cuboids
		if (Test->Quad.IndexCount > 0)
		{
			CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			CommandList->SetPipelineState(ShadowPass.Pipeline);
			CommandList->SetGraphicsRootSignature(ShadowPass.RootSignature);

			// TODO: For now just share the first half of the signature buffer, this needs some sort of distinction between HUD and Game stuff
			CommandList->SetGraphicsRoot32BitConstants(0, sizeof(ShadowPass.RootSignatureBuffer) / 4, &ShadowPass.RootSignatureBuffer, 0);

			// Bind vertex buffer
			DX12CmdSetVertexBuffer(CommandList, 0, Test->Quad.VertexBuffers[CurrentBackBufferIndex].Buffer.Handle, Test->Quad.IndexCount * sizeof(quad_vertex), sizeof(quad_vertex));

			// Bind index buffer
			DX12CmdSetIndexBuffer(CommandList, Test->Quad.IndexBuffer.Buffer.Handle, Test->Quad.IndexCount * sizeof(u32), DXGI_FORMAT_R32_UINT);

			// Issue draw call
			CommandList->DrawIndexedInstanced(Test->Quad.IndexCount, 1, 0, 0, 0);
		}

		// From depth write to resource
		DX12CmdTransition(CommandList, ShadowMap, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_GENERIC_READ);
	}

	// Geometry and composition pass
	{
		// Frame that was presented needs to be set to render target again
		DX12CmdTransition(CommandList, BackBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

		// Set and clear render target view
		v4 ClearColor = { 0.2f, 0.3f, 0.8f, 1.0f };
		CommandList->ClearRenderTargetView(RTV, &ClearColor.x, 0, nullptr);
		CommandList->ClearDepthStencilView(DSV, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
		CommandList->OMSetRenderTargets(1, &RTV, false, &DSV);

		// Set viewport and scissor rect
		DX12CmdSetViewport(CommandList, 0, 0, (FLOAT)SwapChainDesc.BufferDesc.Width, (FLOAT)SwapChainDesc.BufferDesc.Height);
		DX12CmdSetScissorRect(CommandList, 0, 0, SwapChainDesc.BufferDesc.Width, SwapChainDesc.BufferDesc.Height);

		// Render quads
		if (Test->Quad.IndexCount > 0)
		{
			CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			CommandList->SetGraphicsRootSignature(Test->Quad.RootSignature);
			CommandList->SetPipelineState(Test->Quad.Pipeline);

			// 0
			CommandList->SetGraphicsRoot32BitConstants(0, sizeof(Test->Quad.RootSignatureBuffer) / 4, &Test->Quad.RootSignatureBuffer, 0);

			// 1
			CommandList->SetGraphicsRootConstantBufferView(1, Test->LightEnvironmentConstantBuffers[CurrentBackBufferIndex].Buffer.Handle->GetGPUVirtualAddress());

			// 2
			{
				CommandList->SetDescriptorHeaps(1, (ID3D12DescriptorHeap* const*)&Test->ShadowPass.SRVDescriptorHeap);
				auto SRVPTR = Test->ShadowPass.SRVDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
				//SRVPTR.ptr += CurrentBackBufferIndex * Context->Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
				CommandList->SetGraphicsRootDescriptorTable(2, SRVPTR);
			}

			// Bind vertex buffer
			DX12CmdSetVertexBuffer(CommandList, 0, Test->Quad.VertexBuffers[CurrentBackBufferIndex].Buffer.Handle, Test->Quad.IndexCount * sizeof(quad_vertex), sizeof(quad_vertex));

			// Bind index buffer
			DX12CmdSetIndexBuffer(CommandList, Test->Quad.IndexBuffer.Buffer.Handle, Test->Quad.IndexCount * sizeof(u32), DXGI_FORMAT_R32_UINT);

			// Issue draw call
			CommandList->DrawIndexedInstanced(Test->Quad.IndexCount, 1, 0, 0, 0);
		}

		// Rendered frame needs to be transitioned to present state
		DX12CmdTransition(CommandList, BackBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	}

	// Finalize the command list
	DxAssert(CommandList->Close());

	Context->DirectCommandQueue->ExecuteCommandLists(1, (ID3D12CommandList* const*)&CommandList);

	const bool VSync = true;
	DxAssert(Context->SwapChain->Present(VSync, 0));

	// Wait for GPU to finish presenting
	FrameFenceValue = D3D12Context_Signal(Context->DirectCommandQueue, Context->Fence, &Context->FenceValue);
	D3D12Context_WaitForFenceValue(Context->Fence, FrameFenceValue, Context->DirectFenceEvent);

	// Move to another back buffer
	Context->CurrentBackBufferIndex = Context->SwapChain->GetCurrentBackBufferIndex();

	// RESET STATE

	 // Reset indices
	Test->Quad.IndexCount = 0;
	Test->Quad.VertexDataPtr = Test->Quad.VertexDataBase;

	Test->LightEnvironment.Clear();
}