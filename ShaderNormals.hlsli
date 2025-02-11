#ifndef __GGP_SHADER_NORMALS__
#define __GGP_SHADER_NORMALS__

#include "ShaderStructs.hlsli"

// Samples and unpacks the first three components of a normal map
float4 SampleUnpacked(Texture2D _mapNormal, SamplerState _sampler, float2 _uv)
{
    float4 sample = _mapNormal.Sample(_sampler, _uv);
    
    // Unpack only the first three components
    return float4(
        normalize(sample.rgb * 2 - 1),
        sample.a
    );
}

// Calculates a pixel's final normal given the normalized surface normal, the normalized tangent, and the unpacked value from the normal map
float3 NormalFromMap(float3 _surfaceNormal, float3 _surfaceTangent, float3 _unpackedNormal)
{
    // Gram-Schmidt orthonormalize the surface tangent
    _surfaceTangent = normalize(_surfaceTangent - _surfaceNormal * dot(_surfaceTangent, _surfaceNormal));
    // Calculate the bitangent
    float3 surfaceBitangent = cross(_surfaceTangent, _surfaceNormal);
    // Calculate the TBN matrix
    float3x3 TBN = float3x3(_surfaceTangent, surfaceBitangent, _surfaceNormal);
    // Return the rotated map normal vector
    return mul(_unpackedNormal, TBN);
}

#endif