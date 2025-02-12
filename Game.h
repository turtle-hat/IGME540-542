#pragma once

#include <d3d12.h>
#include <wrl/client.h>
#include <vector>

#include "Camera.h"
#include "Entity.h"
#include "Lights.h"
#include "Material.h"
#include "Mesh.h"

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
	void InitializeParameters();
	void CreateRootSigAndPipelineState();

	// Helper methods for creating specific resources
	void CreateMaterials();
	D3D12_CPU_DESCRIPTOR_HANDLE LoadTexture(const wchar_t* _path);
	std::shared_ptr<Material> AddMaterial(const char* _name, Microsoft::WRL::ComPtr<ID3D12PipelineState> _pso);
	std::shared_ptr<Material> AddMaterial(const char* _name, Microsoft::WRL::ComPtr<ID3D12PipelineState> _pso, DirectX::XMFLOAT3 _colorTint);
	void CreateGeometry();
	std::shared_ptr<Entity> AddEntity(const char* _name, unsigned int _meshIndex, unsigned int _materialIndex, DirectX::XMFLOAT3 _position);
	void CreateCameras();
	std::shared_ptr<Camera> AddCamera(const char* _name, DirectX::XMFLOAT3 _position, DirectX::XMFLOAT3 _rotation, float _aspect);
	std::shared_ptr<Camera> AddCamera(const char* _name, DirectX::XMFLOAT3 _position, DirectX::XMFLOAT3 _rotation, float _aspect, float _fov);
	std::shared_ptr<Camera> AddCamera(const char* _name, DirectX::XMFLOAT3 _position, DirectX::XMFLOAT3 _rotation, float _aspect, bool _isOrthographic);
	std::shared_ptr<Camera> AddCamera(const char* _name, DirectX::XMFLOAT3 _position, DirectX::XMFLOAT3 _rotation, float _aspect, bool _isOrthographic, float _orthoWidth);
	void CreateLights();
	Light AddLightDirectional(DirectX::XMFLOAT3 _direction, DirectX::XMFLOAT3 _color, float _intensity, bool _isActive);
	Light AddLightPoint(DirectX::XMFLOAT3 _position, DirectX::XMFLOAT3 _color, float _intensity, float _range, bool _isActive);
	Light AddLightSpot(DirectX::XMFLOAT3 _position, DirectX::XMFLOAT3 _direction, DirectX::XMFLOAT3 _color, float _intensity, float _range, float _innerAngle, float _outerAngle, bool _isActive);
	void ImGuiInitialize();
	void ImGuiUpdate(float _deltaTime);
	void ImGuiBuildInterface();

	// Note the usage of ComPtr below
	//  - This is a smart pointer for objects that abide by the
	//     Component Object Model, which DirectX objects do
	//  - More info here: https://github.com/Microsoft/DirectXTK/wiki/ComPtr

	// Pipeline
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState;
	
	// SIMULATION STATE
	bool isInitialized;

	// CAMERAS
	// The index of the camera currently in use
	unsigned int cameraCurrent;
	std::vector<std::shared_ptr<Camera>> cameras;

	// LIGHTS
	std::vector<Light> lights;

	// MESHES
	std::vector<std::shared_ptr<Mesh>> meshes;

	// TEXTURES
	std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> textures;
	
	// MATERIALS
	std::vector<std::shared_ptr<Material>> materials;

	// ENTITIES
	std::vector<std::shared_ptr<Entity>> entities;

	// IMGUI
	bool igShowDemo;

	// Other graphics data
	D3D12_VIEWPORT viewport{};
	D3D12_RECT scissorRect{};
};

