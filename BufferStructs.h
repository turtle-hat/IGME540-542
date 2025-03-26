#pragma once
#include <DirectXMath.h>
#include "ShaderConstants.hlsli"

// Overall scene data for raytracing
struct RaytracingSceneData
{
	DirectX::XMFLOAT4X4 inverseViewProjection;
	DirectX::XMFLOAT3 cameraPosition;
	float padding;
};

// Ensure this matches Raytracing shader define!
struct RaytracingEntityData
{
	DirectX::XMFLOAT4 albedo[MAX_INSTANCES_PER_BLAS];
	// Stored together in a float4 to ensure alignment with byte boundaries
	// x = roughness, y = metalness, z = refractive index, w = 1.0f;
	DirectX::XMFLOAT4 roughMetalRefract[MAX_INSTANCES_PER_BLAS];
};