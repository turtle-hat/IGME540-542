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
#include "RayTracing.h"

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
	ImGuiInitialize();
	// Initialize raytracing
	RayTracing::Initialize(
		Window::Width(),
		Window::Height(),
		FixPath(L"RayTracing.cso"));
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
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

// --------------------------------------------------------
// Loads the two basic shaders, then creates the root signature
// and pipeline state object for our very basic demo.
// --------------------------------------------------------
void Game::CreateRootSigAndPipelineState()
{
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
			cameras[pCameraCurrent]->SetAspect((Window::Width() + 0.0f) / Window::Height());
		}
	}

	// Resize raytracing output texture
	RayTracing::ResizeOutputUAV(Window::Width(), Window::Height());
}


// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	// Setup new frame for ImGui
	ImGuiUpdate(deltaTime);
	ImGuiBuildInterface();

	// Example input checking: Quit if the escape key is pressed
	if (Input::KeyDown(VK_ESCAPE))
		Window::Quit();

	// Update current camera
	cameras[pCameraCurrent]->Update(deltaTime);

	// Rotate meshes
	for (unsigned int i = 1; i < entities.size(); i++) {
		auto transform = entities[i]->GetTransform();
		transform->Rotate(0.0f, pObjectRotationSpeed * deltaTime, 0.0f);

		if (i > 3) {
			XMFLOAT3 pos = transform->GetPosition();
			transform->SetPosition(pos.x, sinf(totalTime * 2.5f + (float)i) - 5.0f, pos.z);
			transform->SetScale(
				sinf(totalTime + (float)i) * 0.5f + 1.0f,
				sinf(totalTime + (float)i) * 0.5f + 1.0f,
				cosf(totalTime + (float)i) * 0.5f + 1.0f
			);
		}
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

	// Create a resource barrier that can be changed quickly for different transitions
	D3D12_RESOURCE_BARRIER rb = {};
	rb.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	rb.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	rb.Transition.pResource = currentBackBuffer.Get();
	rb.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	// Rendering here!
	{
		RayTracing::CreateTopLevelAccelerationStructureForScene(entities);

		// Perform ray trace (which also copies the results to the back buffer)
		RayTracing::Raytrace(cameras[pCameraCurrent], currentBackBuffer);
	}

	// Display ImGui
	{
		// Set render target and viewport for rasterization
		Graphics::CommandList->OMSetRenderTargets(1, &Graphics::RTVHandles[Graphics::SwapChainIndex()], true, &Graphics::DSVHandle);
		Graphics::CommandList->RSSetViewports(1, &viewport);

		// Transition back buffer back to render target
		rb.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		rb.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		Graphics::CommandList->ResourceBarrier(1, &rb);

		// Rendering
		ImGui::Render();
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), Graphics::CommandList.Get());
	}


	// Present
	{
		rb.Transition.StateBefore	= D3D12_RESOURCE_STATE_RENDER_TARGET;
		rb.Transition.StateAfter	= D3D12_RESOURCE_STATE_PRESENT;
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
		Graphics::ResetAllocatorAndCommandList(Graphics::SwapChainIndex());
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
	auto tFloorAM		= LoadTexture(L"Assets/Textures/T_floor_AM.png");
	auto tFloorNR		= LoadTexture(L"Assets/Textures/T_floor_NR.png");
	//auto tPaintAM		= LoadTexture(L"Assets/Textures/T_paint_AM.png");
	//auto tPaintNR		= LoadTexture(L"Assets/Textures/T_paint_NR.png");
	//auto tRoughAM		= LoadTexture(L"Assets/Textures/T_rough_AM.png");
	//auto tRoughNR		= LoadTexture(L"Assets/Textures/T_rough_NR.png");
	auto tScratchedAM	= LoadTexture(L"Assets/Textures/T_scratched_AM.png");
	auto tScratchedNR	= LoadTexture(L"Assets/Textures/T_scratched_NR.png");
	//auto tWoodAM		= LoadTexture(L"Assets/Textures/T_wood_AM.png");
	//auto tWoodNR		= LoadTexture(L"Assets/Textures/T_wood_NR.png");

	// Create Materials

	// MATERIALS 0-3
	auto matBronze		= AddMaterial("Mat_Bronze", pipelineState);
	matBronze			->AddTexture(tBronzeAM, 0);
	matBronze			->AddTexture(tBronzeNR, 1);
	matBronze			->SetColorTint(XMFLOAT3(1.0f, 0.5f, 0.0f));
	matBronze			->FinalizeMaterial();

	auto matCobblestone	= AddMaterial("Mat_Cobblestone", pipelineState);
	matCobblestone		->AddTexture(tCobblestoneAM, 0);
	matCobblestone		->AddTexture(tCobblestoneNR, 1);
	matCobblestone		->SetColorTint(XMFLOAT3(0.1f, 0.1f, 0.1f));
	matCobblestone		->FinalizeMaterial();

	auto matFloor		= AddMaterial("Mat_Floor", pipelineState);
	matFloor			->AddTexture(tFloorAM, 0);
	matFloor			->AddTexture(tFloorNR, 1);
	matFloor			->SetColorTint(XMFLOAT3(0.2f, 0.3f, 0.25f));
	matFloor			->FinalizeMaterial();

	auto matScratched	= AddMaterial("Mat_Scratched", pipelineState);
	matScratched		->AddTexture(tScratchedAM, 0);
	matScratched		->AddTexture(tScratchedNR, 1);
	matScratched		->SetColorTint(XMFLOAT3(1.0f, 1.0f, 0.1f));
	matScratched		->FinalizeMaterial();
}

