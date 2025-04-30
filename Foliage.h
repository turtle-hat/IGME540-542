#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <memory>
#include <vector>

#include "Transform.h"
#include "Material.h"
#include "Mesh.h"

struct FoliageParams {
	unsigned int seed;					// Random seed
	DirectX::XMFLOAT3 growthDirection;	// Vector of starting trunk
	unsigned int maxIterations;			// Caps the amount of segments that can be a part of a single branch
	
	float segmentLength;				// Average length for each branch segment
	float segmentLengthVariance;		// Width of random range for segment length
	float segmentLengthMultiplier;		// Multiplied to segment length & variance after each segment
	float segmentWidth;					// Average width for each branch segment
	float segmentWidthVariance;			// Width of random range for segment width
	float segmentWidthMultiplier;		// Multiplied to segment width & variance after each segment
	float segmentTurnAngleVariance;		// Width of random range for angle between segments' long sides
										// as radians from directly parallel
	float segmentTwistAngleVariance;	// Width of random range for angle between segments' cross-sections,
										// as radians counterclockwise from directly 

	float segmentCost;					// Value from 0.0f to 1.0f, preferably low. Added to UV coordinates after each segment.
										// If reaches 1.0f or higher, branch terminates.
	float segmentCostVariance;			// Width of random range for segment cost

	float splitChance;					// Percent chance that a branch will split
	float splitChanceMultiplier;		// Multiplied to split chance after each segment
	float splitAngle;					// Average angle at which split branches diverge after a split, as radians from directly upwards
	float splitAngleVariance;			// Width of random range for split angle

	//DirectX::XMFLOAT3 gravity;		// Vector added to the angle of each branch
};

/*
RELATIVE TRANSLATION AXES		RELATIVE ROTATION AXES
								(rotated about each axis)

            Y                             Yaw
        Z  /                         Roll /
         \/___ X                        \/___ Pitch

	_____                          _____
   /\____\                        /\____\
  / /    /                       / /    /
 / /    /                       / /    /
/ /    /                       / /    /
*/

struct FoliageNode {
	DirectX::XMFLOAT3 position;			// Position of node in object space
	DirectX::XMFLOAT3 normal;			// Normal vector of the segment's cross-section at this node
	unsigned int iteration;				// How many nodes away from the root node this is
};



class Foliage
{
public:
	// Constructors
	Foliage(const char* _name, std::shared_ptr<Material> _branchMaterial, std::shared_ptr<Material> _leafMaterial, FoliageParams _params, std::shared_ptr<Transform> _transform);
	Foliage(const char* _name, std::shared_ptr<Material> _branchMaterial, std::shared_ptr<Material> _leafMaterial, FoliageParams _params);
	~Foliage();

	// Getters
	std::shared_ptr<Mesh> GetMesh();
	std::shared_ptr<Material> GetBranchMaterial();
	std::shared_ptr<Material> GetLeafMaterial();
	std::shared_ptr<Transform> GetTransform();
	const char* GetName();

	// Setters
	void SetBranchMaterial(std::shared_ptr<Material> _material);
	void SetLeafMaterial(std::shared_ptr<Material> _material);

	void RegenerateMesh();

private:
	// User-defined fields
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<Material> branchMaterial;
	std::shared_ptr<Material> leafMaterial;
	std::shared_ptr<Transform> transform;
	FoliageParams params;

	// Name for UI
	const char* name;

	// Internal data
	std::vector<FoliageNode> nodes;

	void GenerateMesh();
};

