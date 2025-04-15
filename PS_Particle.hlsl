#include "ShaderStructs.hlsli"

Texture2D ParticleTexture : register(t0);

SamplerState BasicSampler : register(s0); // "s" registers for samplers

float4 main(VertexToPixel_Particle input) : SV_TARGET
{
    // Sample texture
    float4 sampleTexture = ParticleTexture.Sample(BasicSampler, input.uv);
    
    // Tint texture sample by RGB of color tint,
    // then multiply all channels by the color tint alpha channel for opacity with additive blending
    return sampleTexture * input.colorTint;
}