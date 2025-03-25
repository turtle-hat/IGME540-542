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
	DirectX::XMFLOAT3 albedo[MAX_INSTANCES_PER_BLAS];
	float metalness[MAX_INSTANCES_PER_BLAS];

	float roughness[MAX_INSTANCES_PER_BLAS];
	float refractiveIndex[MAX_INSTANCES_PER_BLAS];
	DirectX::XMFLOAT2 padding[MAX_INSTANCES_PER_BLAS];
};