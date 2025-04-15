#include "ShaderStructs.hlsli"

// Data from our primary constant buffer
cbuffer PrimaryBuffer : register(b0)
{
	
}

Texture2D ParticleTexture : register(t0);

SamplerState BasicSampler : register(s0); // "s" registers for samplers

float4 main(VertexToPixel_Particle input) : SV_TARGET
{
    // Sample texture
    float4 sampleTexture = ParticleTexture.Sample(BasicSampler, input.uv);
	
    // Tint texture sample by RGB of color tint,
    // then multiply all channels by the color tint alpha channel for opacity with additive blending
    return (sampleTexture * float4(input.colorTint.rgb, 1.0f)) * input.colorTint.a;
}