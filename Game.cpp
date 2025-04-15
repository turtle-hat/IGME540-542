#include "Game.h"
#include "Graphics.h"
#include "Vertex.h"
#include "Input.h"
#include "PathHelpers.h"
#include "Window.h"
#include "WICTextureLoader.h"

#include <cmath>

// Needed for a helper function to load pre-compiled shader files
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>

// This code assumes files are in "ImGui" subfolder!
// Adjust as necessary for your own folder structure and project setup
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"

// For the DirectX Math library
using namespace std;
using namespace DirectX;

// --------------------------------------------------------
// Called once per program, after the window and graphics API
// are initialized but before the game loop begins
// --------------------------------------------------------
void Game::Initialize()
{
	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	InitializeSimulationParameters();
	LoadShaders();
	CreateLights();
	CreateMaterials();
	BuildShadowMap();
	BuildShadowMatrices();
	CreateGeometry();
	CreateCameras();
	CreateSkyboxes();
	BuildParticleResources();
	CreateParticleEmitters();
	BuildPostProcesses();

	// Set initial graphics API state
	//  - These settings persist until we change them
	//  - Some of these, like the primitive topology & input layout, probably won't change
	//  - Others, like setting shaders, will need to be moved elsewhere later
	{
		// Tell the input assembler (IA) stage of the pipeline what kind of
		// geometric primitives (points, lines or triangles) we want to draw.  
		// Essentially: "What kind of shape should the GPU draw with our vertices?"
		Graphics::Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	// Initialize ImGui itself & platform/renderer backends
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplWin32_Init(Window::Handle());
	ImGui_ImplDX11_Init(Graphics::Device.Get(), Graphics::Context.Get());
	// Pick a style (uncomment one of these 3)
	//ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();
	ImGui::StyleColorsClassic();
}


// --------------------------------------------------------
// Clean up memory or objects created by this class
// 
// Note: Using smart pointers means there probably won't
//       be much to manually clean up here!
// --------------------------------------------------------
Game::~Game()
{
	// ImGui cleanup
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	// Cleanup other variables from helper methods
	CleanupSimulationParameters();
}


// --------------------------------------------------------
// Loads shaders from compiled shader object (.cso) files
// and creates the Input Layout that describes our vertex
// data to the rendering pipeline. 
// - Input Layout creation is done here because it must 
//    be verified against vertex shader byte code
// - We'll have that byte code already loaded below
// --------------------------------------------------------
void Game::LoadShaders()
{
	// Load shaders in with SimpleShader
	


	// MATERIAL SHADERS
	
	// VERTEX SHADERS
	AddVertexShader(L"VS_DiffuseSpecular.cso",	vsDiffuseSpecular);
	AddVertexShader(L"VS_DiffuseNormal.cso",	vsDiffuseNormal);
	AddVertexShader(L"VS_PBR.cso",				vsPBR);
	AddVertexShader(L"VS_Skybox.cso",			vsSkybox);
	AddVertexShader(L"VS_ShadowMap.cso",		vsShadowMap);

	// PIXEL SHADERS
	AddPixelShader(L"PS_DiffuseSpecular.cso",	psDiffuseSpecular);
	AddPixelShader(L"PS_DiffuseNormal.cso",		psDiffuseNormal);
	AddPixelShader(L"PS_PBR.cso",				psPBR);
	AddPixelShader(L"PS_Normals.cso",			psNormals);
	AddPixelShader(L"PS_UVs.cso",				psUVs);
	AddPixelShader(L"PS_Custom.cso",			psCustom);
	AddPixelShader(L"PS_Skybox.cso",			psSkybox);



	// POST-PROCESS SHADERS

	// VERTEX SHADER
	AddVertexShader(L"VS_PostProcess.cso",			ppVS);

	// PIXEL SHADERS
	AddPixelShader(L"PS_PostProcess_Blur.cso",		ppBlurPS);
	AddPixelShader(L"PS_PostProcess_Dither.cso",	ppDitherPS);



	// PARTICLE SHADERS
	AddVertexShader(L"VS_Particle.cso",	vsParticle);
	AddPixelShader(L"PS_Particle.cso",	psParticle);
}

// --------------------------------------------------------
// Loads textures and creates materials
// --------------------------------------------------------
void Game::CreateMaterials()
{
	// Load textures
	// TEXTURES 0-13
	AddTexture(L"../../Assets/Textures/T_bronze_AM.png");
	AddTexture(L"../../Assets/Textures/T_bronze_NR.png");
	AddTexture(L"../../Assets/Textures/T_cobblestone_AM.png");
	AddTexture(L"../../Assets/Textures/T_cobblestone_NR.png");
	AddTexture(L"../../Assets/Textures/T_floor_AM.png");
	AddTexture(L"../../Assets/Textures/T_floor_NR.png");
	AddTexture(L"../../Assets/Textures/T_paint_AM.png");
	AddTexture(L"../../Assets/Textures/T_paint_NR.png");
	AddTexture(L"../../Assets/Textures/T_rough_AM.png");
	AddTexture(L"../../Assets/Textures/T_rough_NR.png");
	AddTexture(L"../../Assets/Textures/T_scratched_AM.png");
	AddTexture(L"../../Assets/Textures/T_scratched_NR.png");
	AddTexture(L"../../Assets/Textures/T_wood_AM.png");
	AddTexture(L"../../Assets/Textures/T_wood_NR.png");
	// TEXTURES 14-20
	AddTexture(L"../../Assets/Textures/Particles/circle_05.png");
	AddTexture(L"../../Assets/Textures/Particles/flame_animated.png");
	AddTexture(L"../../Assets/Textures/Particles/spark_01.png");
	AddTexture(L"../../Assets/Textures/Particles/spark_02.png");
	AddTexture(L"../../Assets/Textures/Particles/spark_03.png");
	AddTexture(L"../../Assets/Textures/Particles/spark_04.png");
	AddTexture(L"../../Assets/Textures/Particles/z_Pepper.png");


	// Set default sampler state settings
	pSamplerFilter = D3D11_FILTER_ANISOTROPIC;
	pAnisotropyPower = 4;

	// Create the sampler state
	SetGlobalSamplerState(pSamplerFilter, (int)pow(2, pAnisotropyPower));

	// Create materials
	// MATERIALS 0-2
	AddMaterial("Mat_Normals",		vsDiffuseSpecular, psNormals);
	AddMaterial("Mat_UVs",			vsDiffuseSpecular, psUVs);
	AddMaterial("Mat_Custom",		vsDiffuseSpecular, psCustom);

	// MATERIALS 3-9
	AddPBRMaterial("Mat_Bronze_PBR",		vsPBR, psPBR, 1.0f, 1.0f);
	materials[3]->AddTextureSRV("MapAlbedoMetalness", textures[0]);
	materials[3]->AddTextureSRV("MapNormalRoughness", textures[1]);
	materials[3]->AddSampler("BasicSampler", samplerState);

	AddPBRMaterial("Mat_Cobblestone_PBR",	vsPBR, psPBR, 1.0f, 1.0f);
	materials[4]->AddTextureSRV("MapAlbedoMetalness", textures[2]);
	materials[4]->AddTextureSRV("MapNormalRoughness", textures[3]);
	materials[4]->AddSampler("BasicSampler", samplerState);

	AddPBRMaterial("Mat_Floor_PBR",			vsPBR, psPBR, 1.0f, 1.0f);
	materials[5]->AddTextureSRV("MapAlbedoMetalness", textures[4]);
	materials[5]->AddTextureSRV("MapNormalRoughness", textures[5]);
	materials[5]->AddSampler("BasicSampler", samplerState);

	AddPBRMaterial("Mat_Paint_PBR",			vsPBR, psPBR, 1.0f, 1.0f);
	materials[6]->AddTextureSRV("MapAlbedoMetalness", textures[6]);
	materials[6]->AddTextureSRV("MapNormalRoughness", textures[7]);
	materials[6]->AddSampler("BasicSampler", samplerState);

	AddPBRMaterial("Mat_Rough_PBR",			vsPBR, psPBR, 1.0f, 1.0f);
	materials[7]->AddTextureSRV("MapAlbedoMetalness", textures[8]);
	materials[7]->AddTextureSRV("MapNormalRoughness", textures[9]);
	materials[7]->AddSampler("BasicSampler", samplerState);

	AddPBRMaterial("Mat_Scratched_PBR",		vsPBR, psPBR, 1.0f, 1.0f);
	materials[8]->AddTextureSRV("MapAlbedoMetalness", textures[10]);
	materials[8]->AddTextureSRV("MapNormalRoughness", textures[11]);
	materials[8]->AddSampler("BasicSampler", samplerState);

	AddPBRMaterial("Mat_Wood_PBR",			vsPBR, psPBR, 1.0f, 1.0f);
	materials[9]->AddTextureSRV("MapAlbedoMetalness", textures[12]);
	materials[9]->AddTextureSRV("MapNormalRoughness", textures[13]);
	materials[9]->AddSampler("BasicSampler", samplerState);
	materials[9]->SetUVScale(XMFLOAT2(3.0f, 3.0f));

	// MATERIALS 10-12
	AddMaterial("Mat_Pepper_Emitter",		vsParticle, psParticle, XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
	materials[10]->AddTextureSRV("ParticleTexture", textures[16]);
	materials[10]->AddSampler("BasicSampler", samplerState);
}

// --------------------------------------------------------
// Creates the geometry we're going to draw and all
// entities in the scene
// --------------------------------------------------------
void Game::CreateGeometry()
{
	// Create meshes from OBJ models
	// MESHES 0-6
	meshes.push_back(make_shared<Mesh>("M_Cube", FixPath(L"../../Assets/Models/cube.obj").c_str()));
	meshes.push_back(make_shared<Mesh>("M_Cylinder", FixPath(L"../../Assets/Models/cylinder.obj").c_str()));
	meshes.push_back(make_shared<Mesh>("M_Helix", FixPath(L"../../Assets/Models/helix.obj").c_str()));
	meshes.push_back(make_shared<Mesh>("M_Quad-SingleSided", FixPath(L"../../Assets/Models/quad.obj").c_str()));
	meshes.push_back(make_shared<Mesh>("M_Quad-DoubleSided", FixPath(L"../../Assets/Models/quad_double_sided.obj").c_str()));
	meshes.push_back(make_shared<Mesh>("M_Sphere", FixPath(L"../../Assets/Models/sphere.obj").c_str()));
	meshes.push_back(make_shared<Mesh>("M_Torus", FixPath(L"../../Assets/Models/torus.obj").c_str()));

	// ENTITIES 0-6
	AddEntity("E_ObjectBronze",			0, 3, XMFLOAT3(-9.0f,  0.0f, 0.0f));
	AddEntity("E_ObjectCobblestone",	1, 4, XMFLOAT3(-6.0f,  0.0f, 0.0f));
	AddEntity("E_ObjectFloor",			2, 5, XMFLOAT3(-3.0f,  0.0f, 0.0f));
	AddEntity("E_ObjectPaint",			3, 6, XMFLOAT3( 0.0f, -1.0f, 0.0f));
	AddEntity("E_ObjectRough",			4, 7, XMFLOAT3( 3.0f, -1.0f, 0.0f));
	AddEntity("E_ObjectScratched",		5, 8, XMFLOAT3( 6.0f,  0.0f, 0.0f));
	AddEntity("E_ObjectWood",			6, 9, XMFLOAT3( 9.0f,  0.0f, 0.0f));

	// ENTITY 7-9
	AddEntity("E_Floor",				0, 9, XMFLOAT3( 0.0f, -2.0f, 0.0f));
	entities[7]->GetTransform()->Scale(XMFLOAT3(50.0f, 0.125f, 50.0f));
	AddEntity("E_Wall1",				0, 6, XMFLOAT3( -12.0f, 1.0f, 0.0f));
	entities[8]->GetTransform()->Scale(XMFLOAT3(0.125f, 3.0f, 5.0f));
	AddEntity("E_Wall2",				0, 6, XMFLOAT3( 0.0f, 1.0f, 5.0f));
	entities[9]->GetTransform()->Scale(XMFLOAT3(12.0f, 3.0f, 0.125f));

	// ENTITIES 10-11
	AddEntity("E_BouncerSpring",		2, 7, XMFLOAT3( 0.0f, -1.0f, 3.0f));
	AddEntity("E_BouncerCylinder",		1, 3, XMFLOAT3( 0.0f, 0.0f, 3.0f));
	entities[11]->GetTransform()->Scale(XMFLOAT3(1.2f, 1.0f, 1.2f));
}

// --------------------------------------------------------
// Creates the lights to be rendered in the scene
// --------------------------------------------------------
void Game::CreateLights() {
	// Create lights
	// LIGHT 0
	AddLightSpot(XMFLOAT3(5.0f, 3.0f, -3.0f), XMFLOAT3(-0.25f, -0.5f, 0.5f), XMFLOAT3(1.0f, 1.0f, 1.0f), 1.0f, 25.0f, 0.0f, 1.4f, true);
	lights[0].Type = LIGHT_TYPE_DIRECTIONAL;
	// LIGHTS 1-2
	AddLightDirectional(XMFLOAT3(1.0f, -1.0f, -1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 0.5f, true);
	AddLightDirectional(XMFLOAT3(-1.0f, 1.0f, -1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), 1.0f, false);
}

// --------------------------------------------------------
// Creates the cameras we'll need in the scene
// --------------------------------------------------------
void Game::CreateCameras() {
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


void Game::CreateSkyboxes() {
	// SKYBOXES 0
	AddSkybox("SB_Blank", L"../../Assets/Textures/Cubemaps/Blank/CM_Blank", XMFLOAT3(0.0f, 0.0f, 0.0f));

	// SKYBOXES 1-4
	AddSkybox("SB_CloudsBlue", L"../../Assets/Textures/Cubemaps/CloudsBlue/CM_CloudsBlue", XMFLOAT3(0.0f, 0.0f, 0.075f));
	// Set this as the environment map used by each material with normal map calculations
	SetMaterialEnvironmentMaps(skyboxes[1]);
	AddSkybox("SB_CloudsPink", L"../../Assets/Textures/Cubemaps/CloudsPink/CM_CloudsPink", XMFLOAT3(0.025f, 0.0f, 0.05f));
	AddSkybox("SB_ColdSunset", L"../../Assets/Textures/Cubemaps/ColdSunset/CM_ColdSunset", XMFLOAT3(0.05f, 0.05f, 0.125f));
	AddSkybox("SB_Planet", L"../../Assets/Textures/Cubemaps/Planet/CM_Planet", XMFLOAT3(0.0f, 0.0f, 0.025f));
}

void Game::CreateParticleEmitters()
{
	ParticleEmitterParams peParamsPepper = {};
	peParamsPepper.emitFrequency			= 1.0f;
	peParamsPepper.lifetime					= 2.0f;
	peParamsPepper.acceleration				= XMFLOAT3(0.0f, -1.0f, 0.0f);
	peParamsPepper.startPositionOffset		= XMFLOAT3(0.0f, 0.0f, 0.0f);
	peParamsPepper.startPositionVariance	= XMFLOAT3(10.0f, 1.0f, 10.0f);
	peParamsPepper.startColor				= XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	peParamsPepper.finalColor				= XMFLOAT4(1.0f, 1.0f, 1.0f, 0.0f);

	particleEmitters.push_back(make_shared<ParticleEmitter>(
		"PE_Pepper", materials[10], peParamsPepper, 20
	));
}

// --------------------------------------------------------
// Handle resizing to match the new window size
//  - Eventually, we'll want to update our 3D camera
// --------------------------------------------------------
void Game::OnResize()
{
	// Only run these functions if the Window has been initialized
	if (isInitialized) {
		// If cameras exist, resize it
		if (cameras.size() > 0) {
			cameras[pCameraCurrent]->SetAspect((Window::Width() + 0.0f) / Window::Height());
		}

		ppPixelSize = XMFLOAT2(1.0f / Window::Width(), 1.0f / Window::Height());

		RebuildPostProcesses();
	}
}


// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	// Update current camera
	cameras[pCameraCurrent]->Update(deltaTime);

	// Rotate meshes
	for (int i = 0; i <= 6; i++) {
		entities[i]->GetTransform()->Rotate(0.0f, deltaTime * pObjectRotationSpeed, 0.0f);
	}

	// Move bouncer
	shared_ptr<Transform> bouncerSpringTransform = entities[10]->GetTransform();
	bouncerSpringTransform->SetPosition(0.0f,
		sin(totalTime * 4.0f) * 2.0f - sin((totalTime + 0.225f) * 8.0f) * 0.8f,
		3.0f);
	bouncerSpringTransform->SetScale(1.0f,
		1.2f + sin((totalTime + 0.225f) * 8.0f) * 0.8f,
		1.0f);
	entities[11]->GetTransform()->SetPosition(0.0f,
		sin(totalTime * 4.0f) * 2.0f + 2.0f,
		3.0f);

	// Update all emitters
	for (auto emitter : particleEmitters) {
		emitter->Update(deltaTime, totalTime);
	}

	ImGuiUpdate(deltaTime);
	ImGuiBuild();

	// Example input checking: Quit if the escape key is pressed
	if (Input::KeyDown(VK_ESCAPE))
		Window::Quit();
}


// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Frame START
	// - These things should happen ONCE PER FRAME
	// - At the beginning of Game::Draw() before drawing *anything*
	{
		// Clear the back buffer (erase what's on screen) and depth buffer
		Graphics::Context->ClearRenderTargetView(Graphics::BackBufferRTV.Get(),	pBackgroundColor);
		Graphics::Context->ClearDepthStencilView(Graphics::DepthBufferDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}


	// RENDER SHADOW MAP
	// Clear shadow map depth buffer
	Graphics::Context->ClearDepthStencilView(shadowDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	// Set shadow map rasterizer state
	Graphics::Context->RSSetState(shadowRasterizer.Get());

	// Set render target to nothing, depth buffer to shadow map
	ID3D11RenderTargetView* nullRTV{};
	Graphics::Context->OMSetRenderTargets(1, &nullRTV, shadowDSV.Get());

	// Change viewport
	D3D11_VIEWPORT viewport = {};
	viewport.Width		= (float)pShadowResolution;
	viewport.Height		= (float)pShadowResolution;
	viewport.MaxDepth	= 1.0f;
	Graphics::Context->RSSetViewports(1, &viewport);

	// Set shaders
	Graphics::Context->PSSetShader(0, 0, 0);
	vsShadowMap->SetShader();
	vsShadowMap->SetMatrix4x4("view", shadowLightViewMatrix);
	vsShadowMap->SetMatrix4x4("projection", shadowLightProjectionMatrix);

	// Draw all entities
	for (int i = 0; i < entities.size(); i++) {
		vsShadowMap->SetMatrix4x4("world", entities[i]->GetTransform()->GetWorld());
		vsShadowMap->CopyAllBufferData();

		// Draw the entity's mesh
		entities[i]->GetMesh()->Draw();
	}

	// Reset viewport, render target, depth buffer, and rasterizer state for normal rendering
	viewport.Width = (float)Window::Width();
	viewport.Height = (float)Window::Height();
	Graphics::Context->RSSetViewports(1, &viewport);


	// POST-PROCESS SETUP
	// Clear RTVs for active post-processes
	if (ppBlurRun) {
		Graphics::Context->ClearRenderTargetView(ppBlurRTV.Get(), pBackgroundColor);
	}
	if (ppDitherRun) {
		Graphics::Context->ClearRenderTargetView(ppDitherRTV.Get(), pBackgroundColor);
	}



	// Set render target based on the first post-process that will run, if any
	if (ppBlurRun) {
		// Set render target to our blur's render target
		Graphics::Context->OMSetRenderTargets(
			1,
			ppBlurRTV.GetAddressOf(),
			Graphics::DepthBufferDSV.Get()
		);
	}
	// If no blur but yes dither, set render target to the dither's texture
	else if (ppDitherRun) {
		// Set render target to our dither's render target
		Graphics::Context->OMSetRenderTargets(
			1,
			ppDitherRTV.GetAddressOf(),
			Graphics::DepthBufferDSV.Get()
		);
	}
	// If no blur or dither, set render target to back buffer
	else {
		Graphics::Context->OMSetRenderTargets(
			1,
			Graphics::BackBufferRTV.GetAddressOf(),
			Graphics::DepthBufferDSV.Get()
		);
	}

	Graphics::Context->RSSetState(0);

	// RENDER OBJECTS
	// Loop through every entity and draw it
	for (int i = 0; i < entities.size(); i++) {

		// Get entity material
		std::shared_ptr<Material> material = entities[i]->GetMaterial();
		// Prepare the material for drawing
		material->PrepareMaterial();

		// Get entity's shaders
		std::shared_ptr<SimpleVertexShader> vs = material->GetVertexShader();
		std::shared_ptr<SimplePixelShader> ps = material->GetPixelShader();

		// Set vertex and pixel shaders
		vs->SetShader();
		ps->SetShader();

		// Fill constant buffers with entity's data
		// VERTEX
		vs->SetMatrix4x4("tfWorld", entities[i]->GetTransform()->GetWorld());
		vs->SetMatrix4x4("tfView", cameras[pCameraCurrent]->GetViewMatrix());
		vs->SetMatrix4x4("tfProjection", cameras[pCameraCurrent]->GetProjectionMatrix());
		vs->SetMatrix4x4("tfWorldIT", entities[i]->GetTransform()->GetWorldInverseTranspose());
		vs->SetMatrix4x4("tfShadowView", shadowLightViewMatrix);
		vs->SetMatrix4x4("tfShadowProjection", shadowLightProjectionMatrix);
		// PIXEL
		ps->SetFloat4("colorTint", material->GetColorTint());
		ps->SetFloat("roughness", material->GetRoughness());
		ps->SetFloat3("cameraPosition", cameras[pCameraCurrent]->GetTransform()->GetPosition());

		ps->SetFloat2("uvPosition", material->GetUVPosition());
		ps->SetFloat2("uvScale", material->GetUVScale());

		// Set lights on pixel shader
		ps->SetData("lights", &lights[0], sizeof(Light) * (int)lights.size());
		// MATERIAL-SPECIFIC PIXEL SHADER CONSTANT BUFFER INPUTS
		if (material->GetName() == "Mat_Custom") {
			ps->SetFloat("totalTime", totalTime);
			ps->SetFloat2("imageCenter", pMatCustomImage);
			ps->SetFloat2("zoomCenter", pMatCustomZoom);
			ps->SetInt("maxIterations", pMatCustomIterations);
		}

		if (material->isPBR) {
			// Only use metalness for PBR materials
			ps->SetFloat("metalness", material->GetMetalness());
		}
		else {
			// Only use ambient light for non-PBR materials
			ps->SetFloat3("lightAmbient", skyboxAmbientColors[pSkyboxCurrent]);
		}

		// COPY DATA TO CONSTANT BUFFERS
		vs->CopyAllBufferData();
		ps->CopyAllBufferData();

		// Draw the entity's mesh
		entities[i]->GetMesh()->Draw();
	}

	// Draw the selected skybox
	skyboxes[pSkyboxCurrent]->Draw(cameras[pCameraCurrent]);

	// PARTICLES
	{
		// Set 
		Graphics::Context->OMSetDepthStencilState(particleDepthStencilState.Get(), 0);
		Graphics::Context->OMSetBlendState(particleBlendState.Get(), NULL, 0xffffffff);

		for (auto emitter : particleEmitters) {
			emitter->Draw(cameras[pCameraCurrent], totalTime);
		}

		// Reset to default states for post-processing
		Graphics::Context->OMSetDepthStencilState(0, 0);
		Graphics::Context->OMSetBlendState(0, NULL, 0xffffffff);
		Graphics::Context->RSSetState(0);
	}

	// POST-PROCESS
	// Blur
	if (ppBlurRun) {
		// If doing dither after this, set render target to dither's RTV
		if (ppDitherRun) {
			Graphics::Context->OMSetRenderTargets(1, ppDitherRTV.GetAddressOf(), 0);
		}
		// If not, set render target to back buffer
		else {
			Graphics::Context->OMSetRenderTargets(1, Graphics::BackBufferRTV.GetAddressOf(), 0);
		}

		// Bind shaders
		ppVS->SetShader();
		ppBlurPS->SetShader();
		ppBlurPS->SetShaderResourceView("BaseRender", ppBlurSRV.Get());
		ppBlurPS->SetSamplerState("ClampSampler", ppSampler.Get());

		// Set constant buffer data
		ppBlurPS->SetInt("blurRadius", ppBlurRadius);
		ppBlurPS->SetFloat2("pixelSize", ppPixelSize);
		ppBlurPS->CopyAllBufferData();
		
		// Draw
		Graphics::Context->Draw(3, 0);
	}
	// Dither
	if (ppDitherRun) {
		// Set render target to back buffer
		Graphics::Context->OMSetRenderTargets(1, Graphics::BackBufferRTV.GetAddressOf(), 0);

		// Bind shaders
		ppVS->SetShader();
		ppDitherPS->SetShader();
		ppDitherPS->SetShaderResourceView("BaseRender", ppDitherSRV.Get());
		// Set the appropriate dither map 
		ppDitherPS->SetShaderResourceView("MapDither", ppDitherMaps[ppDitherMapTextureID].Get());
		ppDitherPS->SetSamplerState("ClampSampler", ppSampler.Get());
		// Set the appropriate sampler
		ppDitherPS->SetSamplerState("DitherMapSampler", ppDitherMapSampler.Get());

		// Set constant buffer data
		ppDitherPS->SetFloat3("colorLight", ppDitherColorLight);
		ppDitherPS->SetInt("ditherPixelSize", ppDitherPixelSize);

		ppDitherPS->SetFloat3("colorDark", ppDitherColorDark);
		ppDitherPS->SetFloat("bias", ppDitherBias);

		XMFLOAT3 camRotation = cameras[pCameraCurrent]->GetTransform()->GetRotation();
		float camFov = cameras[pCameraCurrent]->GetFov();

		ppDitherPS->SetFloat3("cameraRotation", camRotation);
		ppDitherPS->SetFloat("cameraFov", camFov);

		ppDitherPS->SetFloat2("pixelSize", ppPixelSize);
		ppDitherPS->SetFloat2("screenSize", XMFLOAT2((float)Window::Width(), (float)Window::Height()));
		ppDitherPS->CopyAllBufferData();

		// Draw
		Graphics::Context->Draw(3, 0);
	}



	// RENDER IMGUI
	ImGui::Render(); // Turns this frame’s UI into renderable triangles
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData()); // Draws it to the screen
	


	// FRAME END
	// - These should happen exactly ONCE PER FRAME
	// - At the very end of the frame (after drawing *everything*)
	{
		// Present at the end of the frame
		bool vsync = Graphics::VsyncState();
		Graphics::SwapChain->Present(
			vsync ? 1 : 0,
			vsync ? 0 : DXGI_PRESENT_ALLOW_TEARING);

		// Re-bind back buffer and depth buffer after presenting
		Graphics::Context->OMSetRenderTargets(
			1,
			Graphics::BackBufferRTV.GetAddressOf(),
			Graphics::DepthBufferDSV.Get());

		// Unbind all SRVs at the end of the frame
		ID3D11ShaderResourceView* nullSRVs[128] = {};
		Graphics::Context->PSSetShaderResources(0, 128, nullSRVs);
	}
}



// --------------------------------------------------------
// 
//                 CUSTOM HELPER METHODS
// 
// --------------------------------------------------------



// --------------------------------------------------------
// Initializes the all simulation variables
// to their default values
// --------------------------------------------------------
void Game::InitializeSimulationParameters() {
	
	// IMGUI PARAMETERS

	igShowDemo = false;
	
	// Uncomment for original cornflower blue
	//float bgColor[4] = { 0.4f, 0.6f, 0.75f, 1.0f };
	float bgColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	memcpy(pBackgroundColor, bgColor, sizeof(float) * 4);
	pObjectRotationSpeed = 1.0f;

	pSelectedSamplerFilter = 5;

	pMatCustomIterations = 100;
	pMatCustomImage = XMFLOAT2(-1.77f, -0.02f);
	pMatCustomZoom = XMFLOAT2(-0.2f, -0.61f);

	pCameraCurrent = 0;
	pSkyboxCurrent = 1;

	pRenderShadows = true;
	pShadowResolutionExponent = 10;
	pShadowResolution = 1024;
	pShadowAreaWidth = 30.0f;
	pShadowAreaCenter = XMFLOAT3(0.0f, -5.0f, 0.0f);
	pShadowLightDistance = 500.0f;

	ppPixelSize = XMFLOAT2(1.0f / Window::Width(), 1.0f / Window::Height());
	
	ppBlurRun = false;
	ppBlurRadius = 5;

	ppDitherRun = false;
	ppDitherMapTextureID = 3;
	ppDitherPixelSize = 2;
	ppDitherBias = -0.25f;
	ppDitherColorLight = XMFLOAT3(1.0f, 1.0f, 1.0f);
	ppDitherColorDark = XMFLOAT3(0.25f, 0.25f, 0.25f);

	// Framerate graph variables
	igFrameGraphSamples = new float[IG_FRAME_GRAPH_TOTAL_SAMPLES];
	// Zero out all of our sample array
	for (int i = 0; i < IG_FRAME_GRAPH_TOTAL_SAMPLES; i++) {
		igFrameGraphSamples[i] = 0;
	}
	igFrameGraphSampleCount = 240;
	igFrameGraphSampleRate = 60.0f;
	igFrameGraphNextSampleTime = 0.0;
	igFrameGraphSampleOffset = 0;
	igFrameGraphHighest = 0.0f;
	igFrameGraphDoAnimate = true;

	isInitialized = true;
}

// --------------------------------------------------------
// Adds a vertex shader to the list of vertex shaders
// --------------------------------------------------------
void Game::AddVertexShader(const wchar_t* _path, std::shared_ptr<SimpleVertexShader>& _shader)
{
	_shader = make_shared<SimpleVertexShader>(
		Graphics::Device,
		Graphics::Context,
		FixPath(_path).c_str()
	);
}

// --------------------------------------------------------
// Adds a pixel shader to the list of pixel shaders
// --------------------------------------------------------
void Game::AddPixelShader(const wchar_t* _path, std::shared_ptr<SimplePixelShader>& _shader)
{
	_shader = make_shared<SimplePixelShader>(
		Graphics::Device,
		Graphics::Context,
		FixPath(_path).c_str()
	);
}

// --------------------------------------------------------
// Adds a texture to the list of material textures
// --------------------------------------------------------
void Game::AddTexture(const wchar_t* _path)
{
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;

	LoadTexture(_path, srv);

	textures.push_back(srv);
}

// --------------------------------------------------------
// Adds a texture to a given list of textures
// --------------------------------------------------------
void Game::LoadTexture(const wchar_t* _path, std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>>& _srvVector)
{
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;

	LoadTexture(_path, srv);

	_srvVector.push_back(srv);
}

// --------------------------------------------------------
// Loads a texture into the given SRV
// --------------------------------------------------------
void Game::LoadTexture(const wchar_t* _path, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& _srv)
{
	CreateWICTextureFromFile(
		Graphics::Device.Get(),
		Graphics::Context.Get(),
		FixPath(_path).c_str(),
		nullptr,
		_srv.GetAddressOf()
	);
}

// --------------------------------------------------------
// Adds a Material to the list of Materials
// --------------------------------------------------------
void Game::AddMaterial(const char* _name, std::shared_ptr<SimpleVertexShader> _vertexShader, std::shared_ptr<SimplePixelShader> _pixelShader, DirectX::XMFLOAT4 _colorTint, float _roughness, bool _useGlobalEnvironmentMap)
{
	materials.push_back(make_shared<Material>(
		_name,
		_vertexShader,
		_pixelShader,
		_colorTint,
		_roughness,
		_useGlobalEnvironmentMap
	));
}

void Game::AddMaterial(const char* _name, std::shared_ptr<SimpleVertexShader> _vertexShader, std::shared_ptr<SimplePixelShader> _pixelShader, DirectX::XMFLOAT4 _colorTint, float _roughness)
{
	AddMaterial(_name, _vertexShader, _pixelShader, _colorTint, _roughness, false);
}

void Game::AddMaterial(const char* _name, std::shared_ptr<SimpleVertexShader> _vertexShader, std::shared_ptr<SimplePixelShader> _pixelShader, DirectX::XMFLOAT4 _colorTint)
{
	AddMaterial(_name, _vertexShader, _pixelShader, _colorTint, 0.0f, false);
}

void Game::AddMaterial(const char* _name, std::shared_ptr<SimpleVertexShader> _vertexShader, std::shared_ptr<SimplePixelShader> _pixelShader, float _roughness, bool _useGlobalEnvironmentMap)
{
	AddMaterial(_name, _vertexShader, _pixelShader, XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), _roughness, _useGlobalEnvironmentMap);
}

void Game::AddMaterial(const char* _name, std::shared_ptr<SimpleVertexShader> _vertexShader, std::shared_ptr<SimplePixelShader> _pixelShader, float _roughness)
{
	AddMaterial(_name, _vertexShader, _pixelShader, XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), _roughness, false);
}

void Game::AddMaterial(const char* _name, std::shared_ptr<SimpleVertexShader> _vertexShader, std::shared_ptr<SimplePixelShader> _pixelShader)
{
	AddMaterial(_name, _vertexShader, _pixelShader, XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), 0.0f, false);
}

