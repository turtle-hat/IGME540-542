#ifndef __GGP_SHADER_STRUCTS__
#define __GGP_SHADER_STRUCTS__

// Struct representing a single vertex worth of data
// - This should match the vertex definition in our C++ code
// - By "match", I mean the size, order and number of members
// - The name of the struct itself is unimportant, but should be descriptive
// - Each variable must have a semantic, which defines its usage
struct VertexShaderInput
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
    float3 localPosition	: POSITION; // XYZ position
    float3 normal			: NORMAL; // Normal vector
    float3 tangent			: TANGENT; // Tangent vector
    float2 uv				: TEXCOORD; // UV coordinate
};

// Struct representing the data we're sending down the pipeline
// - Should match our pixel shader's input (hence the name: Vertex to Pixel)
// - At a minimum, we need a piece of data defined tagged as SV_POSITION
// - The name of the struct itself is unimportant, but should be descriptive
// - Each variable must have a semantic, which defines its usage

// For shaders that take basic information
struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
    float4 screenPosition	: SV_POSITION; // XYZW position (System Value Position)
    float3 normal           : NORMAL; // Normal vector
    float2 uv				: TEXCOORD; // UV coordinate
    float3 worldPosition	: POSITION; // World position of the pixel
};

// For shaders that take normal maps
struct VertexToPixel_Normal
{
    float4 screenPosition   : SV_POSITION; // XYZW position (System Value Position)
    float3 normal           : NORMAL; // Normal vector
    float3 tangent          : TANGENT; // Tangent vector
    float2 uv               : TEXCOORD; // UV coordinate
    float3 worldPosition    : POSITION; // World position of the pixel
};

// For shaders that take normal maps and shadow maps
struct VertexToPixel_Shadow
{
    float4 screenPosition : SV_POSITION; // XYZW position (System Value Position)
    float3 normal         : NORMAL; // Normal vector
    float3 tangent        : TANGENT; // Tangent vector
    float2 uv             : TEXCOORD; // UV coordinate
    float3 worldPosition  : POSITION; // World position of the pixel
    float4 shadowPosition : SHADOW_POSITION; // UV coordinate of the pixel on the shadow map
};

// For skybox shader
struct VertexToPixel_Sky
{
    float4 screenPosition   : SV_POSITION; // XYZW position (System Value Position)
    float3 sampleDirection  : DIRECTION; // Direction to sample in
};

// For post-process shaders
struct VertexToPixel_PostProcess
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
    float4 position : SV_POSITION; // XYZW position (System Value Position)
    float2 uv       : TEXCOORD0; // UV coordinate
};


// For particle shaders
struct VertexToPixel_Particle
{
    float4 screenPosition   : SV_POSITION; // XYZW position (System Value Position)
    float2 uv               : TEXCOORD0; // UV coordinate
    float4 colorTint        : COLOR; // Color tint
};

// Lights

// From C++
#define LIGHT_TYPE_DIRECTIONAL	0
#define LIGHT_TYPE_POINT		1
#define LIGHT_TYPE_SPOT			2

struct Light
{
    int Type;
    float3 Direction;
    float Range;
    float3 Position;
    float Intensity;
    float3 Color;
    float SpotInnerAngle;
    float SpotOuterAngle;
    int Active;
    float Padding;
};  

// Particles

struct Particle
{
    float EmitTime;
    float3 StartPosition;
    
    float4 StartColor;
    float4 FinalColor;
    
    float3 StartVelocity;
    float Padding;
};

// For HLSL
#define MAX_SPECULAR_EXPONENT 256.0f

#endif
