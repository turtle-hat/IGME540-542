#include "Game.h"

#include <DirectXMath.h>

#include "BufferStructs.h"
#include "Graphics.h"
// This code assumes files are in "ImGui" subfolder!
// Adjust as necessary for your own folder structure and project setup
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx12.h"
#include "ImGui/imgui_impl_win32.h"
#include "Input.h"
#include "PathHelpers.h"
#include "Window.h"
#include "Vertex.h"

#include "ShaderConstants.hlsli"


// Starter code provided by Professor Chris Cascioli

// Needed for a helper function to load pre-compiled shader files
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>

// For the DirectX Math library
using namespace DirectX;
using namespace std;

// --------------------------------------------------------
// Called once per program, after the window and graphics API
// are initialized but before the game loop begins
// --------------------------------------------------------
void Game::Initialize()
{
	isInitialized = false;
	
	InitializeParameters();
	CreateRootSigAndPipelineState();
	CreateCameras();
	CreateMaterials();
	CreateGeometry();
	CreateLights();

	// Game is now fully initialized
	isInitialized = true;
}


// --------------------------------------------------------
// Clean up memory or objects created by this class
// 
// Note: Using smart pointers means there probably won't
//       be much to manually clean up here!
// --------------------------------------------------------
Game::~Game()
{
	// Wait for the GPU before we shut down
	Graphics::WaitForGPU();

	// ImGui clean up
	/*ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();*/
}

