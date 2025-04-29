#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <memory>

#include "Transform.h"
#include "Material.h"
#include "Mesh.h"

struct FoliageParams {
	unsigned int seed;					// Random seed
	DirectX::XMFLOAT3 growthDirection;	// Vector of starting trunk
	
	float segmentLength;				// Average length for each branch segment
	float segmentLengthVariance;		// Width of random range for segment length
	float segmentLengthMultiplier;		// Multiplied to segment length & variance after each segment
	float segmentWidth;					// Average width for each branch segment
	float segmentWidthVariance;			// Width of random range for segment width
	float segmentWidthMultiplier;		// Multiplied to segment width & variance after each segment
	float segmentAngleVariance;			// Width of random range for segment angle
	
	float segmentCost;					// Value from 0.0f to 1.0f, preferably low. Added to UV coordinates after each segment. If reaches 1.0f or higher, branch terminates.
	float segmentCostVariance;			// Width of random range for segment cost

	float splitChance;					// Percent chance that a branch will split
	float splitChanceMultiplier;		// Multiplied to split chance after each segment
	float splitAngle;					// Average angle at which branches diverge after a split
	float splitAngleVariance;			// Width of random range for split angle

	 //DirectX::XMFLOAT3 gravity;		// Vector added to the angle of each branch
};

/*
RELATIVE TRANSLATION AXES		RELATIVE ROTATION AXES
								(rotated about each axis)

            Y                             Yaw
        Z  /                         Roll /
         \/___ X                        \/___ Pitch

	______				           ______
   /\_____\				          /\_____\
  / /     /				         / /     /
 / /     /				        / /     /
/ /     /				         /     /
*/



class Foliage
{
public:
	Foliage(const char* _name, std::shared_ptr<Material> _branchMaterial, std::shared_ptr<Material> _leafMaterial, FoliageParams _params, std::shared_ptr<Transform> _transform);
	Foliage(const char* _name, std::shared_ptr<Material> _branchMaterial, std::shared_ptr<Material> _leafMaterial, FoliageParams _params);
	~Foliage();

	void RegenerateMesh();

private:
	void GenerateMesh();

	Mesh _mesh;
};

