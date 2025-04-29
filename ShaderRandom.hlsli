#ifndef __GGP_SHADER_RANDOM__
#define __GGP_SHADER_RANDOM__

// Basic pseudorandom number generator from 2 values
float rand(float2 _uv)
{
    return frac(sin(dot(_uv, float2(12.9898, 78.233))) * 43758.5453);
}

// Two pseudorandom floats based on 2 values 
float2 rand2(float2 _uv)
{
    return float2(
        rand(_uv),
        rand(_uv.yx)
    );
}

// Generates a random unit vector in a sphere (_uv should be two random values)
float3 RandomVectorSphere(float2 _uv)
{
    float a = _uv.x * 2.0f - 1.0f;
    float b = sqrt(1.0f - a * a);
    float phi = 2.0f * _uv.y * PI;
    
    return float3(
        b * cos(phi),
        b * sin(phi),
        a
    );
}

// Generates a random unit vector in a hemisphere, (_uv should be two random values, _unitNormal is the hemisphere plane's normal)
float3 RandomVectorHemisphere(float2 _uv, float3 _unitNormal)
{
    return RandomVectorSphere(_uv) + _unitNormal;
}

#endif