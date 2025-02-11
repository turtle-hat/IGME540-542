#ifndef __GGP_SHADER_LIGHTING__
#define __GGP_SHADER_LIGHTING__

#include "ShaderStructs.hlsli"

#define LIGHT_COUNT	8





// CONSTANTS ===================


// A constant Fresnel value for non-metals (glass and plastic have values of about 0.04)
static const float F0_NON_METAL = 0.04f;

// Minimum roughness for when spec distribution function denominator goes to zero
static const float MIN_ROUGHNESS = 0.0000001f; // 6 zeros after decimal

// Handy to have this as a constant
static const float PI = 3.14159265359f;





// GENERAL FUNCTIONS ================



// Calculates the strength of a point light for a pixel. Returns a value that attenuates the light when multiplied
float Attenuate(Light _light, float3 _worldPosition)
{
    float attenuationDistance = distance(_light.Position, _worldPosition);
    float attenuation = saturate(1.0f - (pow(attenuationDistance, 2) / pow(_light.Range, 2)));
    return pow(attenuation, 2);
}

// Calculates the strength of a spot light for a pixel. Returns a value that attenuates the light when multiplied
float SpotTerm(Light _light, float3 _lightDirectionIn)
{
    float cosOuter = cos(_light.SpotOuterAngle);
    float cosInner = cos(_light.SpotInnerAngle);
    
    return saturate(
        (cosOuter - 
            saturate(dot(_lightDirectionIn, _light.Direction)) // Angle from pixel to light direction
        ) /
        (cosOuter - cosInner) // Falloff range
    );

}

// Calculates the Fresnel term for a pixel using Schlick's approximation
float FresnelSchlick(float3 _normal, float3 _cameraViewOut, float _specularAmount)
{
    return _specularAmount + (1.0f - _specularAmount) * pow(1.0f - saturate(dot(_normal, _cameraViewOut)), 5);
}





// LAMBERT PHONG FUNCTIONS ================



// Calculates Lambert diffuse shading (N dot L) for a pixel
float DiffuseLambert(float3 _surfaceNormal, float3 _lightDirectionOut)
{
    // Get dot product of normal and light direction, scale by intensity and the color of the light and surface 
    return saturate(dot(_surfaceNormal, _lightDirectionOut));
}

// Calculates Phong specular shading for a pixel 
float SpecularPhong(float3 _lightDirectionIn, float3 _surfaceNormal, float _surfaceRoughness, float3 _surfaceWorldPos, float3 _cameraPosition)
{
    return pow(
        saturate(dot(
            reflect(_lightDirectionIn, _surfaceNormal), // R
            normalize(_cameraPosition - _surfaceWorldPos) // V
        )),
        (1.0f - min(_surfaceRoughness, 0.99f)) * MAX_SPECULAR_EXPONENT // Specular Exponent
    );
}

// Calculates Lambert & Phong lighting result of a single light
float3 LightLambertPhong(Light _light, float3 _lightDirectionIn, float3 _surfaceNormal, float3 _surfaceColor, float _surfaceRoughness, float _surfaceSpecular, float3 _surfaceWorldPos, float3 _cameraPosition)
{
    float diffuse = DiffuseLambert(_surfaceNormal, -_lightDirectionIn);
    float specular = _surfaceSpecular * SpecularPhong(_lightDirectionIn, _surfaceNormal, _surfaceRoughness, _surfaceWorldPos, _cameraPosition) * any(diffuse); // Specular is cut if diffuse term is 0
    
    return _light.Color * _light.Intensity * (_surfaceColor * diffuse + specular);
}

// Calculates Lambert & Phong lighting result of an array of lights
float3 CalculateLightingLambertPhong(Light _lights[LIGHT_COUNT], float3 _lightAmbient, float3 _surfaceNormal, float3 _surfaceColor, float _surfaceRoughness, float _surfaceSpecular, float3 _surfaceWorldPos, float3 _cameraPosition)
{
    // Start accumulator with ambient light value
    float3 lightsFinal = _lightAmbient;

	// For each light in the scene
    for (uint i = 0; i < LIGHT_COUNT; i++)
    {
		// If it's active
        if (_lights[i].Active)
        {
            // Declare lightDirectionIn, calculated differently for each light type
                float3 lightDirectionIn;
            
			// Run a different lighting equation on it depending on the type of light
            switch (_lights[i].Type)
            {
                case LIGHT_TYPE_DIRECTIONAL:
                    // Direction in is the light's direction 
                    lightDirectionIn = normalize(_lights[i].Direction);
                    
                    lightsFinal += LightLambertPhong(_lights[i], lightDirectionIn, _surfaceNormal, _surfaceColor, _surfaceRoughness, _surfaceSpecular, _surfaceWorldPos, _cameraPosition);
                    break;
                case LIGHT_TYPE_POINT:
                    // Direction in is the vector to the light's position
                    lightDirectionIn = normalize(_surfaceWorldPos - _lights[i].Position);
     
                    lightsFinal += (
                        LightLambertPhong(_lights[i], lightDirectionIn, _surfaceNormal, _surfaceColor, _surfaceRoughness, _surfaceSpecular, _surfaceWorldPos, _cameraPosition)
                        // Light attenuates over distance
                        * Attenuate(_lights[i], _surfaceWorldPos)
                    );
                    break;
                case LIGHT_TYPE_SPOT:
                    // Direction in is the vector to the light's position
                    lightDirectionIn = normalize(_surfaceWorldPos - _lights[i].Position);
    
                    lightsFinal += (
                        LightLambertPhong(_lights[i], lightDirectionIn, _surfaceNormal, _surfaceColor, _surfaceRoughness, _surfaceSpecular, _surfaceWorldPos, _cameraPosition)
                        // Light attenuates over distance
                        * Attenuate(_lights[i], _surfaceWorldPos)
                        // Light attenuates at edges of spot light
                        * SpotTerm(_lights[i], lightDirectionIn)
                    );
                    break;
                default:
                    break;
            }
        }
    }
    
    return lightsFinal;
}





