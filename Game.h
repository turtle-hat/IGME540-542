#pragma once

#include <d3d12.h>
#include <wrl/client.h>
#include <vector>

#include "Camera.h"
#include "Entity.h"
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
	void CreateGeometry();
	void AddEntity(const char* _name, unsigned int _meshIndex, DirectX::XMFLOAT3 _position);
	void CreateCameras();
	void AddCamera(const char* _name, DirectX::XMFLOAT3 _position, DirectX::XMFLOAT3 _rotation, float _aspect);
	void AddCamera(const char* _name, DirectX::XMFLOAT3 _position, DirectX::XMFLOAT3 _rotation, float _aspect, float _fov);
	void AddCamera(const char* _name, DirectX::XMFLOAT3 _position, DirectX::XMFLOAT3 _rotation, float _aspect, bool _isOrthographic);
	void AddCamera(const char* _name, DirectX::XMFLOAT3 _position, DirectX::XMFLOAT3 _rotation, float _aspect, bool _isOrthographic, float _orthoWidth);

	// Note the usage of ComPtr below
	//  - This is a smart pointer for objects that abide by the
	//     Component Object Model, which DirectX objects do
	//  - More info here: https://github.com/Microsoft/DirectXTK/wiki/ComPtr

	// Pipeline
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState;
	
	// CAMERAS
	// The index of the camera currently in use
	unsigned int cameraCurrent;
	std::vector<std::shared_ptr<Camera>> cameras;

	// MESHES
	std::vector<std::shared_ptr<Mesh>> meshes;

	// ENTITIES
	std::vector<std::shared_ptr<Entity>> entities;

	// Other graphics data
	D3D12_VIEWPORT viewport{};
	D3D12_RECT scissorRect{};
};

