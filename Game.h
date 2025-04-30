#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <memory>
#include <vector>
#include <DirectXMath.h>

#include "Mesh.h"
#include "Material.h"
#include "Entity.h"
#include "Lights.h"
#include "Camera.h"
#include "Skybox.h"
#include "ParticleEmitter.h"
#include "Foliage.h"
#include "SimpleShader.h"

class Game
{
public:
	// Basic OOP setup
	Game() = default;
	~Game();
	Game(const Game&) = delete; // Remove copy constructor
	Game& operator=(const Game&) = delete; // Remove copy-assignment operator

	// Primary functions
	void Initialize();
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);
	void OnResize();

private:

	// Initialization helper methods - feel free to customize, combine, remove, etc.
	void LoadShaders();
	void CreateMaterials();
	void CreateGeometry();
	void CreateLights();
	void CreateCameras();
	void CreateSkyboxes();
	void CreateParticleEmitters();
	void InitializeSimulationParameters();
	void AddVertexShader(const wchar_t* _path, std::shared_ptr<SimpleVertexShader>& _shader);
	void AddPixelShader(const wchar_t* _path, std::shared_ptr<SimplePixelShader>& _shader);
	void AddTexture(const wchar_t* _path);
	void LoadTexture(const wchar_t* _path, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& _srv);
	void LoadTexture(const wchar_t* _path, std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>>& _srvVector);
	void AddMaterial(const char* _name, std::shared_ptr<SimpleVertexShader> _vertexShader, std::shared_ptr<SimplePixelShader> _pixelShader, DirectX::XMFLOAT4 _colorTint, float _roughness, bool _useGlobalEnvironmentMap);
	void AddMaterial(const char* _name, std::shared_ptr<SimpleVertexShader> _vertexShader, std::shared_ptr<SimplePixelShader> _pixelShader, DirectX::XMFLOAT4 _colorTint, float _roughness);
	void AddMaterial(const char* _name, std::shared_ptr<SimpleVertexShader> _vertexShader, std::shared_ptr<SimplePixelShader> _pixelShader, DirectX::XMFLOAT4 _colorTint);
	void AddMaterial(const char* _name, std::shared_ptr<SimpleVertexShader> _vertexShader, std::shared_ptr<SimplePixelShader> _pixelShader, float _roughness, bool _useGlobalEnvironmentMap);
	void AddMaterial(const char* _name, std::shared_ptr<SimpleVertexShader> _vertexShader, std::shared_ptr<SimplePixelShader> _pixelShader, float _roughness);
	void AddMaterial(const char* _name, std::shared_ptr<SimpleVertexShader> _vertexShader, std::shared_ptr<SimplePixelShader> _pixelShader);
	void AddPBRMaterial(const char* _name, std::shared_ptr<SimpleVertexShader> _vertexShader, std::shared_ptr<SimplePixelShader> _pixelShader, DirectX::XMFLOAT4 _colorTint, float _roughness, float _metalness);
	void AddPBRMaterial(const char* _name, std::shared_ptr<SimpleVertexShader> _vertexShader, std::shared_ptr<SimplePixelShader> _pixelShader, float _roughness, float _metalness);
	void AddEntity(const char* _name, unsigned int _meshIndex, unsigned int _materialIndex, DirectX::XMFLOAT3 _position);
	void AddLightDirectional(DirectX::XMFLOAT3 _direction, DirectX::XMFLOAT3 _color, float _intensity, bool _isActive);
	void AddLightPoint(DirectX::XMFLOAT3 _position, DirectX::XMFLOAT3 _color, float _intensity, float _range, bool _isActive);
	void AddLightSpot(DirectX::XMFLOAT3 _position, DirectX::XMFLOAT3 _direction, DirectX::XMFLOAT3 _color, float _intensity, float _range, float _innerAngle, float _outerAngle, bool _isActive);
	void AddCamera(const char* _name, DirectX::XMFLOAT3 _position, DirectX::XMFLOAT3 _rotation, float _aspect);
	void AddCamera(const char* _name, DirectX::XMFLOAT3 _position, DirectX::XMFLOAT3 _rotation, float _aspect, float _fov);
	void AddCamera(const char* _name, DirectX::XMFLOAT3 _position, DirectX::XMFLOAT3 _rotation, float _aspect, bool _isOrthographic);
	void AddCamera(const char* _name, DirectX::XMFLOAT3 _position, DirectX::XMFLOAT3 _rotation, float _aspect, bool _isOrthographic, float _orthoWidth);
	void AddSkybox(const char* _name, std::wstring _pathBase, DirectX::XMFLOAT3 _ambientColor);
	void SetGlobalSamplerState(D3D11_FILTER _filter, int _anisotropyLevel);
	void SetMaterialSamplerStates();
	void SetMaterialEnvironmentMaps(std::shared_ptr<Skybox> _skybox);
	void BuildShadowMap();
	void RebuildShadowMap();
	void BuildParticleResources();
	void BuildShadowMatrices();
	void BuildPostProcesses();
	void RebuildPostProcesses();
	void RebuildPostProcess(Microsoft::WRL::ComPtr<ID3D11RenderTargetView>& _ppRTV, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& _ppSRV);
	void ImGuiUpdate(float _deltaTime);

	// Update helper methods
	void ImGuiBuild();

	// Draw helper methods

	// Destructor helper methods
	void CleanupSimulationParameters();




	// SIMULATION PARAMETERS
	// All start with a "p" for "parameter"
	
	// Whether parameters have been initialized
	bool isInitialized;
	// The background color used by Direct3D
	float pBackgroundColor[4];
	// How fast to rotate the objects
	float pObjectRotationSpeed;

	// Filtering mode
	D3D11_FILTER pSamplerFilter;
	// Names for each filtering mode
	const char* SAMPLER_FILTER_STRINGS[6] = { "Point", "Linear Magnification", "Linear Minification", "Bilinear", "Trilinear", "Anisotropic" };
	// Corresponding filtering modes
	const D3D11_FILTER SAMPLER_FILTERS[6] = {
		D3D11_FILTER_MIN_MAG_MIP_POINT,
		D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT,
		D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT,
		D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT,
		D3D11_FILTER_MIN_MAG_MIP_LINEAR,
		D3D11_FILTER_ANISOTROPIC
	};
	// Selected array item
	int pSelectedSamplerFilter;
	// Level of anisotropy is 2 to this power
	int pAnisotropyPower;

	// How many iterations the custom material should go through
	int pMatCustomIterations;
	// Where to center the image and the zoom-in on it
	DirectX::XMFLOAT2 pMatCustomImage;
	DirectX::XMFLOAT2 pMatCustomZoom;

	const char* LIGHT_TYPE_STRINGS[3] = { "Directional", "Point", "Spot" };

	// Note the usage of ComPtr below
	//  - This is a smart pointer for objects that abide by the
	//     Component Object Model, which DirectX objects do
	//  - More info here: https://github.com/Microsoft/DirectXTK/wiki/ComPtr

	// MATERIAL SHADERS
	std::shared_ptr<SimpleVertexShader> vsDiffuseSpecular;
	std::shared_ptr<SimpleVertexShader> vsDiffuseNormal;
	std::shared_ptr<SimpleVertexShader> vsPBR;

	std::shared_ptr<SimplePixelShader> psDiffuseSpecular;
	std::shared_ptr<SimplePixelShader> psDiffuseNormal;
	std::shared_ptr<SimplePixelShader> psPBR;
	std::shared_ptr<SimplePixelShader> psNormals;
	std::shared_ptr<SimplePixelShader> psUVs;
	std::shared_ptr<SimplePixelShader> psCustom;

	// MESHES
	std::vector<std::shared_ptr<Mesh>> meshes;
	
	// TEXTURES
	std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> textures;
	// Pointer to the general sampler state
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerState;

	// MATERIALS
	std::vector<std::shared_ptr<Material>> materials;

	// ENTITIES
	std::vector<std::shared_ptr<Entity>> entities;

	// LIGHTS
	std::vector<Light> lights;

	// CAMERAS
	std::vector<std::shared_ptr<Camera>> cameras;
	// Index of the current camera
	int pCameraCurrent;

	// SKYBOXES
	std::vector<std::shared_ptr<Skybox>> skyboxes;
	// Shaders
	std::shared_ptr<SimpleVertexShader> vsSkybox;
	std::shared_ptr<SimplePixelShader> psSkybox;

	// Ambient light colors for each skybox
	std::vector<DirectX::XMFLOAT3> skyboxAmbientColors;
	int pSkyboxCurrent;

	// PARTICLES
	std::vector<std::shared_ptr<ParticleEmitter>> particleEmitters;
	// Shaders
	std::shared_ptr<SimpleVertexShader> vsParticle;
	std::shared_ptr<SimplePixelShader> psParticle;
	// Pipeline State
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> particleDepthStencilState;
	Microsoft::WRL::ComPtr<ID3D11BlendState> particleBlendState;

	// FOLIAGE
	std::vector<std::shared_ptr<Foliage>> foliages;
	// Shaders
	std::shared_ptr<SimpleVertexShader> vsFoliageLeaf;
	std::shared_ptr<SimplePixelShader> psFoliageLeaf;



	// SHADOWS
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> shadowDSV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shadowSRV;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> shadowRasterizer;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> shadowSampler;
	DirectX::XMFLOAT4X4 shadowLightViewMatrix;
	DirectX::XMFLOAT4X4 shadowLightProjectionMatrix;
	// Vertex Shader
	std::shared_ptr<SimpleVertexShader> vsShadowMap;
	// Parameters
	// Whether to render shadows
	bool pRenderShadows;
	// Shadow map's dimensions
	int pShadowResolution;
	// Shadow map's dimensions are 2^this value
	int pShadowResolutionExponent;
	// Area the shadow map covers in the world
	float pShadowAreaWidth;
	// Center of the area the shadow map covers in the world (far clip plane ends at this point)
	DirectX::XMFLOAT3 pShadowAreaCenter;
	// Distance from the shadow map's center to pull the camera back when rendering the shadow map
	float pShadowLightDistance;

	// POST-PROCESSING
	Microsoft::WRL::ComPtr<ID3D11SamplerState> ppSampler;
	std::shared_ptr<SimpleVertexShader> ppVS;
	DirectX::XMFLOAT2 ppPixelSize;

	// POST-PROCESS 1: Blur
	std::shared_ptr<SimplePixelShader> ppBlurPS;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> ppBlurRTV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ppBlurSRV;
	// Whether to run the blur effect
	bool ppBlurRun;
	// The width/height of the box blur
	int ppBlurRadius;

	// POST-PROCESS 1: Dither
	std::shared_ptr<SimplePixelShader> ppDitherPS;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> ppDitherRTV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ppDitherSRV;
	// Whether to run the dither effect
	bool ppDitherRun;
	// Which dither map texture to use
	int ppDitherMapTextureID;
	// The number of screen pixels each output pixel should occupy
	int ppDitherPixelSize;
	// Biases the dither calculations to make the image overall lighter or darker
	float ppDitherBias;
	// Colors to use for light and dark pixels
	DirectX::XMFLOAT3 ppDitherColorLight;
	DirectX::XMFLOAT3 ppDitherColorDark;
	// List of possible dither textures
	std::vector<Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> ppDitherMaps;
	// Normal texture sampler for dither
	Microsoft::WRL::ComPtr<ID3D11SamplerState> ppDitherMapSampler;







	// IMGUI-SPECIFIC VARIABLES
	// All start with a "ig" for "ImGui"

	// Whether to show the ImGui demo
	bool igShowDemo;

	// Framerate Graph Variables

	// How many framerate samples are recorded for the graph
	const int IG_FRAME_GRAPH_TOTAL_SAMPLES = 1000;
	// Array of recorded framerate samples
	float* igFrameGraphSamples;
	// How many framerate samples are displayed on the graph
	int igFrameGraphSampleCount;
	// The rate at which new framerate samples should be added to the graph
	float igFrameGraphSampleRate;
	// The timestamp at which to add new framerate samples
	double igFrameGraphNextSampleTime;
	// The index of pFrameTimes to write the next sample to
	int igFrameGraphSampleOffset;
	// The highest recorded framerate; sets the scale of the framerate graph
	float igFrameGraphHighest;
	// Whether to continue sampling framerate for the graph
	bool igFrameGraphDoAnimate;
};