// PBR FUNCTIONS ================
// Provided by Prof. Cascioli



// Calculates diffuse amount based on energy conservation
//
// diffuse   - Diffuse amount
// F         - Fresnel result from microfacet BRDF
// metalness - surface metalness amount 
float3 DiffuseEnergyConserve(float3 _surfaceDiffuse, float3 _fresnel, float _surfaceMetalness)
{
    return _surfaceDiffuse * (1.0f - _fresnel) * (1.0f - _surfaceMetalness);
}

// Normal Distribution Function: GGX (Trowbridge-Reitz)
//
// n - Normal
// a - Roughness
// h - Half vector
// 
// D(h, n, a) = a^2 / pi * ((n dot h)^2 * (a^2 - 1) + 1)^2
float NormDistGGX(float3 _surfaceNormal, float3 _halfViewLight, float _surfaceRoughness)
{
	// Pre-calculations
    float NdotH = saturate(dot(_surfaceNormal, _halfViewLight));
    float NdotH2 = NdotH * NdotH;
    float a = _surfaceRoughness * _surfaceRoughness;
    float a2 = max(a * a, MIN_ROUGHNESS); // Applied after remap!

	// ((n dot h)^2 * (a^2 - 1) + 1)
	// Can go to zero if roughness is 0 and NdotH is 1
    float denomToSquare = NdotH2 * (a2 - 1) + 1;

	// Final value
    return a2 / (PI * denomToSquare * denomToSquare);
}

// Fresnel term - Schlick approx. for microfacet models
// 
// v - View vector
// h - Half vector
// f0 - Value when l = n
//
// F(v,h,f0) = f0 + (1-f0)(1 - (v dot h))^5
float3 FresnelSchlickPBR(float3 _cameraViewOut, float3 _halfViewLight, float3 _specularAmount)
{
	// Final value
    return _specularAmount + (1.0f - _specularAmount) * pow(1.0f - saturate(dot(_cameraViewOut, _halfViewLight)), 5);
}

// Geometric Shadowing - Schlick-GGX
// - k is remapped to a / 2, roughness remapped to (r+1)/2 before squaring!
//
// n - Normal
// v - View vector
//
// G_Schlick(n,v,a) = (n dot v) / ((n dot v) * (1 - k) * k)
//
// Full G(n,v,l,a) term = G_SchlickGGX(n,v,a) * G_SchlickGGX(n,l,a)
float GeoShadowSchlickGGX(float3 _surfaceNormal, float3 _viewOut, float _surfaceRoughness)
{
	// End result of remapping:
    float k = pow(_surfaceRoughness + 1, 2) / 8.0f;
    float NdotV = saturate(dot(_surfaceNormal, _viewOut));

	// Final value
	// Note: Numerator should be NdotV (or NdotL depending on parameters).
	// However, these are also in the BRDF's denominator, so they'll cancel!
	// We're leaving them out here AND in the BRDF function as the
	// dot products can get VERY small and cause rounding errors.
    return 1 / (NdotV * (1 - k) + k);
}

// Cook-Torrance Microfacet BRDF (Specular)
//
// f(l,v) = D(h)F(v,h)G(l,v,h) / 4(n dot l)(n dot v)
// - parts of the denominator are canceled out by numerator (see below)
//
// D() - Normal Distribution Function - Trowbridge-Reitz (GGX)
// F() - Fresnel - Schlick approx
// G() - Geometric Shadowing - Schlick-GGX
float3 MicrofacetBRDF(float3 _surfaceNormal, float3 _lightDirectionOut, float3 _cameraViewOut, float _surfaceRoughness, float3 _specularAmount, float _NdotL, out float3 _fresnel_out)
{
	// Other vectors
    float3 halfViewLight = normalize(_cameraViewOut + _lightDirectionOut);

	// Run numerator functions
    float D = NormDistGGX(_surfaceNormal, halfViewLight, _surfaceRoughness);
    float3 F = FresnelSchlickPBR(_cameraViewOut, halfViewLight, _specularAmount);
    float G = GeoShadowSchlickGGX(_surfaceNormal, _cameraViewOut, _surfaceRoughness) * GeoShadowSchlickGGX(_surfaceNormal, _lightDirectionOut, _surfaceRoughness);
	
	// Pass F out of the function for diffuse balance
    _fresnel_out = F;

	// Final specular formula
	// Note: The denominator SHOULD contain (NdotV)(NdotL), but they'd be
	// canceled out by our G() term.  As such, they have been removed
	// from BOTH places to prevent floating point rounding errors.
    float3 specularResult = (D * F * G) / 4;

	// One last non-obvious requirement: According to the rendering equation,
	// specular must have the same NdotL applied as diffuse!  We'll apply
	// that here so that minimal changes are required elsewhere.
    return specularResult * _NdotL;
}

