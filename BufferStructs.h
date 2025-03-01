#pragma once
#include <DirectXMath.h>
#include "ShaderConstants.hlsli"

// Overall scene data for raytracing
struct RaytracingSceneData
{
	DirectX::XMFLOAT4X4 inverseViewProjection;
	DirectX::XMFLOAT3 cameraPosition;
	float pad;
};

// Ensure this matches Raytracing shader define!
struct RaytracingEntityData
{
	DirectX::XMFLOAT4 color[MAX_INSTANCES_PER_BLAS];
};