// --------------------------------------------------------
// Loads the two basic shaders, then creates the root signature
// and pipeline state object for our very basic demo.
// --------------------------------------------------------
void Game::CreateRootSigAndPipelineState()
{
	// Blobs to hold raw shader byte code used in several steps below
	Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderByteCode;
	Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderByteCode;

	// Load shaders
	{
		// Read our compiled vertex shader code into a blob
		// - Essentially just "open the file and plop its contents here"
		D3DReadFileToBlob(
			FixPath(L"VS_PBR.cso").c_str(),
			vertexShaderByteCode.GetAddressOf());
		D3DReadFileToBlob(
			FixPath(L"PS_PBR.cso").c_str(),
			pixelShaderByteCode.GetAddressOf());
	}

	// Input layout
	const unsigned int inputElementCount = 4;
	D3D12_INPUT_ELEMENT_DESC inputElements[inputElementCount] = {};
	{
		inputElements[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		inputElements[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;	// R32 G32 B32 = float3
		inputElements[0].SemanticName = "POSITION";				// Name must match semantic in shader
		inputElements[0].SemanticIndex = 0;						// This is the first POSITION semantic

		inputElements[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		inputElements[1].Format = DXGI_FORMAT_R32G32_FLOAT;		// R32 G32 = float2
		inputElements[1].SemanticName = "TEXCOORD";
		inputElements[1].SemanticIndex = 0;						// This is the first TEXCOORD semantic

		inputElements[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		inputElements[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;	// R32 G32 B32 = float3
		inputElements[2].SemanticName = "NORMAL";
		inputElements[2].SemanticIndex = 0;						// This is the first NORMAL semantic

		inputElements[3].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		inputElements[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;	// R32 G32 B32 = float3
		inputElements[3].SemanticName = "TANGENT";
		inputElements[3].SemanticIndex = 0;						// This is the first TANGENT semantic
	}

	// Root Signature
	{
		// Describe the range of CBVs needed for the vertex shader
		D3D12_DESCRIPTOR_RANGE cbvRangeVS = {};
		cbvRangeVS.RangeType							= D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		cbvRangeVS.NumDescriptors						= 1;
		cbvRangeVS.BaseShaderRegister					= 0;
		cbvRangeVS.RegisterSpace						= 0;
		cbvRangeVS.OffsetInDescriptorsFromTableStart	= D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		// Describe the range of CBVs needed for the pixel shader
		D3D12_DESCRIPTOR_RANGE cbvRangePS = {};
		cbvRangePS.RangeType							= D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		cbvRangePS.NumDescriptors						= 1;
		cbvRangePS.BaseShaderRegister					= 0;
		cbvRangePS.RegisterSpace						= 0;
		cbvRangePS.OffsetInDescriptorsFromTableStart	= D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		// Create a range of SRV's for textures
		D3D12_DESCRIPTOR_RANGE srvRange = {};
		srvRange.RangeType							= D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		srvRange.NumDescriptors						= 4; // Set to max number of textures at once (match pixel shader!)
		srvRange.BaseShaderRegister					= 0; // Starts at s0 (match pixel shader!)
		srvRange.RegisterSpace						= 0;
		srvRange.OffsetInDescriptorsFromTableStart	= D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		// Create the root parameters
		D3D12_ROOT_PARAMETER rootParams[3] = {};

		// CBV table param for vertex shader
		rootParams[0].ParameterType							= D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParams[0].ShaderVisibility						= D3D12_SHADER_VISIBILITY_VERTEX;
		rootParams[0].DescriptorTable.NumDescriptorRanges	= 1;
		rootParams[0].DescriptorTable.pDescriptorRanges		= &cbvRangeVS;

		// CBV table param for pixel shader
		rootParams[1].ParameterType							= D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParams[1].ShaderVisibility						= D3D12_SHADER_VISIBILITY_PIXEL;
		rootParams[1].DescriptorTable.NumDescriptorRanges	= 1;
		rootParams[1].DescriptorTable.pDescriptorRanges		= &cbvRangePS;

		// SRV table param
		rootParams[2].ParameterType							= D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParams[2].ShaderVisibility						= D3D12_SHADER_VISIBILITY_PIXEL;
		rootParams[2].DescriptorTable.NumDescriptorRanges	= 1;
		rootParams[2].DescriptorTable.pDescriptorRanges		= &srvRange;

		// Create a single static sampler (available to all pixel shaders at the same slot)
		D3D12_STATIC_SAMPLER_DESC anisoWrap = {};
		anisoWrap.AddressU			= D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		anisoWrap.AddressV			= D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		anisoWrap.AddressW			= D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		anisoWrap.Filter			= D3D12_FILTER_ANISOTROPIC;
		anisoWrap.MaxAnisotropy		= 16;
		anisoWrap.MaxLOD			= D3D12_FLOAT32_MAX;
		anisoWrap.ShaderRegister	= 0; // register(s0)
		anisoWrap.ShaderVisibility	= D3D12_SHADER_VISIBILITY_PIXEL;
		D3D12_STATIC_SAMPLER_DESC samplers[] = { anisoWrap };

		// Describe the full root signature
		D3D12_ROOT_SIGNATURE_DESC rootSig = {};
		rootSig.Flags				= D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
		rootSig.NumParameters		= ARRAYSIZE(rootParams);
		rootSig.pParameters			= rootParams;
		rootSig.NumStaticSamplers	= ARRAYSIZE(samplers);
		rootSig.pStaticSamplers		= samplers;

		ID3DBlob* serializedRootSig = 0;
		ID3DBlob* errors = 0;

		D3D12SerializeRootSignature(
			&rootSig,
			D3D_ROOT_SIGNATURE_VERSION_1,
			&serializedRootSig,
			&errors);

		// Check for errors during serialization
		if (errors != 0)
		{
			OutputDebugString((wchar_t*)errors->GetBufferPointer());
		}

		// Actually create the root sig
		Graphics::Device->CreateRootSignature(
			0,
			serializedRootSig->GetBufferPointer(),
			serializedRootSig->GetBufferSize(),
			IID_PPV_ARGS(rootSignature.GetAddressOf()));
	}

	// Pipeline state
	{
		// Describe the pipeline state
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};

		// -- Input assembler related ---
		psoDesc.InputLayout.NumElements = inputElementCount;
		psoDesc.InputLayout.pInputElementDescs = inputElements;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		// Overall primitive topology type (triangle, line, etc.) is set here
		// IASetPrimTop() is still used to set list/strip/adj options

		// Root sig
		psoDesc.pRootSignature = rootSignature.Get();

		// -- Shaders (VS/PS) ---
		psoDesc.VS.pShaderBytecode	= vertexShaderByteCode->GetBufferPointer();
		psoDesc.VS.BytecodeLength	= vertexShaderByteCode->GetBufferSize();
		psoDesc.PS.pShaderBytecode	= pixelShaderByteCode->GetBufferPointer();
		psoDesc.PS.BytecodeLength	= pixelShaderByteCode->GetBufferSize();

		// -- Render targets ---
		psoDesc.NumRenderTargets	= 1;
		psoDesc.RTVFormats[0]		= DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.DSVFormat			= DXGI_FORMAT_D24_UNORM_S8_UINT;
		psoDesc.SampleDesc.Count	= 1;
		psoDesc.SampleDesc.Quality	= 0;

		// -- States ---
		psoDesc.RasterizerState.FillMode		= D3D12_FILL_MODE_SOLID;
		psoDesc.RasterizerState.CullMode		= D3D12_CULL_MODE_BACK;
		psoDesc.RasterizerState.DepthClipEnable	= true;

		psoDesc.DepthStencilState.DepthEnable		= true;
		psoDesc.DepthStencilState.DepthFunc			= D3D12_COMPARISON_FUNC_LESS;
		psoDesc.DepthStencilState.DepthWriteMask	= D3D12_DEPTH_WRITE_MASK_ALL;

		psoDesc.BlendState.RenderTarget[0].SrcBlend					= D3D12_BLEND_ONE;
		psoDesc.BlendState.RenderTarget[0].DestBlend				= D3D12_BLEND_ZERO;
		psoDesc.BlendState.RenderTarget[0].BlendOp					= D3D12_BLEND_OP_ADD;
		psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask	= D3D12_COLOR_WRITE_ENABLE_ALL;

		// -- Misc ---
		psoDesc.SampleMask = 0xffffffff;

		// Create the pipe state object
		Graphics::Device->CreateGraphicsPipelineState(
			&psoDesc,
			IID_PPV_ARGS(pipelineState.GetAddressOf()));
	}

	// Set up the viewport and scissor rectangle
	{
		// Set up the viewport so we render into the correct
		// portion of the render target
		viewport = {};
		viewport.TopLeftX	= 0;
		viewport.TopLeftY	= 0;
		viewport.Width		= (float)Window::Width();
		viewport.Height		= (float)Window::Height();
		viewport.MinDepth	= 0.0f;
		viewport.MaxDepth	= 1.0f;

		// Define a scissor rectangle that defines a portion of
		// the render target for clipping. This is different from
		// a viewport in that it is applied after the pixel shader.
		// We need at least one of these, but we're rendering to
		// the entire window, so it'll be the same size.
		scissorRect = {};
		scissorRect.left	= 0;
		scissorRect.top		= 0;
		scissorRect.right	= Window::Width();
		scissorRect.bottom	= Window::Height();
	}
}

// --------------------------------------------------------
// Handle resizing to match the new window size
//  - Eventually, we'll want to update our 3D camera
// --------------------------------------------------------
void Game::OnResize()
{
	// Resize the viewport and scissor rectangle
	{
		// Set up the viewport so we render into the correct
		// portion of the render target
		viewport = {};
		viewport.TopLeftX	= 0;
		viewport.TopLeftY	= 0;
		viewport.Width		= (float)Window::Width();
		viewport.Height		= (float)Window::Height();
		viewport.MinDepth	= 0.0f;
		viewport.MaxDepth	= 1.0f;

		// Define a scissor rectangle that defines a portion of
		// the render target for clipping. This is different from
		// a viewport in that it is applied after the pixel shader.
		// We need at least one of these, but we're rendering to
		// the entire window, so it'll be the same size.
		scissorRect = {};
		scissorRect.left	= 0;
		scissorRect.top		= 0;
		scissorRect.right	= Window::Width();
		scissorRect.bottom	= Window::Height();
	}

	if (isInitialized) {
		// Resize camera
		if (cameras.size() > 0) {
			cameras[cameraCurrent]->SetAspect((Window::Width() + 0.0f) / Window::Height());
		}
	}
}


// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	// Example input checking: Quit if the escape key is pressed
	if (Input::KeyDown(VK_ESCAPE))
		Window::Quit();

	// Update current camera
	cameras[cameraCurrent]->Update(deltaTime);

	// Rotate meshes
	for (unsigned int i = 0; i < entities.size(); i++) {
		entities[i]->GetTransform()->Rotate(0.0f, deltaTime, 0.0f);
	}
}


// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Grab the current back buffer for this frame
	Microsoft::WRL::ComPtr<ID3D12Resource> currentBackBuffer =
		Graphics::BackBuffers[Graphics::SwapChainIndex()];

	// Clearing the render target
	{
		// Transition the back buffer from present to render target
		D3D12_RESOURCE_BARRIER rb = {};
		rb.Type						= D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		rb.Flags					= D3D12_RESOURCE_BARRIER_FLAG_NONE;
		rb.Transition.pResource		= currentBackBuffer.Get();
		rb.Transition.StateBefore	= D3D12_RESOURCE_STATE_PRESENT;
		rb.Transition.StateAfter	= D3D12_RESOURCE_STATE_RENDER_TARGET;
		rb.Transition.Subresource	= D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		Graphics::CommandList->ResourceBarrier(1, &rb);

		// Background color (Cornflower Blue in this case) for clearing
		float color[] = { 0.4f, 0.6f, 0.75f, 1.0f };
		
		// Clear the RTV
		Graphics::CommandList->ClearRenderTargetView(
			Graphics::RTVHandles[Graphics::SwapChainIndex()],
			color,
			0, 0); // No scissor rectangles

		// Clear the depth buffer, too
		Graphics::CommandList->ClearDepthStencilView(
			Graphics::DSVHandle,
			D3D12_CLEAR_FLAG_DEPTH,
			1.0f,	// Max depth = 1.0f
			0,		// Not clearing stencil, but need a value
			0, 0);	// No scissor rects
	}

	// Rendering here!
	{
		// Set overall pipeline state
		Graphics::CommandList->SetPipelineState(pipelineState.Get());

		// Root sig (must happen before root descriptor table)
		Graphics::CommandList->SetGraphicsRootSignature(rootSignature.Get());
		// Set descriptor heap for CBVs
		Graphics::CommandList->SetDescriptorHeaps(1,
			Graphics::CBVSRVDescriptorHeap.GetAddressOf());

		// Set up other commands for rendering
		Graphics::CommandList->OMSetRenderTargets(
			1,
			&Graphics::RTVHandles[Graphics::SwapChainIndex()],
			true,
			&Graphics::DSVHandle);
		Graphics::CommandList->RSSetViewports(1, &viewport);
		Graphics::CommandList->RSSetScissorRects(1, &scissorRect);
		Graphics::CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);



		// RENDER SCENE

		// Grab the current Camera
		std::shared_ptr<Camera> camera = cameras[cameraCurrent];

		// Collect base vertex shader data that will be used for all Entities
		VertexShaderExternalData vsBaseData = {};
		vsBaseData.view			= camera->GetViewMatrix();
		vsBaseData.projection	= camera->GetProjectionMatrix();
		// Collect base pixel shader data that will be used for all Entities
		PixelShaderExternalData psBaseData = {};
		psBaseData.cameraPosition = camera->GetTransform()->GetPosition();
		psBaseData.lightCount = lights.size();
		memcpy(psBaseData.lights, &lights[0], sizeof(Light) * MAX_LIGHTS);

		// Loop through and render all Entities
		for (unsigned int i = 0; i < entities.size(); i++) {

			// Grab the Entity's Mesh and Material
			std::shared_ptr<Mesh> mesh = entities[i]->GetMesh();
			std::shared_ptr<Material> material = entities[i]->GetMaterial();

			// Set pipeline state for Material
			Graphics::CommandList->SetPipelineState(material->GetPipelineState().Get());

			// Set the SRV descriptor handle for this material's textures
			// Note: This assumes that descriptor table 2 is for textures (as per our root sig)
			Graphics::CommandList->SetGraphicsRootDescriptorTable(2, material->GetFinalGPUHandleForSRVs());
			
			// Collect Entity data to send to the vertex shader
			{
				VertexShaderExternalData vsData = vsBaseData;
				vsData.world = entities[i]->GetTransform()->GetWorld();
				vsData.worldIT = entities[i]->GetTransform()->GetWorldInverseTranspose();

				// Put vertex shader data into the ring buffer and get a handle to its descriptor
				D3D12_GPU_DESCRIPTOR_HANDLE handle = Graphics::FillNextConstantBufferAndGetGPUDescriptorHandle(
					(void*)(&vsData), sizeof(VertexShaderExternalData));

				// Add a command to set the descriptor
				Graphics::CommandList->SetGraphicsRootDescriptorTable(0, handle);
			}

			// Pixel shader data and cbuffer setup
			{
				PixelShaderExternalData psData = psBaseData;
				psData.uvScale = material->GetUVScale();
				psData.uvOffset = material->GetUVOffset();
				psData.colorTint = material->GetColorTint();
				psData.roughness = material->GetRoughness();
				psData.metalness = material->GetMetalness();

				// Send this to a chunk of the constant buffer heap
				// and grab the GPU handle for it so we can set it for this draw
				D3D12_GPU_DESCRIPTOR_HANDLE handle = Graphics::FillNextConstantBufferAndGetGPUDescriptorHandle(
					(void*)(&psData), sizeof(PixelShaderExternalData));
				
				// Set this constant buffer handle
				// Note: This assumes that descriptor table 1 is the
				// place to put this particular descriptor. This
				// is based on how we set up our root signature.
				Graphics::CommandList->SetGraphicsRootDescriptorTable(1, handle);
			}

			// Set index and vertex buffers for this Entity
			D3D12_VERTEX_BUFFER_VIEW vbView = mesh->GetVertexBufferView();
			Graphics::CommandList->IASetVertexBuffers(0, 1, &vbView);

			D3D12_INDEX_BUFFER_VIEW ibView = mesh->GetIndexBufferView();
			Graphics::CommandList->IASetIndexBuffer(&ibView);

			Graphics::CommandList->DrawIndexedInstanced(mesh->GetIndexCount(), 1, 0, 0, 0);
		}
	}

	// Present
	{
		// Transition back to present
		D3D12_RESOURCE_BARRIER rb = {};
		rb.Type						= D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		rb.Flags					= D3D12_RESOURCE_BARRIER_FLAG_NONE;
		rb.Transition.pResource		= currentBackBuffer.Get();
		rb.Transition.StateBefore	= D3D12_RESOURCE_STATE_RENDER_TARGET;
		rb.Transition.StateAfter	= D3D12_RESOURCE_STATE_PRESENT;
		rb.Transition.Subresource	= D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		Graphics::CommandList->ResourceBarrier(1, &rb);

		// Must occur BEFORE present
		Graphics::CloseAndExecuteCommandList();

		// Present the current back buffer and move to the next one
		bool vsync = Graphics::VsyncState();
		Graphics::SwapChain->Present(
			vsync ? 1 : 0,
			vsync ? 0 : DXGI_PRESENT_ALLOW_TEARING);
		Graphics::AdvanceSwapChainIndex();

		// Wait for the GPU to be done and then reset the command list & allocator
		Graphics::WaitForGPU();
		Graphics::ResetAllocatorAndCommandList();
	}
}





// RESOURCE CREATION HELPER METHODS



// --------------------------------------------------------
// Creates the textures and materials needed for the scene
// --------------------------------------------------------
void Game::CreateMaterials()
{
	// Load textures

	// TEXTURES 0-13
	auto tBronzeAM		= LoadTexture(L"Assets/Textures/T_bronze_AM.png");
	auto tBronzeNR		= LoadTexture(L"Assets/Textures/T_bronze_NR.png");
	auto tCobblestoneAM = LoadTexture(L"Assets/Textures/T_cobblestone_AM.png");
	auto tCobblestoneNR = LoadTexture(L"Assets/Textures/T_cobblestone_NR.png");
	//auto tFloorAM		= LoadTexture(L"Assets/Textures/T_floor_AM.png");
	//auto tFloorNR		= LoadTexture(L"Assets/Textures/T_floor_NR.png");
	//auto tPaintAM		= LoadTexture(L"Assets/Textures/T_paint_AM.png");
	//auto tPaintNR		= LoadTexture(L"Assets/Textures/T_paint_NR.png");
	//auto tRoughAM		= LoadTexture(L"Assets/Textures/T_rough_AM.png");
	//auto tRoughNR		= LoadTexture(L"Assets/Textures/T_rough_NR.png");
	auto tScratchedAM	= LoadTexture(L"Assets/Textures/T_scratched_AM.png");
	auto tScratchedNR	= LoadTexture(L"Assets/Textures/T_scratched_NR.png");
	//auto tWoodAM		= LoadTexture(L"Assets/Textures/T_wood_AM.png");
	//auto tWoodNR		= LoadTexture(L"Assets/Textures/T_wood_NR.png");

	// Create Materials

	// MATERIALS 0-2
	auto matBronze		= AddMaterial("Mat_Bronze", pipelineState);
	matBronze			->AddTexture(tBronzeAM, 0);
	matBronze			->AddTexture(tBronzeNR, 1);
	matBronze			->FinalizeMaterial();

	auto matCobblestone	= AddMaterial("Mat_Cobblestone", pipelineState);
	matCobblestone		->AddTexture(tCobblestoneAM, 0);
	matCobblestone		->AddTexture(tCobblestoneNR, 1);
	matCobblestone		->FinalizeMaterial();

	auto matScratched	= AddMaterial("Mat_Scratched", pipelineState);
	matScratched		->AddTexture(tScratchedAM, 0);
	matScratched		->AddTexture(tScratchedNR, 1);
	matScratched		->FinalizeMaterial();
}

// --------------------------------------------------------
// Creates the textures and materials needed for the scene
// --------------------------------------------------------
D3D12_CPU_DESCRIPTOR_HANDLE Game::LoadTexture(const wchar_t* _path)
{
	D3D12_CPU_DESCRIPTOR_HANDLE newTexture = Graphics::LoadTexture(_path);
	textures.push_back(newTexture);
	return newTexture;
}

std::shared_ptr<Material> Game::AddMaterial(const char* _name, Microsoft::WRL::ComPtr<ID3D12PipelineState> _pso)
{
	return AddMaterial(_name, _pso, XMFLOAT3(1.0f, 1.0f, 1.0f));
}

std::shared_ptr<Material> Game::AddMaterial(const char* _name, Microsoft::WRL::ComPtr<ID3D12PipelineState> _pso, DirectX::XMFLOAT3 _colorTint)
{
	std::shared_ptr<Material> newMaterial = std::make_shared<Material>(
		_name,
		_pso,
		_colorTint,
		XMFLOAT2(1.0f, 1.0f),
		XMFLOAT2(0.0f, 0.0f)
	);
	materials.push_back(newMaterial);
	return newMaterial;
}

// --------------------------------------------------------
// Creates the meshes and entities we're going to draw
// --------------------------------------------------------
void Game::CreateGeometry()
{
	// Create meshes from OBJ models
	// MESHES 0-6
	meshes.push_back(make_shared<Mesh>("M_Cube",				FixPath(L"../../Assets/Models/cube.obj").c_str()));
	meshes.push_back(make_shared<Mesh>("M_Cylinder",			FixPath(L"../../Assets/Models/cylinder.obj").c_str()));
	meshes.push_back(make_shared<Mesh>("M_Helix",				FixPath(L"../../Assets/Models/helix.obj").c_str()));
	meshes.push_back(make_shared<Mesh>("M_Quad-SingleSided",	FixPath(L"../../Assets/Models/quad.obj").c_str()));
	meshes.push_back(make_shared<Mesh>("M_Quad-DoubleSided",	FixPath(L"../../Assets/Models/quad_double_sided.obj").c_str()));
	meshes.push_back(make_shared<Mesh>("M_Sphere",				FixPath(L"../../Assets/Models/sphere.obj").c_str()));
	meshes.push_back(make_shared<Mesh>("M_Torus",				FixPath(L"../../Assets/Models/torus.obj").c_str()));

	// ENTITIES 0-2
	AddEntity("E_Cube",		0,	0,	XMFLOAT3(-3.0f,	0.0f,	0.0f));
	AddEntity("E_Helix",	2,	1,	XMFLOAT3( 0.0f,	0.0f,	0.0f));
	AddEntity("E_Sphere",	5,	2,	XMFLOAT3( 3.0f,	0.0f,	0.0f));
}

// --------------------------------------------------------
// Adds an Entity to the list of Entities
// --------------------------------------------------------
std::shared_ptr<Entity> Game::AddEntity(const char* _name, unsigned int _meshIndex, unsigned int _materialIndex, DirectX::XMFLOAT3 _position)
{
	shared_ptr<Entity> entity = make_shared<Entity>(
		_name,
		meshes[_meshIndex],
		materials[_materialIndex]
	);

	entity->GetTransform()->SetPosition(_position);
	entities.push_back(entity);
	return entity;
}

// --------------------------------------------------------
// Creates all cameras the simulation can use
// --------------------------------------------------------
void Game::CreateCameras()
{
	// Create cameras
	float aspect = (Window::Width() + 0.0f) / Window::Height();
	// CAMERAS 0-3
	AddCamera("C_Main",		XMFLOAT3(0.0f, 0.0f, -5.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),					aspect, false);
	auto orthoYZ = AddCamera("C_OrthoYZ",	XMFLOAT3(100.0f, 0.0f, 0.0f),	XMFLOAT3(0.0f, -XM_PIDIV2, 0.0f),			aspect, true);
	orthoYZ->SetLookSpeed(1.0f);
	auto orthoXZ = AddCamera("C_OrthoXZ",	XMFLOAT3(0.0f, 100.0f, 0.0f),	XMFLOAT3(XM_PIDIV2 - 0.001f, 0.0f, 0.0f),	aspect, true);
	orthoXZ->SetLookSpeed(1.0f);
	auto orthoXY = AddCamera("C_OrthoXY",	XMFLOAT3(0.0f, 0.0f, -100.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),					aspect, true);
	orthoXY->SetLookSpeed(1.0f);
}

// --------------------------------------------------------
// Adds a Camera to the list of Cameras
// --------------------------------------------------------
shared_ptr<Camera> Game::AddCamera(const char* _name, DirectX::XMFLOAT3 _position, DirectX::XMFLOAT3 _rotation, float _aspect)
{
	shared_ptr<Camera> camera = make_shared<Camera>(
		_name,
		make_shared<Transform>(),
		_aspect
	);

	camera->GetTransform()->SetPosition(_position);
	camera->GetTransform()->SetRotation(_rotation);

	cameras.push_back(camera);
	return camera;
}

shared_ptr<Camera> Game::AddCamera(const char* _name, DirectX::XMFLOAT3 _position, DirectX::XMFLOAT3 _rotation, float _aspect, float _fov)
{
	shared_ptr<Camera> camera = make_shared<Camera>(
		_name,
		make_shared<Transform>(),
		_aspect,
		_fov
	);

	camera->GetTransform()->SetPosition(_position);
	camera->GetTransform()->SetRotation(_rotation);

	cameras.push_back(camera);
	return camera;
}

shared_ptr<Camera> Game::AddCamera(const char* _name, DirectX::XMFLOAT3 _position, DirectX::XMFLOAT3 _rotation, float _aspect, bool _isOrthographic)
{
	shared_ptr<Camera> camera = make_shared<Camera>(
		_name,
		make_shared<Transform>(),
		_aspect,
		_isOrthographic
	);

	camera->GetTransform()->SetPosition(_position);
	camera->GetTransform()->SetRotation(_rotation);

	cameras.push_back(camera);
	return camera;
}

shared_ptr<Camera> Game::AddCamera(const char* _name, DirectX::XMFLOAT3 _position, DirectX::XMFLOAT3 _rotation, float _aspect, bool _isOrthographic, float _orthoWidth)
{
	shared_ptr<Camera> camera = make_shared<Camera>(
		_name,
		make_shared<Transform>(),
		_aspect,
		_isOrthographic,
		_orthoWidth
	);

	camera->GetTransform()->SetPosition(_position);
	camera->GetTransform()->SetRotation(_rotation);

	cameras.push_back(camera);
	return camera;
}

void Game::CreateLights()
{
	// Create lights
	// LIGHTS 0-2
	AddLightDirectional(XMFLOAT3(-1.0f, -1.0f, 1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 1.0f, true);
	AddLightDirectional(XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 0.5f, true);
	AddLightDirectional(XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 1.0f, true);
	// LIGHTS 3-4
	AddLightPoint(XMFLOAT3(-2.0f, 3.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 1.0f), 1.0f, 10.0f, true);
	AddLightPoint(XMFLOAT3(2.0f, 0.0f, 3.0f), XMFLOAT3(1.0f, 0.0f, 1.0f), 1.0f, 5.0f, true);
}

// --------------------------------------------------------
// Adds a Light of a given type to the list of Lights
// --------------------------------------------------------
Light Game::AddLightDirectional(DirectX::XMFLOAT3 _direction, DirectX::XMFLOAT3 _color, float _intensity, bool _isActive)
{
	Light light = {};
	if (lights.size() >= MAX_LIGHTS) return light; // If all light slots have been used, don't add light
	light.Type = LIGHT_TYPE_DIRECTIONAL;
	light.Direction = _direction;
	light.Color = _color;
	light.Intensity = _intensity;
	light.Active = _isActive ? 1 : 0;
	lights.push_back(light);
	return light;
}

Light Game::AddLightPoint(DirectX::XMFLOAT3 _position, DirectX::XMFLOAT3 _color, float _intensity, float _range, bool _isActive)
{
	Light light = {};
	if (lights.size() >= MAX_LIGHTS) return light;
	light.Type = LIGHT_TYPE_POINT;
	light.Position = _position;
	light.Color = _color;
	light.Intensity = _intensity;
	light.Range = _range;
	light.Active = _isActive ? 1 : 0;
	lights.push_back(light);
	return light;
}

Light Game::AddLightSpot(DirectX::XMFLOAT3 _position, DirectX::XMFLOAT3 _direction, DirectX::XMFLOAT3 _color, float _intensity, float _range, float _innerAngle, float _outerAngle, bool _isActive)
{
	Light light = {};
	if (lights.size() >= MAX_LIGHTS) return light;
	light.Type = LIGHT_TYPE_SPOT;
	light.Position = _position;
	light.Direction = _direction;
	light.Color = _color;
	light.Intensity = _intensity;
	light.Range = _range;
	light.SpotInnerAngle = _innerAngle;
	light.SpotOuterAngle = _outerAngle;
	light.Active = _isActive ? 1 : 0;
	lights.push_back(light);
	return light;
}

void Game::ImGuiInitialize()
{
	// Initialize ImGui itself & platform/renderer backends
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplWin32_Init(Window::Handle());
	
	ImGui_ImplDX12_InitInfo init_info = {};
	init_info.Device = Graphics::Device.Get();
	init_info.CommandQueue = Graphics::CommandQueue.Get();
	init_info.NumFramesInFlight = Graphics::NumBackBuffers;
	init_info.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	init_info.DSVFormat = DXGI_FORMAT_UNKNOWN;

	// Allocating SRV descriptors (for textures) is up to the application, so we provide callbacks.
	// (current version of the backend will only allocate one descriptor, future versions will need to allocate more)
	/*init_info.SrvDescriptorHeap = Graphics::CBVSRVDescriptorHeap.Get();
	init_info.SrvDescriptorAllocFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE* out_cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE* out_gpu_handle) { return .Alloc(out_cpu_handle, out_gpu_handle); };
	init_info.SrvDescriptorFreeFn = [](ImGui_ImplDX12_InitInfo*, D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle, D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle)			{ return g_pd3dSrvDescHeapAlloc.Free(cpu_handle, gpu_handle); };
	ImGui_ImplDX12_Init(&init_info);*/

	// Pick a style (uncomment one of these 3)
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();
	//ImGui::StyleColorsClassic();
}

void Game::ImGuiUpdate()
{
}

void Game::ImGuiBuildInterface()
{
}





// OTHER CUSTOM HELPER METHODS


// --------------------------------------------------------
// Initializes all simulation parameters
// --------------------------------------------------------
void Game::InitializeParameters()
{
	cameraCurrent = 0;
}

