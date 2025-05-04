#include "ShaderStructs.hlsli"
#include "ShaderLighting.hlsli"
#include "ShaderNormals.hlsli"

// Data from our primary constant buffer
cbuffer PrimaryBuffer : register(b0)
{
	float4 colorTint;
	float roughness;
	float3 cameraPosition;

	float2 uvPosition;
	float2 uvScale;

	Light lights[LIGHT_COUNT];

	float metalness;
	int shadowsActive;
    float2 padding;
}

Texture2D MapAlbedoMetalness : register(t0); // "t" registers for textures
Texture2D MapNormalRoughness : register(t1);
Texture2D MapShadow          : register(t2);

SamplerState BasicSampler : register(s0); // "s" registers for samplers
SamplerComparisonState ShadowSampler : register(s1);

float4 main(VertexToPixel_Shadow input) : SV_TARGET
{
	// Renormalize the normal and tangent
	input.normal = normalize(input.normal);
	input.tangent = normalize(input.tangent);

	// Sample the textures at this pixel
	float4 sampleAM = MapAlbedoMetalness.Sample(BasicSampler, input.uv * uvScale + uvPosition);
	// Pull info out of the diffuse/specular texture
	float3 sampleAlbedo = pow(sampleAM.rgb, 2.2f); // Gamma uncorrected so it gets the expected value after end correction
	float finalMetalness = sampleAM.a * metalness;
	float3 surfaceColor = sampleAlbedo * colorTint.rgb;

	// Unpack and normalize the normal map
	float4 sampleNR = SampleUnpacked(MapNormalRoughness, BasicSampler, input.uv * uvScale + uvPosition);
	// Calculate normal from map
	float3 finalNormal = NormalFromMap(input.normal, input.tangent, sampleNR.rgb);
	float finalRoughness = sampleNR.a * roughness;



	// Shadow calculations

	// Perspective divide
	input.shadowPosition /= input.shadowPosition.w;

	// Convert NDCs to UV coordinates for sampling
	float2 shadowUV = input.shadowPosition.xy * 0.5f + 0.5f;
	shadowUV.y = 1 - shadowUV.y;

	// Get distances from light to the closest surface and to this pixel
	float lightToPixel = input.shadowPosition.z;
	float shadowAmount = MapShadow.SampleCmpLevelZero(ShadowSampler, shadowUV, lightToPixel).r;



	// Testing overrides
	//surfaceColor = colorTint.rgb;
	//finalRoughness = roughness;
	//finalMetalness = metalness;
	//finalNormal = input.normal;

	// Color of surface with all lighting calculated
	float3 litColor = CalculateLightingLambertCookTorrance(
		lights,
		finalNormal,
		surfaceColor,
		finalRoughness,
		finalMetalness,
		input.worldPosition,
		cameraPosition,
		shadowAmount
	);

	// Return the result of our lighting equations
	
	return float4(
		pow(litColor, 1.0f / 2.2f),
		1.0f
	);
}