void Game::AddPBRMaterial(const char* _name, std::shared_ptr<SimpleVertexShader> _vertexShader, std::shared_ptr<SimplePixelShader> _pixelShader, DirectX::XMFLOAT4 _colorTint, float _roughness, float _metalness)
{
	materials.push_back(make_shared<Material>(
		_name,
		_vertexShader,
		_pixelShader,
		_colorTint,
		_roughness,
		_metalness
	));
}

void Game::AddPBRMaterial(const char* _name, std::shared_ptr<SimpleVertexShader> _vertexShader, std::shared_ptr<SimplePixelShader> _pixelShader, float _roughness, float _metalness)
{
	AddPBRMaterial(_name, _vertexShader, _pixelShader, XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), 1.0f, 1.0f);
}

// --------------------------------------------------------
// Adds an Entity to the list of Entities
// --------------------------------------------------------
void Game::AddEntity(const char* _name, unsigned int _meshIndex, unsigned int _materialIndex, DirectX::XMFLOAT3 _position)
{
	shared_ptr<Entity> entity = make_shared<Entity>(
		_name,
		meshes[_meshIndex],
		materials[_materialIndex]
	);

	entity->GetTransform()->SetPosition(_position);
	entities.push_back(entity);
}

// --------------------------------------------------------
// Adds a Light of a given type to the list of Lights
// --------------------------------------------------------
void Game::AddLightDirectional(DirectX::XMFLOAT3 _direction, DirectX::XMFLOAT3 _color, float _intensity, bool _isActive)
{
	Light light = {};
	light.Type = LIGHT_TYPE_DIRECTIONAL;
	light.Direction = _direction;
	light.Color = _color;
	light.Intensity = _intensity;
	light.Active = _isActive ? 1 : 0;
	lights.push_back(light);
}