// --------------------------------------------------------
// Creates the textures and materials needed for the scene
// --------------------------------------------------------
D3D12_CPU_DESCRIPTOR_HANDLE Game::LoadTexture(const wchar_t* _path)
{
	D3D12_CPU_DESCRIPTOR_HANDLE newTexture = Graphics::LoadTexture(_path, false); // Don't autogenerate mips, since they're ignored for now
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

	// ENTITIES 0-3
	auto floor = AddEntity("E_Floor",	0,	2,	XMFLOAT3(0.0f, -110.0f, 0.0f));
	floor->GetTransform()->SetScale(100.0f, 100.0f, 100.0f);

	AddEntity("E_Cube",		0,	0,	XMFLOAT3(-3.0f,	0.0f,	0.0f));
	AddEntity("E_Helix",	2,	1,	XMFLOAT3( 0.0f,	0.0f,	0.0f));
	AddEntity("E_Sphere",	5,	3,	XMFLOAT3( 3.0f,	0.0f,	0.0f));

	// Get how many materials have been created so we can index past them
	int firstGeneratedMaterialIndex = (int)materials.size();

	// MATERIALS & ENTITIES 4-28
	for (int i = 0; i < 25; i++) {
		auto newMat = AddMaterial("Mat_Generated", pipelineState);
		newMat->SetColorTint(XMFLOAT3(
			(float)rand() / RAND_MAX,
			(float)rand() / RAND_MAX,
			(float)rand() / RAND_MAX
		));

		AddEntity("E_Generated", 0, firstGeneratedMaterialIndex + i, XMFLOAT3(
			(float)((i % 5) - 2) * 3,
			-5.0f,
			(float)((i / 5) - 2) * 3
		));
	}

	// Create the TLAS for our “scene”
	RayTracing::CreateTopLevelAccelerationStructureForScene(entities);

	// Finalize any initialization and wait for the GPU
	// before proceeding to the game loop
	Graphics::CloseAndExecuteCommandList();
	Graphics::WaitForGPU();
	Graphics::ResetAllocatorAndCommandList(Graphics::SwapChainIndex());
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
	auto orthoYZ = AddCamera("C_PlaneZY",	XMFLOAT3(20.0f, 0.0f, 0.0f),	XMFLOAT3(0.0f, -XM_PIDIV2, 0.0f),			aspect, false);
	orthoYZ->SetLookSpeed(1.0f);
	auto orthoXZ = AddCamera("C_PlaneXZ",	XMFLOAT3(0.0f, 20.0f, 0.0f),	XMFLOAT3(XM_PIDIV2 - 0.001f, 0.0f, 0.0f),	aspect, false);
	orthoXZ->SetLookSpeed(1.0f);
	auto orthoXY = AddCamera("C_PlaneXY",	XMFLOAT3(0.0f, 0.0f, -20.0f),	XMFLOAT3(0.0f, 0.0f, 0.0f),					aspect, false);
	orthoXY->SetLookSpeed(1.0f);
	
	pCameraCurrent = 0;
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
	// Reserve descriptor slot for ImGui's font texture
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
	Graphics::ReserveDescriptorHeapSlot(&cpuHandle, &gpuHandle);

	// Initialize ImGui itself & platform/renderer backends
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	// Pick a style (uncomment one of these 3)
	//ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();
	ImGui::StyleColorsClassic();
	ImGui_ImplWin32_Init(Window::Handle());
	
	ImGui_ImplDX12_InitInfo init_info = {};
	init_info.CommandQueue = Graphics::CommandQueue.Get();
	init_info.Device = Graphics::Device.Get();
	init_info.DSVFormat = DXGI_FORMAT_UNKNOWN;
	init_info.LegacySingleSrvCpuDescriptor = cpuHandle;
	init_info.LegacySingleSrvGpuDescriptor = gpuHandle;
	init_info.NumFramesInFlight = Graphics::NumBackBuffers;
	init_info.RTVFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	// Allocating SRV descriptors (for textures) is up to the application, so we provide callbacks.
	// (current version of the backend will only allocate one descriptor, future versions will need to allocate more)
	init_info.SrvDescriptorHeap = Graphics::CBVSRVDescriptorHeap.Get();

	ImGui_ImplDX12_Init(&init_info);
}

