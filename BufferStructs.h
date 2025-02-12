#pragma once
#include <DirectXMath.h>
#include "ShaderConstants.hlsli"

struct VertexShaderExternalData
{
	DirectX::XMFLOAT4X4 world;
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 projection;
	DirectX::XMFLOAT4X4 worldIT;
};

struct PixelShaderExternalData
{
	DirectX::XMFLOAT3 cameraPosition;
	int lightCount;

	Light lights[MAX_LIGHTS];

	DirectX::XMFLOAT2 uvScale;
	DirectX::XMFLOAT2 uvOffset;

	DirectX::XMFLOAT3 colorTint;
	float roughness;

	float metalness;
	DirectX::XMFLOAT3 padding;
};