void Game::AddLightPoint(DirectX::XMFLOAT3 _position, DirectX::XMFLOAT3 _color, float _intensity, float _range, bool _isActive)
{
	Light light = {};
	light.Type = LIGHT_TYPE_POINT;
	light.Position = _position;
	light.Color = _color;
	light.Intensity = _intensity;
	light.Range = _range;
	light.Active = _isActive ? 1 : 0;
	lights.push_back(light);
}

void Game::AddLightSpot(DirectX::XMFLOAT3 _position, DirectX::XMFLOAT3 _direction, DirectX::XMFLOAT3 _color, float _intensity, float _range, float _innerAngle, float _outerAngle, bool _isActive)
{
	Light light = {};
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

// --------------------------------------------------------
// Adds a Skybox to the list of Skyboxes
// --------------------------------------------------------
void Game::AddSkybox(const char* _name, std::wstring _pathBase, DirectX::XMFLOAT3 _ambientColor)
{
	skyboxes.push_back(make_shared<Skybox>(
		_name, meshes[0], samplerState, vsSkybox, psSkybox,
		_pathBase
	));
	skyboxAmbientColors.push_back(_ambientColor);
}

// --------------------------------------------------------
// Sets the sampler state in the samplerState variable
// based on the given graphics settings
// --------------------------------------------------------
void Game::SetGlobalSamplerState(D3D11_FILTER _filter, int _anisotropyLevel) {
	// Reset sampler state if one exists
	samplerState = nullptr;

	// Sampler state description for changing sampler state with UI
	D3D11_SAMPLER_DESC samplerDescription = {};
	// Set default sampler state values
	samplerDescription.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDescription.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDescription.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDescription.Filter = _filter;
	samplerDescription.MaxAnisotropy = _anisotropyLevel;
	samplerDescription.MaxLOD = D3D11_FLOAT32_MAX;

	// Create sampler state
	Graphics::Device->CreateSamplerState(&samplerDescription, &samplerState);
}

// --------------------------------------------------------
// Sets the sampler states of each Material
// --------------------------------------------------------
void Game::SetMaterialSamplerStates() {
	for (int i = 0; i < materials.size(); i++) {
		materials[i]->AddSampler("BasicSampler", samplerState);
	}
}

// --------------------------------------------------------
// Sets the environment map of each Material, if they use one
// --------------------------------------------------------
void Game::SetMaterialEnvironmentMaps(shared_ptr<Skybox> _skybox)
{
	for (int i = 0; i < materials.size(); i++) {
		if (materials[i]->useGlobalEnvironmentMap) {
			materials[i]->AddTextureSRV("MapCube", _skybox->GetSRV());
		}
	}
}

// --------------------------------------------------------
// Builds all DirectX resources for the shadow map for the first time
// Code written by Chris Cascioli
// --------------------------------------------------------
void Game::BuildShadowMap() {
	// Create the rasterizer state for rendering the shadow map
	D3D11_RASTERIZER_DESC shadowRastDesc = {};
	shadowRastDesc.FillMode = D3D11_FILL_SOLID;
	shadowRastDesc.CullMode = D3D11_CULL_BACK;
	shadowRastDesc.DepthClipEnable = true;
	shadowRastDesc.DepthBias = 1000; // Min. precision units, not world units!
	shadowRastDesc.SlopeScaledDepthBias = 1.0f; // Bias more based on slope
	Graphics::Device->CreateRasterizerState(&shadowRastDesc, &shadowRasterizer);

	// Create the sampler for the shadow map texture
	D3D11_SAMPLER_DESC shadowSampDesc = {};
	shadowSampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	shadowSampDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
	shadowSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.BorderColor[0] = 1.0f; // Only need the first component
	Graphics::Device->CreateSamplerState(&shadowSampDesc, &shadowSampler);

	// Add shadow map sampler state to materials that need it
	for (int i = 3; i < materials.size(); i++) {
		materials[i]->AddSampler("ShadowSampler", shadowSampler);
	}

	// Build DSV and SRV
	RebuildShadowMap();
}

// --------------------------------------------------------
// Rebuilds just the shadow map's DSV and SRV
// Code written by Chris Cascioli
// --------------------------------------------------------
void Game::RebuildShadowMap()
{
	// Reset DSV and SRV pointers
	shadowDSV.ReleaseAndGetAddressOf();
	shadowSRV.ReleaseAndGetAddressOf();

	// Create the actual texture that will be the shadow map
	D3D11_TEXTURE2D_DESC shadowTexDesc = {};
	shadowTexDesc.Width = pShadowResolution;
	shadowTexDesc.Height = pShadowResolution;
	shadowTexDesc.ArraySize = 1;
	shadowTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	shadowTexDesc.CPUAccessFlags = 0;
	shadowTexDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	shadowTexDesc.MipLevels = 1;
	shadowTexDesc.MiscFlags = 0;
	shadowTexDesc.SampleDesc.Count = 1;
	shadowTexDesc.SampleDesc.Quality = 0;
	shadowTexDesc.Usage = D3D11_USAGE_DEFAULT;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> shadowTexture;
	Graphics::Device->CreateTexture2D(&shadowTexDesc, 0, shadowTexture.GetAddressOf());

	// Create the depth/stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC shadowDSVDesc = {};
	shadowDSVDesc.Format = DXGI_FORMAT_D32_FLOAT;
	shadowDSVDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	shadowDSVDesc.Texture2D.MipSlice = 0;
	Graphics::Device->CreateDepthStencilView(
		shadowTexture.Get(),
		&shadowDSVDesc,
		shadowDSV.GetAddressOf());

	// Create the SRV for the shadow map
	D3D11_SHADER_RESOURCE_VIEW_DESC shadowSRVDesc = {};
	shadowSRVDesc.Format = DXGI_FORMAT_R32_FLOAT;
	shadowSRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shadowSRVDesc.Texture2D.MipLevels = 1;
	shadowSRVDesc.Texture2D.MostDetailedMip = 0;
	Graphics::Device->CreateShaderResourceView(
		shadowTexture.Get(),
		&shadowSRVDesc,
		shadowSRV.GetAddressOf());

	// Add shadow map texture to materials that need it
	for (int i = 3; i < 10; i++) {
		materials[i]->AddTextureSRV("MapShadow", shadowSRV);
	}
}

void Game::BuildParticleResources()
{
	// Code for creating pipeline resources by Professor Chris Cascioli

	// Set up render states for particles (since all emitters might use similar ones)
	D3D11_DEPTH_STENCIL_DESC particleDepthDesc = {};
	particleDepthDesc.DepthEnable		= true; // READ from depth buffer
	particleDepthDesc.DepthWriteMask	= D3D11_DEPTH_WRITE_MASK_ZERO; // No depth WRITING
	particleDepthDesc.DepthFunc			= D3D11_COMPARISON_LESS; // Standard depth comparison
	Graphics::Device->CreateDepthStencilState(&particleDepthDesc, particleDepthStencilState.GetAddressOf());

	// Blend State
	D3D11_BLEND_DESC additiveBlendDesc = {};
	additiveBlendDesc.RenderTarget[0].BlendEnable			= true;
	additiveBlendDesc.RenderTarget[0].BlendOp				= D3D11_BLEND_OP_ADD; // Add both colors
	additiveBlendDesc.RenderTarget[0].BlendOpAlpha			= D3D11_BLEND_OP_ADD; // Add both alpha values
	additiveBlendDesc.RenderTarget[0].SrcBlend				= D3D11_BLEND_ONE;
	additiveBlendDesc.RenderTarget[0].DestBlend				= D3D11_BLEND_ONE;
	additiveBlendDesc.RenderTarget[0].SrcBlendAlpha			= D3D11_BLEND_ONE;
	additiveBlendDesc.RenderTarget[0].DestBlendAlpha		= D3D11_BLEND_ONE;
	additiveBlendDesc.RenderTarget[0].RenderTargetWriteMask	= D3D11_COLOR_WRITE_ENABLE_ALL;
	Graphics::Device->CreateBlendState(&additiveBlendDesc, particleBlendState.GetAddressOf());
}

// --------------------------------------------------------
// Builds or rebuilds the light's matrices
// --------------------------------------------------------
void Game::BuildShadowMatrices() {
	// Only do so if a light exists
	if (lights.size() > 0) {
		XMVECTOR lightDirection = XMLoadFloat3(&lights[0].Direction);

		if (lights[0].Type == LIGHT_TYPE_DIRECTIONAL) {
			// Build light View matrix
			XMStoreFloat4x4(&shadowLightViewMatrix, XMMatrixLookToLH(
				XMLoadFloat3(&pShadowAreaCenter) + (-lightDirection * pShadowLightDistance),
				lightDirection,
				XMVectorSet(0, 1, 0, 0)
			));

			// Build light Projection matrix
			XMStoreFloat4x4(&shadowLightProjectionMatrix,
				XMMatrixOrthographicLH(
					pShadowAreaWidth,
					pShadowAreaWidth,
					0.1f,	// Near clip value is hardcoded
					pShadowLightDistance
				)
			);
		} else {
			// Build light View matrix
			XMStoreFloat4x4(&shadowLightViewMatrix, XMMatrixLookToLH(
				XMLoadFloat3(&lights[0].Position),
				lightDirection,
				XMVectorSet(0, 1, 0, 0)
			));

			// Build light Projection matrix
			XMStoreFloat4x4(&shadowLightProjectionMatrix,
				XMMatrixPerspectiveFovLH(
					(lights[0].SpotOuterAngle * 2.0f),
					1.0f,
					0.1f,
					max(lights[0].Range, 0.2f)
				)
			);
		}
	}
}

// --------------------------------------------------------
// Builds post-process resources for the first time
// --------------------------------------------------------
void Game::BuildPostProcesses()
{
	// Describe sampler state for reading screen renders during post processing
	D3D11_SAMPLER_DESC ppSamplerDesc = {};
	ppSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	ppSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	ppSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	ppSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	ppSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	
	// Create the sampler state
	Graphics::Device->CreateSamplerState(&ppSamplerDesc, ppSampler.GetAddressOf());

	// Describe sampler state for reading the dither maps during post processing
	D3D11_SAMPLER_DESC ppDitherMapSamplerDesc = {};
	ppDitherMapSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	ppDitherMapSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	ppDitherMapSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	// Use point sampling for dither map
	ppDitherMapSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	ppDitherMapSamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the sampler state
	Graphics::Device->CreateSamplerState(&ppDitherMapSamplerDesc, ppDitherMapSampler.GetAddressOf());

	// Load dither map textures
	LoadTexture(L"../../Assets/Textures/T_bayer2px.png", ppDitherMaps);
	LoadTexture(L"../../Assets/Textures/T_bayer4px.png", ppDitherMaps);
	LoadTexture(L"../../Assets/Textures/T_bayer8px.png", ppDitherMaps);
	LoadTexture(L"../../Assets/Textures/T_bluenoise.png", ppDitherMaps);
	LoadTexture(L"../../Assets/Textures/T_bayer8px_test.png", ppDitherMaps);

	// Build other resources that may need to change during runtime
	RebuildPostProcesses();
}

// --------------------------------------------------------
// Builds or rebuilds the texture, RTV, and SRV for all
// post-processes
// --------------------------------------------------------
void Game::RebuildPostProcesses()
{
	RebuildPostProcess(ppBlurRTV, ppBlurSRV);
	RebuildPostProcess(ppDitherRTV, ppDitherSRV);
}

// --------------------------------------------------------
// Builds or rebuilds the texture, RTV, and SRV for one
// post-process
// --------------------------------------------------------
void Game::RebuildPostProcess(Microsoft::WRL::ComPtr<ID3D11RenderTargetView>& _ppRTV, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& _ppSRV)
{
	// Describe the texture
	D3D11_TEXTURE2D_DESC ppTexDesc = {};
	ppTexDesc.Width = Window::Width();
	ppTexDesc.Height = Window::Height();
	ppTexDesc.ArraySize = 1;
	ppTexDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	ppTexDesc.CPUAccessFlags = 0;
	ppTexDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	ppTexDesc.MipLevels = 1;
	ppTexDesc.MiscFlags = 0;
	ppTexDesc.SampleDesc.Count = 1;
	ppTexDesc.SampleDesc.Quality = 0;
	ppTexDesc.Usage = D3D11_USAGE_DEFAULT;

	// Create the texture
	Microsoft::WRL::ComPtr<ID3D11Texture2D> ppTex;
	Graphics::Device->CreateTexture2D(&ppTexDesc, 0, ppTex.ReleaseAndGetAddressOf());

	// Describe the Render Target View
	D3D11_RENDER_TARGET_VIEW_DESC ppRTVDesc = {};
	ppRTVDesc.Format = ppTexDesc.Format;
	ppRTVDesc.Texture2D.MipSlice = 0;
	ppRTVDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;

	// Create the RTV
	Graphics::Device->CreateRenderTargetView(ppTex.Get(), &ppRTVDesc, _ppRTV.ReleaseAndGetAddressOf());

	// Create the SRV
	Graphics::Device->CreateShaderResourceView(ppTex.Get(), 0, _ppSRV.ReleaseAndGetAddressOf());
}

// --------------------------------------------------------
// Prepares the ImGui UI window for being created
// --------------------------------------------------------
void Game::ImGuiUpdate(float _deltaTime) {
	// Put this all in a helper method that is called from Game::Update()
	// Feed fresh data to ImGui
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = _deltaTime;
	io.DisplaySize.x = (float)Window::Width();
	io.DisplaySize.y = (float)Window::Height();
	// Reset the frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	// Determine new input capture
	Input::SetKeyboardCapture(io.WantCaptureKeyboard);
	Input::SetMouseCapture(io.WantCaptureMouse);
	// Show the demo window if it's activated
	if (igShowDemo) {
		ImGui::ShowDemoWindow();
	}
}

// --------------------------------------------------------
// Builds the ImGui UI window structure each frame
// --------------------------------------------------------
void Game::ImGuiBuild() {
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

			if (ImGui::TreeNode("Framerate Graph")) {					// Graph of framerate over time
				// Sets tooptip of enclosing TreeNode
				ImGui::SetItemTooltip("Records the framerate over time\n(Slows down performance in Debug build while open)");
				
				ImGui::Spacing();
			
				// Plotting code taken from ImGui demo

				// If not animating or just initialized
				if (!igFrameGraphDoAnimate || igFrameGraphNextSampleTime == 0.0) {
					// Reset refresh time
					igFrameGraphNextSampleTime = ImGui::GetTime();
				}
				// Record however many samples should have been captured within the elapsed time this frame
				while (igFrameGraphNextSampleTime < ImGui::GetTime()) {
					// Get the framerate for this frame
					float framerate = ImGui::GetIO().Framerate;
					// Record framerate in array at the current place
					igFrameGraphSamples[igFrameGraphSampleOffset] = framerate;
					// Step one sample further in the array, mod the current number of samples we're supposed to capture
					igFrameGraphSampleOffset = (igFrameGraphSampleOffset + 1) % (igFrameGraphSampleCount);
					// Set next sample time based on sample rate
					igFrameGraphNextSampleTime += 1.0f / igFrameGraphSampleRate;
					// If this framerate is the highest recorded, rescale graph so it's in view
					if (framerate > igFrameGraphHighest) {
						igFrameGraphHighest = framerate;
					}
				}
				// Draw the graph
				ImGui::PlotLines("Framerate", igFrameGraphSamples, igFrameGraphSampleCount, igFrameGraphSampleOffset, "", 0.0f, igFrameGraphHighest, ImVec2(0, 100.0f));
			
				// Pauses or resumes the graph
				if (ImGui::Button(igFrameGraphDoAnimate ? "Pause Framerate Graph" : "Resume Framerate Graph")) {
					igFrameGraphDoAnimate = !igFrameGraphDoAnimate;
				}

				ImGui::SliderFloat("Graph Rate", &igFrameGraphSampleRate, 0.5f, 120.0f, "%3.1fHz", ImGuiSliderFlags_Logarithmic);
				ImGui::SetItemTooltip("How often the graph updates per second\n(Rate will update after next sample)");
			
				ImGui::SliderInt("Graph Scale", &igFrameGraphSampleCount, 1, 1000, "%d samples", ImGuiSliderFlags_Logarithmic);
				ImGui::SetItemTooltip("How many samples are shown on the graph\n(WARNING: Changing will mess up the current graph!)");

				ImGui::TreePop();
				ImGui::Spacing();
			}
			else {
				// Sets tooptip of enclosing TreeNode if it's closed
				ImGui::SetItemTooltip("Slows down performance in Debug mode");
				// Resets sample time
				igFrameGraphNextSampleTime = 0.0;
			}

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
		
		// Pass in the preview value visible before opening the combo
		const char* currentFilterName = SAMPLER_FILTER_STRINGS[pSelectedSamplerFilter];

		if (ImGui::BeginCombo("Filter", currentFilterName))
		{
			for (int i = 0; i < IM_ARRAYSIZE(SAMPLER_FILTER_STRINGS); i++)
			{
				const bool isSelected = (pSelectedSamplerFilter == i);
				// If option clicked,
				if (ImGui::Selectable(SAMPLER_FILTER_STRINGS[i], isSelected)) {
					// Set its index as the currently selected array item
					pSelectedSamplerFilter = i;
					// Set the filter itself
					pSamplerFilter = SAMPLER_FILTERS[i];
					// Set the sampler state of all materials
					SetGlobalSamplerState(pSamplerFilter, (int)pow(2, pAnisotropyPower));
					SetMaterialSamplerStates();
				}

				// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		// If anisotropic filtering is selecter, reveal anisotropy level slider
		if (pSamplerFilter == D3D11_FILTER_ANISOTROPIC) {
			if (ImGui::SliderInt("Anisotropy", &pAnisotropyPower, 0, 4)) {
				SetGlobalSamplerState(pSamplerFilter, (int)pow(2, pAnisotropyPower));
				SetMaterialSamplerStates();
			}
		}
		ImGui::SetItemTooltip("Sets anisotropy level to 2^n");
		
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

	if (ImGui::CollapsingHeader("Textures")) {					// Info about each texture
		ImGui::Spacing();

		ImGui::PushID("TEXTURE");
		for (int i = 0; i < textures.size(); i++) {

			// Each mesh gets its own Tree Node
			ImGui::PushID(i);
			if (ImGui::TreeNode("", "(%06d)", i)) {
				ImGui::Spacing();

				ImGui::Image((void*)textures[i].Get(), ImVec2(240, 240));

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
				XMFLOAT4 tint_xm = materials[i]->GetColorTint();
				float tint_f[4] = {tint_xm.x, tint_xm.y, tint_xm.z, tint_xm.w};
				float roughness = materials[i]->GetRoughness();
				vector<ID3D11ShaderResourceView*> textures = materials[i]->GetTextures();
				XMFLOAT2 uv_pos = materials[i]->GetUVPosition();
				XMFLOAT2 uv_sca = materials[i]->GetUVScale();

				// If the user has edited the tint this frame, change the material's tint
				if (ImGui::ColorEdit4("Tint", tint_f)) {
					materials[i]->SetColorTint(XMFLOAT4(tint_f));
				}
				// If the user has edited the material's roughness this frame, change the material's roughness
				if (ImGui::SliderFloat("Roughness", &roughness, 0.0f, 1.0f, "%.2f")) {
					materials[i]->SetRoughness(roughness);
				}

				// Only display metalness if the material uses PBR
				if (materials[i]->isPBR) {
					float metalness = materials[i]->GetMetalness();
					// If the user has edited the material's metalness this frame, change the material's roughness
					if (ImGui::SliderFloat("Metalness", &metalness, 0.0f, 1.0f, "%.2f")) {
						materials[i]->SetMetalness(metalness);
					}
				}

				// If any textures exist, include texture settings
				if (textures.size() > 0) {
					if (ImGui::DragFloat2("UV Position", (float*)&uv_pos, 0.01f, NULL, NULL, "%.2f")) {
						materials[i]->SetUVPosition(uv_pos);
					}
					if (ImGui::DragFloat2("UV Scale", (float*)&uv_sca, 0.01f, NULL, NULL, "%.2f")) {
						materials[i]->SetUVScale(uv_sca);
					}

					int non2DTextures = 0;

					ImGui::Text("Textures:");
					for (ID3D11ShaderResourceView* texture : textures) {
						D3D11_SHADER_RESOURCE_VIEW_DESC blah = {};
						texture->GetDesc(&blah);
						
						// Only display the texture if it's a Texture2D
						if (blah.ViewDimension == D3D11_SRV_DIMENSION_TEXTURE2D) {
							ImGui::Image(
								(void*)texture,
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

				// Custom settings for specific materials
				if (materials[i]->GetName() == "Mat_Custom") {
					float image[2] = { pMatCustomImage.x, pMatCustomImage.y };
					float zoom[2] = { pMatCustomZoom.x, pMatCustomZoom.y };

					ImGui::DragInt("Iterations", &pMatCustomIterations, 1, 0, 100);
					if (ImGui::DragFloat2("Image Center", image, 0.01f, -4.0f, 4.0f)) {
						pMatCustomImage = XMFLOAT2(image);
					}
					if (ImGui::DragFloat2("Zoom Center", zoom, 0.01f, -4.0f, 4.0f)) {
						pMatCustomZoom = XMFLOAT2(zoom);
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
			if (ImGui::TreeNode("", "(%06d) %s", i, LIGHT_TYPE_STRINGS[lights[i].Type])) {
				ImGui::Spacing();

				ImGui::ColorEdit3("Color", &lights[i].Color.x);
				if (ImGui::DragFloat("Intensity", &lights[i].Intensity, 0.1f, 0.0f, NULL, "%.1f")) {
					lights[i].Intensity = max(lights[i].Intensity, 0.0f);
				}
				
				ImGui::Spacing();
				ImGui::Text("Type:");
				if (ImGui::RadioButton("Directional", &lights[i].Type, LIGHT_TYPE_DIRECTIONAL) && i == 0) {
					BuildShadowMatrices();
				}
				ImGui::SameLine();
				if (ImGui::RadioButton("Point", &lights[i].Type, LIGHT_TYPE_POINT) && i == 0) {
					BuildShadowMatrices();
				}
				ImGui::SameLine();
				if (ImGui::RadioButton("Spot", &lights[i].Type, LIGHT_TYPE_SPOT) && i == 0) {
					BuildShadowMatrices();
				}

				ImGui::Spacing();
				if (lights[i].Type != LIGHT_TYPE_DIRECTIONAL) {
					// If first light's position is changed, update shadow matrices
					if (ImGui::DragFloat3("Position", &lights[i].Position.x, 0.01f) && i == 0) {
						BuildShadowMatrices();
					}
				}
				if (lights[i].Type != LIGHT_TYPE_POINT) {
					// If first light's direction is changed, update shadow matrices
					if (ImGui::DragFloat3("Direction", &lights[i].Direction.x, 0.01f) && i == 0) {
						BuildShadowMatrices();
					}
				}
				if (lights[i].Type != LIGHT_TYPE_DIRECTIONAL) {
					if (ImGui::DragFloat("Range", &lights[i].Range, 0.1f, 0.0f, NULL, "%.1f")) {
						lights[i].Range = max(lights[i].Range, 0.0f);
						// If first light's range is changed, update shadow matrices
						if (i == 0) {
							BuildShadowMatrices();
						}
					}
				}
				if (lights[i].Type == LIGHT_TYPE_SPOT) {
					if (ImGui::DragFloat("Spot Inner Angle", &lights[i].SpotInnerAngle, 0.01f, 0.0f, XM_PIDIV2, "%.2f")) {
						if (lights[i].SpotOuterAngle <= lights[i].SpotInnerAngle) {
							lights[i].SpotOuterAngle = lights[i].SpotInnerAngle + 0.01f;
							// If changing the first light's inner angle would change its outer angle, update shadow matrices
							if (i == 0) {
								BuildShadowMatrices();
							}
						}
					}
					ImGui::SetItemTooltip("In radians");
					if (ImGui::DragFloat("Spot Outer Angle", &lights[i].SpotOuterAngle, 0.01f, 0.01f, XM_PIDIV2, "%.2f")) {
						if (lights[i].SpotOuterAngle <= lights[i].SpotInnerAngle) {
							lights[i].SpotInnerAngle = lights[i].SpotOuterAngle - 0.01f;
						}
						// If first light's outer angle is changed, update shadow matrices
						if (i == 0) {
							BuildShadowMatrices();
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
			if (ImGui::TreeNode("", "(%06d) %s", i, cameras[i]->GetName())) {
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
						cameras[i]->SetFarClip(floor(cameraNear) + 1.0f);
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

	if (ImGui::CollapsingHeader("Skyboxes")) {					// Info about each skybox
		ImGui::Spacing();

		ImGui::PushID("SKYBOX");

		for (int i = 0; i < skyboxes.size(); i++) {
			// Each skybox gets its own Tree Node
			ImGui::PushID(i);
			ImGui::AlignTextToFramePadding();
			if (ImGui::RadioButton("", &pSkyboxCurrent, i)) {
				SetMaterialEnvironmentMaps(skyboxes[pSkyboxCurrent]);
			}
			ImGui::SetItemTooltip("Set as active skybox");

			ImGui::SameLine();
			if (ImGui::TreeNode("", "(%06d) %s", i, skyboxes[i]->GetName())) {
				ImGui::ColorEdit3("Ambient Light", &skyboxAmbientColors[i].x);

				ImGui::TreePop();
				ImGui::Spacing();
			}
			
			ImGui::PopID();
			ImGui::Spacing();
		}
		ImGui::PopID();

		ImGui::Spacing();
	}

	if (ImGui::CollapsingHeader("Particle Emitters")) {			// Info about each particle emitter
		ImGui::Spacing();

		ImGui::PushID("PARTICLEEMITTER");

		for (int i = 0; i < particleEmitters.size(); i++) {
			// Each skybox gets its own Tree Node
			ImGui::PushID(i);

			if (ImGui::TreeNode("", "(%06d) %s", i, particleEmitters[i]->GetName())) {
				ImGui::Text("Total Particles: %d", particleEmitters[i]->GetParticleCount());
				ImGui::Text("First Alive: %d", particleEmitters[i]->GetFirstAlive());
				ImGui::Text("First Dead: %d", particleEmitters[i]->GetFirstDead());
				ImGui::Text("Alive Count: %d", particleEmitters[i]->GetAliveCount());

				ImGui::TreePop();
				ImGui::Spacing();
			}

			ImGui::PopID();
			ImGui::Spacing();
		}
		ImGui::PopID();

		ImGui::Spacing();
	}

	if (ImGui::CollapsingHeader("Shadows")) {					// Info about the shadow map
		ImGui::Spacing();

		ImGui::Text("Fun tip: Try changing the first light in the scene to a Spot Light!");

		ImGui::Checkbox("Render shadows?", &pRenderShadows);
		ImGui::SetItemTooltip("Shadows are cast from the first light in the scene.");
		ImGui::Spacing();

		if (pRenderShadows) {
			if (ImGui::SliderInt("Shadow Map Resolution", &pShadowResolutionExponent, 1, 12)) {
				pShadowResolution = (int)pow(2, pShadowResolutionExponent);
				RebuildShadowMap();
			}
			ImGui::SetItemTooltip("Shadow map will be rendered at %d tx.", pShadowResolution);
			
			if (ImGui::SliderFloat("Shadow Area Width", &pShadowAreaWidth, 0.1f, 100.0f, "%.1f", ImGuiSliderFlags_Logarithmic)) {
				BuildShadowMatrices();
			}
			ImGui::SetItemTooltip("The width of the area in the world onto which shadows will be cast.");
			
			if (ImGui::DragFloat3("Shadow Area Center", &pShadowAreaCenter.x, 0.1f, NULL, NULL, "%.1f")) {
				BuildShadowMatrices();
			}
			ImGui::SetItemTooltip("The center of the area in the world onto which shadows will be cast.\nThe shadow map's far clip plane intersects this point.");

			if (ImGui::SliderFloat("Shadow Light Distance", &pShadowLightDistance, 0.2f, 1000.0f, "%.1f", ImGuiSliderFlags_Logarithmic)) {
				BuildShadowMatrices();
			}
			ImGui::SetItemTooltip("The distance from the area center to pull back the camera.");

			ImGui::Spacing();
			ImGui::Image((void*)shadowSRV.Get(), ImVec2(256, 256));
		}

		ImGui::Spacing();
	}

	if (ImGui::CollapsingHeader("Post-Processing")) {			// Info about post-process effects
		ImGui::Spacing();

		if (ImGui::TreeNode("Blur")) {								// Box Blur
			ImGui::Checkbox("Run Blur?", &ppBlurRun);
			ImGui::Spacing();

			if (ppBlurRun) {
				ImGui::SliderInt("Blur Radius", &ppBlurRadius, 0, 30);

				ImGui::Text("Initial Render:");
				ImGui::Spacing();
				ImGui::Image((void*)ppBlurSRV.Get(), ImVec2(256, 256));
			}

			ImGui::TreePop();
			ImGui::Spacing();
		}

		if (ImGui::TreeNode("Dither")) {							// Dithering (method by Lucas Pope)
			ImGui::Checkbox("Run Dither?", &ppDitherRun);
			ImGui::Spacing();

			if (ppDitherRun) {
				ImGui::SliderInt("Dither Pixel Size", &ppDitherPixelSize, 1, 32);
				ImGui::SliderFloat("Bias", &ppDitherBias, -1.0f, 1.0f, "%.2f");
				ImGui::SetItemTooltip("Adjusts overall brightness of the image");
				ImGui::ColorEdit3("Light Color", &ppDitherColorLight.x);
				ImGui::ColorEdit3("Dark Color", &ppDitherColorDark.x);


				ImGui::Spacing();

				switch (ppDitherMapTextureID) {
				case 0:
					ImGui::Text("Dither Map: Bayer Matrix (2px)");
					break;
				case 1:
					ImGui::Text("Dither Map: Bayer Matrix (4px)");
					break;
				case 2:
					ImGui::Text("Dither Map: Bayer Matrix (8px)");
					break;
				case 3:
					ImGui::Text("Dither Map: Blue Noise");
					break;
				case 4:
					ImGui::Text("Dither Map: Test Bayer Matrix (8px)");
					break;
				}

				ImGui::PushID("Dither Map Texture");
				ImGui::SliderInt("", &ppDitherMapTextureID, 0, (int)ppDitherMaps.size() - 1, "");
				ImGui::Image((void*)ppDitherMaps[ppDitherMapTextureID].Get(), ImVec2(256, 256));
				ImGui::PopID();

				ImGui::Text("Initial Render:");
				ImGui::Spacing();
				ImGui::Image((void*)ppDitherSRV.Get(), ImVec2(256, 256));
			}

			ImGui::TreePop();
			ImGui::Spacing();
		}

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

// --------------------------------------------------------
// Called by destructor, cleans up pointers used by helper functions
// --------------------------------------------------------
void Game::CleanupSimulationParameters() {
	if (igFrameGraphSamples != nullptr) {
		delete[] igFrameGraphSamples;
		igFrameGraphSamples = nullptr;
	}
}