void Game::ImGuiUpdate(float _deltaTime)
{
	// Put this all in a helper method that is called from Game::Update()
	// Feed fresh data to ImGui
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = _deltaTime;
	io.DisplaySize.x = (float)Window::Width();
	io.DisplaySize.y = (float)Window::Height();

	// Reset the frame
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	// Determine new input capture
	Input::SetKeyboardCapture(io.WantCaptureKeyboard);
	Input::SetMouseCapture(io.WantCaptureMouse);
}

void Game::ImGuiBuildInterface()
{
	// Show the demo window if it's activated
	if (igShowDemo) {
		ImGui::ShowDemoWindow();
	}

	ImGui::Begin("Inspector");

	if (ImGui::CollapsingHeader("App Details")) {				// Statistics about the app window and performance; no input elements
		if (ImGui::TreeNode("Window")) {							// Meta stats about the window, mouse, and other stuff outside the simulation
			ImGui::Spacing();
			ImVec2 mousePos = ImGui::GetIO().MousePos;

			ImGui::Text("Resolution:   %6dx %6d", Window::Width(), Window::Height());
			ImGui::SetItemTooltip("Window resolution in pixels");

			ImGui::Text("Mouse (px):  (%6d, %6d)", (int)mousePos.x, (int)mousePos.y);
			ImGui::SetItemTooltip("Mouse position in pixels,\nstarting at top-left corner");

			ImGui::Text("Mouse (NDC): (%+6.3f, %+6.3f)",
				2.0f * (mousePos.x - (Window::Width() * 0.5f)) / Window::Width(),
				-2.0f * (mousePos.y - (Window::Height() * 0.5f)) / Window::Height()
			);
			ImGui::SetItemTooltip("Mouse position in Normalized Device Coordinates\n(-1 to 1), starting at top-left corner");

			ImGui::Text("Aspect Ratio: %6.3f", ((Window::Width() + 0.0f) / Window::Height()));
			ImGui::SetItemTooltip("Window aspect ratio (width/height)");

			ImGui::TreePop();
			ImGui::Spacing();
		}
		if (ImGui::TreeNode("Performance")) {						// Stats about the app's performance
			ImGui::Spacing();

			ImGui::Text("Framerate:    %6dfps", (int)ImGui::GetIO().Framerate);

			ImGui::Text("Delta Time:   %6dus", (int)(ImGui::GetIO().DeltaTime * 1000000));
			ImGui::SetItemTooltip("Time between frames in microseconds\n(I didn't want to break things by trying to print the mu)");

			ImGui::TreePop();
			ImGui::Spacing();
		}
	}

	if (ImGui::CollapsingHeader("Settings")) {					// General settings parameters for graphics and simulation
		ImGui::Spacing();

		ImGui::ColorEdit3("Background Color", pBackgroundColor);
		ImGui::Spacing();

		ImGui::SliderFloat("Object Rotation", &pObjectRotationSpeed, -2.0f, 2.0f, "%.1f");
		ImGui::Spacing();
	}

	if (ImGui::CollapsingHeader("Meshes")) {					// Info about each mesh
		ImGui::Spacing();

		ImGui::PushID("MESH");
		for (int i = 0; i < meshes.size(); i++) {

			// Each mesh gets its own Tree Node
			ImGui::PushID(i);
			if (ImGui::TreeNode("", "(%06d) %s", i, meshes[i]->GetName())) {
				ImGui::Spacing();

				ImGui::Text("Triangles: %6d", meshes[i]->GetIndexCount() / 3);
				ImGui::Text("Vertices:  %6d", meshes[i]->GetVertexCount());
				ImGui::Text("Indices:   %6d", meshes[i]->GetIndexCount());

				ImGui::TreePop();
				ImGui::Spacing();
			}
			ImGui::PopID();
		}
		ImGui::PopID();

		ImGui::Spacing();
	}

	if (ImGui::CollapsingHeader("Materials")) {					// Info about each material
		ImGui::Spacing();

		ImGui::PushID("MATERIAL");
		for (int i = 0; i < materials.size(); i++) {

			// Each material gets its own Tree Node
			ImGui::PushID(i);
			if (ImGui::TreeNode("", "(%06d) %s", i, materials[i]->GetName())) {
				ImGui::Spacing();

				// Get material's tint as a float array
				XMFLOAT3 tint_xm = materials[i]->GetColorTint();
				float tint_f[3] = { tint_xm.x, tint_xm.y, tint_xm.z };
				float roughness = materials[i]->GetRoughness();
				float metalness = materials[i]->GetMetalness();
				XMFLOAT2 uv_pos = materials[i]->GetUVOffset();
				XMFLOAT2 uv_sca = materials[i]->GetUVScale();

				// If the user has edited the tint this frame, change the material's tint
				if (ImGui::ColorEdit3("Tint", tint_f)) {
					materials[i]->SetColorTint(XMFLOAT3(tint_f));
				}
				// If the user has edited the material's roughness this frame, change the material's roughness
				if (ImGui::SliderFloat("Roughness", &roughness, 0.0f, 1.0f, "%.2f")) {
					materials[i]->SetRoughness(roughness);
				}
				// If the user has edited the material's metalness this frame, change the material's metalness
				if (ImGui::SliderFloat("Metalness", &metalness, 0.0f, 1.0f, "%.2f")) {
					materials[i]->SetMetalness(metalness);
				}
				if (ImGui::DragFloat2("UV Offset", (float*)&uv_pos, 0.01f, NULL, NULL, "%.2f")) {
					materials[i]->SetUVOffset(uv_pos);
				}
				if (ImGui::DragFloat2("UV Scale", (float*)&uv_sca, 0.01f, NULL, NULL, "%.2f")) {
					materials[i]->SetUVScale(uv_sca);
				}

				int numTextures = materials[i]->GetNumTextures();
				// If any textures exist, include texture images
				if (numTextures > 0) {
					D3D12_GPU_DESCRIPTOR_HANDLE textures = materials[i]->GetFinalGPUHandleForSRVs();

					int non2DTextures = 0;
					D3D12_GPU_DESCRIPTOR_HANDLE textureCurrent = textures;

					ImGui::Text("Textures:");
					for (int j = 0; j < numTextures; j++) {
						// Increment texture ptr for textures after the first
						if (j > 0) {
							textureCurrent.ptr += Graphics::GetCBVSRVDescriptorHeapIncrementSize();
						}
						
						ImGui::Text("(%06d) GPU Handle: %p", j, textureCurrent);
						// Get a description of the texture
						D3D12_SHADER_RESOURCE_VIEW_DESC srvDescription = {};

						// Only display the texture if it's a Texture2D
						if (typeid(srvDescription.Texture2D) == typeid(D3D12_TEX2D_SRV)) { // Figure out which union member it's using
							ImGui::Image(
								(ImTextureID)textureCurrent.ptr,
								ImVec2(256, 256),
								ImVec2(uv_pos.x, uv_pos.y),
								ImVec2(uv_pos.x + uv_sca.x, uv_pos.y + uv_sca.y)
							);
						}
						else {
							// Count the number of non-Texture2D textures
							non2DTextures++;
						}

					}

					// Print number of SRVs not displayed
					if (non2DTextures > 0) {
						ImGui::Text("(%d non-Texture2D SRV(s) not displayed)", non2DTextures);
					}
				}

				ImGui::TreePop();
				ImGui::Spacing();
			}
			ImGui::PopID();
		}
		ImGui::PopID();

		ImGui::Spacing();
	}

	if (ImGui::CollapsingHeader("Entities")) {					// Info about each entity
		ImGui::Spacing();

		// Store the position, rotation, scale, and tint of each entity as they're read in


		ImGui::PushID("ENTITY");
		for (int i = 0; i < entities.size(); i++) {
			// Get position, rotation, scale, and tint
			XMFLOAT3 entityPos = entities[i]->GetTransform()->GetPosition();
			XMFLOAT3 entityRot = entities[i]->GetTransform()->GetRotation();
			XMFLOAT3 entitySca = entities[i]->GetTransform()->GetScale();

			// Each entity gets its own Tree Node
			ImGui::PushID(i);
			if (ImGui::TreeNode("", "(%06d) %s", i, entities[i]->GetName())) {
				ImGui::Spacing();

				ImGui::Text("Mesh:      %s", (entities[i]->GetMesh()->GetName()));
				ImGui::Text("Material:  %s", (entities[i]->GetMaterial()->GetName()));
				ImGui::Spacing();

				if (ImGui::DragFloat3("Position", &entityPos.x, 0.01f)) {
					entities[i]->GetTransform()->SetPosition(entityPos);
				}
				if (ImGui::DragFloat3("Rotation", &entityRot.x, 0.01f)) {
					entities[i]->GetTransform()->SetRotation(entityRot);
				}
				ImGui::SetItemTooltip("In radians");
				if (ImGui::DragFloat3("Scale", &entitySca.x, 0.01f, 0.0f)) {
					entities[i]->GetTransform()->SetScale(entitySca);
				}
				// Clamp scale to 0
				if (entitySca.x < 0.0f) entitySca.x = 0.0f;
				if (entitySca.y < 0.0f) entitySca.y = 0.0f;
				if (entitySca.z < 0.0f) entitySca.z = 0.0f;

				ImGui::TreePop();
				ImGui::Spacing();
			}
			ImGui::PopID();
		}
		ImGui::PopID();

		ImGui::Spacing();
	}

	if (ImGui::CollapsingHeader("Lights")) {					// Info about each light
		ImGui::Spacing();

		ImGui::PushID("LIGHT");
		for (int i = 0; i < lights.size(); i++) {					// List of lights in the scene

			// Each light gets its own Tree Node
			ImGui::PushID(i);

			bool active = lights[i].Active == 1;
			ImGui::AlignTextToFramePadding();
			if (ImGui::Checkbox("", &active)) {
				lights[i].Active = active;
			}
			ImGui::SetItemTooltip("Toggle whether light is active");

			ImGui::SameLine();
			if (ImGui::TreeNode("node", "(%06d) %s", i, LIGHT_TYPE_STRINGS[lights[i].Type])) {
				ImGui::Spacing();

				ImGui::ColorEdit3("Color", &lights[i].Color.x);
				if (ImGui::DragFloat("Intensity", &lights[i].Intensity, 0.1f, 0.0f, NULL, "%.1f")) {
					lights[i].Intensity = max(lights[i].Intensity, 0.0f);
				}

				ImGui::Spacing();
				ImGui::Text("Type:");
				ImGui::RadioButton("Directional", &lights[i].Type, LIGHT_TYPE_DIRECTIONAL);
				ImGui::SameLine();
				ImGui::RadioButton("Point", &lights[i].Type, LIGHT_TYPE_POINT);
				ImGui::SameLine();
				ImGui::RadioButton("Spot", &lights[i].Type, LIGHT_TYPE_SPOT);

				ImGui::Spacing();
				if (lights[i].Type != LIGHT_TYPE_DIRECTIONAL) {
					ImGui::DragFloat3("Position", &lights[i].Position.x, 0.01f);
				}
				if (lights[i].Type != LIGHT_TYPE_POINT) {
					ImGui::DragFloat3("Direction", &lights[i].Direction.x, 0.01f);
				}
				if (lights[i].Type != LIGHT_TYPE_DIRECTIONAL) {
					if (ImGui::DragFloat("Range", &lights[i].Range, 0.1f, 0.0f, NULL, "%.1f")) {
						lights[i].Range = max(lights[i].Range, 0.0f);
					}
				}
				if (lights[i].Type == LIGHT_TYPE_SPOT) {
					if (ImGui::DragFloat("Spot Inner Angle", &lights[i].SpotInnerAngle, 0.01f, 0.0f, XM_PIDIV2, "%.2f")) {
						if (lights[i].SpotOuterAngle <= lights[i].SpotInnerAngle) {
							lights[i].SpotOuterAngle = lights[i].SpotInnerAngle + 0.01f;
						}
					}
					ImGui::SetItemTooltip("In radians");
					if (ImGui::DragFloat("Spot Outer Angle", &lights[i].SpotOuterAngle, 0.01f, 0.01f, XM_PIDIV2, "%.2f")) {
						if (lights[i].SpotOuterAngle <= lights[i].SpotInnerAngle) {
							lights[i].SpotInnerAngle = lights[i].SpotOuterAngle - 0.01f;
						}
					}
					ImGui::SetItemTooltip("In radians");
				}

				ImGui::TreePop();
				ImGui::Spacing();
			}
			ImGui::PopID();
		}
		ImGui::PopID();
	}

	if (ImGui::CollapsingHeader("Cameras")) {					// Info about each camera
		ImGui::Spacing();

		ImGui::PushID("CAMERA");
		for (int i = 0; i < cameras.size(); i++) {
			// Get position, rotation, scale, and tint
			XMFLOAT3 cameraPos = cameras[i]->GetTransform()->GetPosition();
			XMFLOAT3 cameraRot = cameras[i]->GetTransform()->GetRotation();
			XMFLOAT3 cameraRight = cameras[i]->GetTransform()->GetRight();
			XMFLOAT3 cameraUp = cameras[i]->GetTransform()->GetUp();
			XMFLOAT3 cameraFwd = cameras[i]->GetTransform()->GetForward();
			bool cameraMode = cameras[i]->GetProjectionMode();
			float cameraMove = cameras[i]->GetMoveSpeed();
			float cameraLook = cameras[i]->GetLookSpeed();
			float cameraNear = cameras[i]->GetNearClip();
			float cameraFar = cameras[i]->GetFarClip();

			// Each camera gets its own Tree Node
			ImGui::PushID(i);
			ImGui::AlignTextToFramePadding();
			ImGui::RadioButton("", &pCameraCurrent, i);
			ImGui::SetItemTooltip("Set as active camera");

			ImGui::SameLine();
			if (ImGui::TreeNode("node", "(%06d) %s", i, cameras[i]->GetName())) {
				ImGui::Spacing();

				if (ImGui::Button(cameraMode ? "Mode: Orthographic" : "Mode: Perspective")) {
					cameras[i]->ToggleProjectionMode();
				}
				if (cameraMode) {
					float cameraWidth = cameras[i]->GetOrthographicWidth();
					if (ImGui::DragFloat("Width", &cameraWidth, 1.0f, 1.0f, 1000.0f, "%.0f")) {
						cameras[i]->SetOrthographicWidth(cameraWidth);
					}
					ImGui::SetItemTooltip("In world units");
				}
				else {
					float cameraFov = (cameras[i]->GetFov() * 180 * XM_1DIVPI);
					if (ImGui::DragFloat("Field of View", &cameraFov, 1.0f, 1.0f, 179.0f, "%.0f")) {
						cameras[i]->SetFov(cameraFov * XM_PI / 180);
					}
					ImGui::SetItemTooltip("In degrees (stored as radians)");
				}
				ImGui::Spacing();

				if (ImGui::DragFloat3("Position", &cameraPos.x, 0.01f)) {
					cameras[i]->GetTransform()->SetPosition(cameraPos);
				}
				if (ImGui::DragFloat3("Rotation", &cameraRot.x, 0.01f)) {
					cameras[i]->GetTransform()->SetRotation(cameraRot);
				}
				ImGui::SetItemTooltip("In radians");
				ImGui::Text("Right:       (%+6.3f, %+6.3f, %+6.3f)", cameraRight.x, cameraRight.y, cameraRight.z);
				ImGui::Text("Up:          (%+6.3f, %+6.3f, %+6.3f)", cameraUp.x, cameraUp.y, cameraUp.z);
				ImGui::Text("Forward:     (%+6.3f, %+6.3f, %+6.3f)", cameraFwd.x, cameraFwd.y, cameraFwd.z);
				ImGui::Spacing();

				if (ImGui::DragFloat("Move Speed", &cameraMove, 0.1f, 0.1f, 100.0f, "%.1f", ImGuiSliderFlags_Logarithmic)) {
					cameras[i]->SetMoveSpeed(cameraMove);
				}
				ImGui::SetItemTooltip("In units per second");
				if (ImGui::DragFloat("Look Speed", &cameraLook, 0.01f, 0.01f, 10.0f, "%.2f", ImGuiSliderFlags_Logarithmic)) {
					cameras[i]->SetLookSpeed(cameraLook);
				}
				ImGui::SetItemTooltip("In milliradians per pixel\nof mouse movement");
				ImGui::Spacing();

				if (ImGui::DragFloat("Near Clip", &cameraNear, 0.01f, 0.001f, 10.0f, "%.3f", ImGuiSliderFlags_Logarithmic)) {
					if (cameraFar > cameraNear) {
						cameras[i]->SetNearClip(cameraNear);
					}
					else {
						cameras[i]->SetNearClip(cameraFar - 0.001f);
					}
				}
				if (ImGui::DragFloat("Far Clip", &cameraFar, 1.0f, 11.0f, 10000.0f, "%.0f", ImGuiSliderFlags_Logarithmic)) {
					if (cameraFar > cameraNear) {
						cameras[i]->SetFarClip(cameraFar);
					}
					else {
						cameras[i]->SetFarClip(floorf(cameraNear) + 1.0f);
					}
				}

				ImGui::TreePop();
				ImGui::Spacing();
			}
			ImGui::PopID();

		}
		ImGui::PopID();

		ImGui::Spacing();
	}

	if (ImGui::CollapsingHeader("Dear ImGui")) {				// Settings related to ImGui itself
		ImGui::Spacing();

		if (ImGui::Button("Toggle Dear ImGui Demo")) {				// Toggles the ImGui Demo window
			igShowDemo = !igShowDemo;
		}

		ImGui::Spacing();
	}

	ImGui::End();
}





// OTHER CUSTOM HELPER METHODS


// --------------------------------------------------------
// Initializes all simulation parameters
// --------------------------------------------------------
void Game::InitializeParameters()
{
	pCameraCurrent = 0;
	igShowDemo = false;
	// Background color (Cornflower Blue in this case) for clearing
	float bgColor[4] = {0.4f, 0.6f, 0.75f, 1.0f};
	memcpy(pBackgroundColor, bgColor, sizeof(float) * 4);
	pObjectRotationSpeed = 1.0f;
}