// Calculates Lambert & Cook-Torrance lighting result of a single light
float3 LightLambertCookTorrance(Light _light, float3 _lightDirectionOut, float3 _surfaceNormal, float3 _surfaceColor, float _surfaceRoughness, float _surfaceMetalness, float3 _surfaceWorldPos, float3 _cameraPosition)
{
    // Find color of specularity
    float3 specularColor = lerp(F0_NON_METAL, _surfaceColor, _surfaceMetalness);
    
    // Calculate diffuse term
    float diffuse = DiffuseLambert(_surfaceNormal, _lightDirectionOut);
    
    // Fresnel term to be calculated with specular term
    float3 fresnel;
    
    // Calculate specular term
    float3 specular = MicrofacetBRDF(
        _surfaceNormal,
        _lightDirectionOut,
        normalize(_cameraPosition - _surfaceWorldPos),
        max(_surfaceRoughness, MIN_ROUGHNESS),
        specularColor,
        diffuse,
        fresnel
    );
    
    // Balance diffuse term based on specular term
    float3 diffuseBalanced = DiffuseEnergyConserve(diffuse, fresnel, _surfaceMetalness);

    // Combine diffuse, specular, light, and color
    return (diffuseBalanced * _surfaceColor + specular) * _light.Intensity * _light.Color;
}

// Calculates physically based Lambert & Cook-Torrance lighting result of an array of lights
float3 CalculateLightingLambertCookTorrance(Light _lights[LIGHT_COUNT], float3 _surfaceNormal, float3 _surfaceColor, float _surfaceRoughness, float _surfaceMetalness, float3 _surfaceWorldPos, float3 _cameraPosition)
{
    // Start accumulator with nothing
    float3 lightsFinal = 0.0f;

	// For each light in the scene
    for (uint i = 0; i < LIGHT_COUNT; i++)
    {
		// If it's active
        if (_lights[i].Active)
        {
			// Declare lightDirectionIn, calculated differently for each light type
            float3 lightDirectionIn;
            float3 lightResult = 0;
            
			// Run a different lighting equation on it depending on the type of light
            switch (_lights[i].Type)
            {
                case LIGHT_TYPE_DIRECTIONAL:
                    // Direction in is the light's direction 
                    lightDirectionIn = normalize(_lights[i].Direction);
                    
                    lightResult = LightLambertCookTorrance(_lights[i], -lightDirectionIn, _surfaceNormal, _surfaceColor, _surfaceRoughness, _surfaceMetalness, _surfaceWorldPos, _cameraPosition);
                    break;
                case LIGHT_TYPE_POINT:
                    // Direction in is the vector to the light's position
                    lightDirectionIn = normalize(_surfaceWorldPos - _lights[i].Position);
     
                    lightResult = (
                        LightLambertCookTorrance(_lights[i], -lightDirectionIn, _surfaceNormal, _surfaceColor, _surfaceRoughness, _surfaceMetalness, _surfaceWorldPos, _cameraPosition)
                        // Light attenuates over distance
                        * Attenuate(_lights[i], _surfaceWorldPos)
                    );
                    break;
                case LIGHT_TYPE_SPOT:
                    // Direction in is the vector to the light's position
                    lightDirectionIn = normalize(_surfaceWorldPos - _lights[i].Position);
    
                    lightResult = (
                        LightLambertCookTorrance(_lights[i], -lightDirectionIn, _surfaceNormal, _surfaceColor, _surfaceRoughness, _surfaceMetalness, _surfaceWorldPos, _cameraPosition)
                        // Light attenuates over distance
                        * Attenuate(_lights[i], _surfaceWorldPos)
                        // Light attenuates at edges of spot light
                        * SpotTerm(_lights[i], lightDirectionIn)
                    );
                    break;
                default:
                    break;
            }
            
            lightsFinal += lightResult;
        }
    }
    
    return lightsFinal;
}

// Interpolates calculated lit color with reflection using a Fresnel calculation
float3 CalculateReflections(float3 _totalLight, float3 _reflectionColor, float3 _normal, float3 _cameraDirectionOut)
{
    return lerp(
        _totalLight,
        _reflectionColor,
        FresnelSchlick(_normal, _cameraDirectionOut, F0_NON_METAL)
    );
}

#endif
