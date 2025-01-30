#include "Game.h"

#include <DirectXMath.h>

#include "Graphics.h"
#include "Vertex.h"
#include "Input.h"
#include "PathHelpers.h"
#include "Window.h"
#include "BufferStructs.h"


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
	InitializeParameters();
	CreateRootSigAndPipelineState();
	CreateGeometry();
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
			FixPath(L"VertexShader.cso").c_str(),
			vertexShaderByteCode.GetAddressOf());
		D3DReadFileToBlob(
			FixPath(L"PixelShader.cso").c_str(), pixelShaderByteCode.GetAddressOf());
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
		// Define a table of CBV's (constant buffer views)
		D3D12_DESCRIPTOR_RANGE cbvTable = {};
		cbvTable.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		cbvTable.NumDescriptors = 1;
		cbvTable.BaseShaderRegister = 0;
		cbvTable.RegisterSpace = 0;
		cbvTable.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		// Define the root parameter
		D3D12_ROOT_PARAMETER rootParam = {};
		rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParam.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
		rootParam.DescriptorTable.NumDescriptorRanges = 1;
		rootParam.DescriptorTable.pDescriptorRanges = &cbvTable;

		// Describe the overall the root signature
		D3D12_ROOT_SIGNATURE_DESC rootSig = {};
		rootSig.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
		rootSig.NumParameters = 1;
		rootSig.pParameters = &rootParam;
		rootSig.NumStaticSamplers = 0;
		rootSig.pStaticSamplers = 0;

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
}


// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	// Example input checking: Quit if the escape key is pressed
	if (Input::KeyDown(VK_ESCAPE))
		Window::Quit();
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

		// Collect base data that will be used for all Entities
		VertexShaderExternalData baseData = {};
		baseData.view		= camera->GetViewMatrix();
		baseData.projection	= camera->GetProjectionMatrix();

		// Loop through and render all Entities
		for (unsigned int i = 0; i < entities.size(); i++) {

			// Grab the Entity's Mesh
			std::shared_ptr<Mesh> mesh = entities[i]->GetMesh();

			// Collect Entity data to send to the vertex shader
			VertexShaderExternalData vsData = baseData;
			vsData.world = entities[i]->GetTransform()->GetWorld();

			// Put vertex shader data into the ring buffer and get a handle to its descriptor
			D3D12_GPU_DESCRIPTOR_HANDLE handle = Graphics::FillNextConstantBufferAndGetGPUDescriptorHandle(
				&vsData, sizeof(VertexShaderExternalData));

			// Add a command to set the descriptor
			Graphics::CommandList->SetGraphicsRootDescriptorTable(0, handle);

			// Set index and vertex buffers for this Entity
			D3D12_VERTEX_BUFFER_VIEW vbView = mesh->GetVertexBufferView();
			Graphics::CommandList->IASetVertexBuffers(0, 1, &vbView);

			D3D12_INDEX_BUFFER_VIEW ibView = mesh->GetIndexBufferView();
			Graphics::CommandList->IASetIndexBuffer(&ibView);

			Graphics::CommandList->DrawIndexedInstanced(mesh->GetIndexCount(), 1, 0, 0, 0);
		}

		Graphics::CommandList->SetDescriptorHeaps(1,
			Graphics::CBVSRVDescriptorHeap.GetAddressOf());
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

	AddEntity("E_Cube",		0,	XMFLOAT3(-3.0f,	0.0f,	0.0f));
	AddEntity("E_Helix",	2,	XMFLOAT3( 0.0f,	0.0f,	0.0f));
	AddEntity("E_Sphere",	5,	XMFLOAT3( 3.0f,	0.0f,	0.0f));
}

// --------------------------------------------------------
// Adds an Entity to the list of Entities
// --------------------------------------------------------
void Game::AddEntity(const char* _name, unsigned int _meshIndex, DirectX::XMFLOAT3 _position)
{
	shared_ptr<Entity> entity = make_shared<Entity>(
		_name,
		meshes[_meshIndex]
	);

	entity->GetTransform()->SetPosition(_position);
	entities.push_back(entity);
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
	AddCamera("C_OrthoYZ",	XMFLOAT3(100.0f, 0.0f, 0.0f),	XMFLOAT3(0.0f, -XM_PIDIV2, 0.0f),			aspect, true);
	cameras[1]->SetLookSpeed(1.0f);
	AddCamera("C_OrthoXZ",	XMFLOAT3(0.0f, 100.0f, 0.0f),	XMFLOAT3(XM_PIDIV2 - 0.001f, 0.0f, 0.0f),	aspect, true);
	cameras[2]->SetLookSpeed(1.0f);
	AddCamera("C_OrthoXY",	XMFLOAT3(0.0f, 0.0f, -100.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),					aspect, true);
	cameras[3]->SetLookSpeed(1.0f);
}

// --------------------------------------------------------
// Adds a Camera to the list of Cameras
// --------------------------------------------------------
void Game::AddCamera(const char* _name, DirectX::XMFLOAT3 _position, DirectX::XMFLOAT3 _rotation, float _aspect)
{
	shared_ptr<Camera> camera = make_shared<Camera>(
		_name,
		make_shared<Transform>(),
		_aspect
	);

	camera->GetTransform()->SetPosition(_position);
	camera->GetTransform()->SetRotation(_rotation);

	cameras.push_back(camera);
}

void Game::AddCamera(const char* _name, DirectX::XMFLOAT3 _position, DirectX::XMFLOAT3 _rotation, float _aspect, float _fov)
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
}

void Game::AddCamera(const char* _name, DirectX::XMFLOAT3 _position, DirectX::XMFLOAT3 _rotation, float _aspect, bool _isOrthographic)
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
}

void Game::AddCamera(const char* _name, DirectX::XMFLOAT3 _position, DirectX::XMFLOAT3 _rotation, float _aspect, bool _isOrthographic, float _orthoWidth)
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
}





// OTHER CUSTOM HELPER METHODS


// --------------------------------------------------------
// Initializes all simulation parameters
// --------------------------------------------------------
void Game::InitializeParameters()
{
	cameraCurrent = 0;
}

