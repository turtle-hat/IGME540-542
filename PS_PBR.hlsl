#include "ShaderStructs.hlsli"
#include "ShaderLighting.hlsli"
#include "ShaderNormals.hlsli"

// Data from our primary constant buffer
cbuffer PrimaryBuffer : register(b0)
{
	float3 cameraPosition;
	int lightCount;

	Light lights[LIGHT_COUNT];

	float2 uvScale;
	float2 uvOffset;

	float4 colorTint;

	float roughness;
	float metalness;
	float2 padding;
}

Texture2D MapAlbedoMetalness : register(t0); // "t" registers for textures
Texture2D MapNormalRoughness : register(t1);

SamplerState BasicSampler : register(s0); // "s" registers for samplers

float4 main(VertexToPixel_Normal input) : SV_TARGET
{
	// Renormalize the normal and tangent
	input.normal = normalize(input.normal);
	input.tangent = normalize(input.tangent);

	// Sample the textures at this pixel
	float4 sampleAM = MapAlbedoMetalness.Sample(BasicSampler, input.uv * uvScale + uvOffset);
	// Pull info out of the diffuse/specular texture
	float3 sampleAlbedo = pow(sampleAM.rgb, 2.2f); // Gamma uncorrected so it gets the expected value after end correction
	float finalMetalness = sampleAM.a * metalness;
	float3 surfaceColor = sampleAlbedo * colorTint.rgb;

	// Unpack and normalize the normal map
	float4 sampleNR = SampleUnpacked(MapNormalRoughness, BasicSampler, input.uv * uvScale + uvOffset);
	// Calculate normal from map
	float3 finalNormal = NormalFromMap(input.normal, input.tangent, sampleNR.rgb);
	float finalRoughness = sampleNR.a * roughness;

	// Color of surface with all lighting calculated
	float3 litColor = CalculateLightingLambertCookTorrance(
		lights,
		finalNormal,
		surfaceColor,
		finalRoughness,
		finalMetalness,
		input.worldPosition,
		cameraPosition
	);

	// Return the result of our lighting equations
	
	return float4(
		pow(litColor, 1.0f / 2.2f),
		1.0f
	);
}
