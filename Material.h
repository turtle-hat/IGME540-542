#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include <DirectXMath.h>

class Material
{
public:
	// Constructor
	Material(Microsoft::WRL::ComPtr<ID3D12PipelineState> _pso, DirectX::XMFLOAT3 _colorTint, DirectX::XMFLOAT2 _uvScale, DirectX::XMFLOAT2 _uvOffset);
	
	// Getters
	D3D12_GPU_DESCRIPTOR_HANDLE GetFinalGPUHandleForSRVs();
	DirectX::XMFLOAT3 GetColorTint();
	DirectX::XMFLOAT2 GetUVScale();
	DirectX::XMFLOAT2 GetUVOffset();

	// Setters
	void SetColorTint(DirectX::XMFLOAT3 _colorTint);
	void SetUVScale(DirectX::XMFLOAT2 _uvScale);
	void SetUVOffset(DirectX::XMFLOAT2 _uvOffset);

	// Material Management
	void AddTexture(D3D12_CPU_DESCRIPTOR_HANDLE _srv, int _slot);
	void FinalizeMaterial();

private:
	// Material Properties
	DirectX::XMFLOAT3 colorTint;
	DirectX::XMFLOAT2 uvScale;
	DirectX::XMFLOAT2 uvOffset;

	// Texture Data
	bool finalized;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pso;
	D3D12_CPU_DESCRIPTOR_HANDLE textureSRVsBySlot[128];
	unsigned int highestTextureSlotInUse;
	D3D12_GPU_DESCRIPTOR_HANDLE finalGPUHandleForSRVs;
};

