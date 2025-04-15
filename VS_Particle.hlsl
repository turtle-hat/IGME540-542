#include "ShaderStructs.hlsli"

cbuffer PrimaryBuffer : register(b0)
{
    float4x4 tfView;
    float4x4 tfProjection;
    
    float lifetime;
    float acceleration;
    float totalTime;
    float padding;
};

StructuredBuffer<Particle> ParticleData : register(t0);

// Takes a vertex ID
VertexToPixel_Particle main(uint id : SV_VertexID)
{
    // Set up output struct
    VertexToPixel_Particle output;
    
    // Find particle identifying information
    uint particleIndex  = id / 4;   // Every group of 4 verts are ONE particle! (int division)
    uint cornerIndex    = id % 4;   // 0, 1, 2, 3 = which corner of the particle’s "quad"
    Particle p = ParticleData.Load(particleIndex); // Each vertex gets associated particle!
    
    // Find age-related values
    float age = totalTime - p.EmitTime;
    float agePercent = age / lifetime;
    
    // Simulate position mathematically
    // x_f = 0.5at^2 + vt + x_i
    float3 pos = 0.5f * acceleration * totalTime * totalTime + p.StartVelocity * totalTime + p.StartPosition;

    
    // Offset positions for the 4 corners of a quad - we'll only use one for each
    // vertex, but which one depends on the cornerID
    float2 offsets[4];
    offsets[0] = float2(-1.0f, +1.0f); // TL
    offsets[1] = float2(+1.0f, +1.0f); // TR
    offsets[2] = float2(+1.0f, -1.0f); // BR
    offsets[3] = float2(-1.0f, -1.0f); // BL
    
    // Billboarding
    // Offset the position based on the camera's right and up vectors
    pos += float3(tfView._11, tfView._12, tfView._13) * offsets[cornerIndex].x; // RIGHT
    pos += float3(tfView._21, tfView._22, tfView._23) * offsets[cornerIndex].y; // UP
    
    // Finally, multiply view and projection matrices to find output position
    matrix viewProjection = mul(tfProjection, tfView);
    output.screenPosition = mul(viewProjection, float4(pos, 1.0f));
    
    // Find UV Coordinates for this vertex
    float2 uvs[4];
    uvs[0] = float2(0, 0); // TL
    uvs[1] = float2(1, 0); // TR
    uvs[2] = float2(1, 1); // BR
    uvs[3] = float2(0, 1); // BL
    output.uv = uvs[cornerIndex];

    // Interpolate color
    output.color = lerp(p.StartColor, p.FinalColor, agePercent);
    
    return output;
}