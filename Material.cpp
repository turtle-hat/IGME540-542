#include "Material.h"

Material::Material(Microsoft::WRL::ComPtr<ID3D12PipelineState> _pso, DirectX::XMFLOAT3 _colorTint, DirectX::XMFLOAT2 _uvScale, DirectX::XMFLOAT2 _uvOffset)
{
	pso = _pso;
	colorTint = _colorTint;
	uvScale = _uvScale;
	uvOffset = _uvOffset;

	finalized = false;
	highestTextureSlotInUse = 0;
}

D3D12_GPU_DESCRIPTOR_HANDLE Material::GetFinalGPUHandleForSRVs()
{
	return finalGPUHandleForSRVs;
}

DirectX::XMFLOAT3 Material::GetColorTint()
{
	return DirectX::XMFLOAT3();
}

DirectX::XMFLOAT2 Material::GetUVScale()
{
	return DirectX::XMFLOAT2();
}

DirectX::XMFLOAT2 Material::GetUVOffset()
{
	return DirectX::XMFLOAT2();
}

void Material::SetColorTint(DirectX::XMFLOAT3 _colorTint)
{
}

void Material::SetUVScale(DirectX::XMFLOAT2 _uvScale)
{
}

void Material::SetUVOffset(DirectX::XMFLOAT2 _uvOffset)
{
}

void Material::AddTexture(D3D12_CPU_DESCRIPTOR_HANDLE _srv, int _slot)
{
}

void Material::FinalizeMaterial()
{
}
