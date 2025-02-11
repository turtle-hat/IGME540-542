#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include <DirectXMath.h>

class Material
{
public:
	// Constructor
	Material(const char* _name, Microsoft::WRL::ComPtr<ID3D12PipelineState> _pso, DirectX::XMFLOAT3 _colorTint, DirectX::XMFLOAT2 _uvScale, DirectX::XMFLOAT2 _uvOffset);
	
	// Getters
	D3D12_GPU_DESCRIPTOR_HANDLE GetFinalGPUHandleForSRVs();
	Microsoft::WRL::ComPtr<ID3D12PipelineState> GetPipelineState();
	DirectX::XMFLOAT3 GetColorTint();
	DirectX::XMFLOAT2 GetUVScale();
	DirectX::XMFLOAT2 GetUVOffset();
	const char* GetName();
	float GetRoughness();
	float GetMetalness();

	// Setters
	void SetPipelineState(Microsoft::WRL::ComPtr<ID3D12PipelineState> _pso);
	void SetColorTint(DirectX::XMFLOAT3 _colorTint);
	void SetUVScale(DirectX::XMFLOAT2 _uvScale);
	void SetUVOffset(DirectX::XMFLOAT2 _uvOffset);
	void SetRoughness(float _roughness);
	void SetMetalness(float _metalness);

	// Material Management
	void AddTexture(D3D12_CPU_DESCRIPTOR_HANDLE _srv, unsigned int _slot);
	void FinalizeMaterial();

private:
	// Material Properties
	DirectX::XMFLOAT3 colorTint;
	DirectX::XMFLOAT2 uvScale;
	DirectX::XMFLOAT2 uvOffset;
	float roughness;
	float metalness;

	// Texture Data
	bool finalized;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pso;
	D3D12_CPU_DESCRIPTOR_HANDLE textureSRVsBySlot[128]{};
	unsigned int highestTextureSlotInUse;
	D3D12_GPU_DESCRIPTOR_HANDLE finalGPUHandleForSRVs;

	// Name for UI
	const char* name;
};

