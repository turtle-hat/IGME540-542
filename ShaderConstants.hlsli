#ifndef __GGP_SHADER_CONSTANTS__
#define __GGP_SHADER_CONSTANTS__

#define MAX_LIGHTS	64
#define MAX_SPECULAR_EXPONENT 256.0f

// Ensure this matches Raytracing shader define!
#define MAX_INSTANCES_PER_BLAS 100
#define RAYS_PER_PIXEL 15
#define MAX_RAY_RECURSIONS 5
// Handy to have this as a constant
static const float PI = 3.14159265359f;

#endif