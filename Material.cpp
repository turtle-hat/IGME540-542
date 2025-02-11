#include "Material.h"
#include "Graphics.h"

Material::Material(const char* _name, Microsoft::WRL::ComPtr<ID3D12PipelineState> _pso, DirectX::XMFLOAT3 _colorTint, DirectX::XMFLOAT2 _uvScale, DirectX::XMFLOAT2 _uvOffset)
{
	name = _name;
	pso = _pso;
	colorTint = _colorTint;
	uvScale = _uvScale;
	uvOffset = _uvOffset;
	roughness = 1.0f;
	metalness = 1.0f;

	finalized = false;
	highestTextureSlotInUse = 0;
	finalGPUHandleForSRVs = (D3D12_GPU_DESCRIPTOR_HANDLE)0;
	textureSRVsBySlot[0] = (D3D12_CPU_DESCRIPTOR_HANDLE)0;
}

D3D12_GPU_DESCRIPTOR_HANDLE Material::GetFinalGPUHandleForSRVs()
{
	return finalGPUHandleForSRVs;
}

Microsoft::WRL::ComPtr<ID3D12PipelineState> Material::GetPipelineState()
{
	return pso;
}

DirectX::XMFLOAT3 Material::GetColorTint()
{
	return colorTint;
}

DirectX::XMFLOAT2 Material::GetUVScale()
{
	return uvScale;
}

DirectX::XMFLOAT2 Material::GetUVOffset()
{
	return uvOffset;
}

const char* Material::GetName()
{
	return name;
}

float Material::GetRoughness()
{
	return roughness;
}

float Material::GetMetalness()
{
	return metalness;
}

void Material::SetPipelineState(Microsoft::WRL::ComPtr<ID3D12PipelineState> _pso)
{
	pso = _pso;
}

void Material::SetColorTint(DirectX::XMFLOAT3 _colorTint)
{
	colorTint = _colorTint;
}

void Material::SetUVScale(DirectX::XMFLOAT2 _uvScale)
{
	uvScale = _uvScale;
}

void Material::SetUVOffset(DirectX::XMFLOAT2 _uvOffset)
{
	uvOffset = _uvOffset;
}

void Material::SetRoughness(float _roughness)
{
	roughness = _roughness;
}

void Material::SetMetalness(float _metalness)
{
	metalness = _metalness;
}

// Adds a texture SRV to the material to be referred to by a specific texture register
void Material::AddTexture(D3D12_CPU_DESCRIPTOR_HANDLE _srv, unsigned int _slot)
{
	// Don't continue if already finalized
	if (finalized) return;

	textureSRVsBySlot[_slot] = _srv;
	// If this is the highest texture slot the material has had assigned, set the marker accordingly
	highestTextureSlotInUse = max(highestTextureSlotInUse, _slot);
}

void Material::FinalizeMaterial()
{
	// Don't continue if already finalized
	if (finalized) return;

	// Copy each texture SRV to the GPU
	for (unsigned int i = 0; i <= highestTextureSlotInUse; i++) {
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = Graphics::CopySRVsToDescriptorHeapAndGetGPUDescriptorHandle(textureSRVsBySlot[i], 1);
		
		// Save GPU handle of first texture
		if (i == 0) {
			finalGPUHandleForSRVs = gpuHandle;
		}
	}

	finalized = true;
}
