#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <memory>
#include <vector>

#include "Transform.h"
#include "Material.h"
#include "Mesh.h"



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

/*
SPLIT GENERATION VERTEX NAMES

		   .  Top  .
		.   .  |  / ..
		 .   \ V / // .
		  \   \./ // /
		   \.___../ /
  Bottom-> /\.____./
		  / /    /
		 / /    /
		/ /    /
*/

struct FoliageParams {
	unsigned int seed;					// Random seed
	DirectX::XMFLOAT3 growthDirection;	// Vector of starting trunk
	unsigned int maxIterations;			// Caps the number of segments that can be a part of a single branch
	float maxLength;					// Caps the maximum length branches can grow to
	
	float segmentLength;				// Average length for each branch segment
	float segmentLengthVariance;		// Width of random range for segment length
	float segmentLengthMultiplier;		// Multiplied to segment length & variance after each segment
	float segmentWidth;					// Average width for each branch segment
	float segmentWidthVariance;			// Width of random range for segment width
	float segmentWidthMultiplier;		// Multiplied to segment width & variance after each segment
	float segmentTurnAngleVariance;		// Width of random range for angle between segments' long sides,
										// as radians from directly parallel
	float segmentTwistAngleVariance;	// Width of random range for angle between segments' cross-sections,
										// as radians from directly aligned

	float splitChance;					// Percent chance that a branch will split
	float splitChanceMultiplier;		// Multiplied to split chance after each segment
	float splitAngle;					// Average angle at which split branches diverge after a split,
										// as radians from directly upwards
	float splitAngleVariance;			// Width of random range for split angle

	//DirectX::XMFLOAT3 gravity;		// Vector added to the angle of each branch
};

struct FoliageNode {
	DirectX::XMFLOAT4X4 tfLocal;		// The local transformation of this node from the root
	unsigned int iteration;				// How many nodes away from the root node this is
	unsigned int verticesStart;			// The index, in the Mesh's vertex array,
										// of the first vertex created by this node
	unsigned int indicesStart;			// The index, in the Mesh's index array,
										// of the first vertex created by this node
	float width;						// The width the ring around this node should be
	float totalLength;					// The total length accumulated by this node and its ancestors
	bool isFinal;							// Whether this node should be an end cap
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
	FoliageParams GetParams();
	const char* GetName();
	FoliageNode GetRootNode();
	FoliageNode GetNode(unsigned int _index);

	// Setters
	void SetBranchMaterial(std::shared_ptr<Material> _material);
	void SetLeafMaterial(std::shared_ptr<Material> _material);
	void SetParams(FoliageParams _params);

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
	unsigned int nodeCount;

	// Generates new nodes and a new mesh for branches from the parameters
	void GenerateBranchMesh();



	// HELPER FUNCTIONS FOR MESH GENERATION ONLY
	void TransformVectorByMatrix(
		DirectX::XMFLOAT3* _vector, 
		DirectX::XMFLOAT4X4 _matrix
	);
	void ScaleAndTransformVectorByMatrix(
		DirectX::XMFLOAT3* _vector, 
		DirectX::XMFLOAT4X4 _matrix, 
		float _scale
	);
	// Extends a new node off a given previous node
	FoliageNode BuildNodeFromParent(
		const FoliageNode& _parent, 
		unsigned int* _vertexCount, 
		unsigned int* _indexCount
	);
	// Adds a new quad of four vertices to the mesh, centered around a Node, and the necessary indices for it
	void AddNodeQuadVertices(
		std::vector<Vertex>* _vertices,
		unsigned int* _vertexCount,
		std::vector<UINT>* _indices,
		unsigned int* _indexCount,
		const FoliageNode& _node
	);
	// Adds a new ring of four vertices to the mesh, centered around a Node (Doesn't add indices yet)
	void AddNodeRingVerticesHardEdge(
		std::vector<Vertex>* _vertices,
		unsigned int* _vertexCount,
		const FoliageNode& _node
	);
	// Adds indices for the eight triangles in the segment between two Nodes
	void AddSegmentIndicesHardEdge(
		std::vector<Vertex>* _vertices,
		unsigned int* _vertexCount,
		std::vector<UINT>* _indices,
		unsigned int* _indexCount,
		unsigned int _parentNodeFirstVertex,
		unsigned int _childNodeFirstVertex
	);
	// Adds a new end cap of four vertices to the mesh, centered around a Node (Doesn't add indices yet)
	void AddNodeEndCapVerticesHardEdge(
		std::vector<Vertex>* _vertices,
		unsigned int* _vertexCount,
		const FoliageNode& _node
	);
	// Adds indices for the four triangles in the end cap between two Nodes
	void AddEndCapIndicesHardEdge(
		std::vector<Vertex>* _vertices,
		unsigned int* _vertexCount,
		std::vector<UINT>* _indices,
		unsigned int* _indexCount,
		unsigned int _parentNodeFirstVertex,
		unsigned int _childNodeFirstVertex
	);


	// CONSTANTS FOR MESH GENERATION

	// Constant quad vertices to copy to make new quads
	/*
	 0---1
	 |   |
	 2---3
	*/
	const Vertex QUAD_V0 = {
		DirectX::XMFLOAT3(-0.5f, 0.0f, 0.5f),	// Position
		DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f),	// Normal
		DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f),	// Tangent (will be calculated automatically)
		DirectX::XMFLOAT2(0.0f, 0.0f)			// UV
	};
	const Vertex QUAD_V1 = {
		DirectX::XMFLOAT3(0.5f, 0.0f, 0.5f),
		DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f),
		DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f),
		DirectX::XMFLOAT2(1.0f, 0.0f)
	};
	const Vertex QUAD_V2 = {
		DirectX::XMFLOAT3(-0.5f, 0.0f, -0.5f),
		DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f),
		DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f),
		DirectX::XMFLOAT2(0.0f, 1.0f)
	};
	const Vertex QUAD_V3 = {
		DirectX::XMFLOAT3(0.5f, 0.0f, -0.5f),
		DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f),
		DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f),
		DirectX::XMFLOAT2(1.0f, 1.0f)
	};
	// Indices for making a new quad
	const unsigned int QUAD_INDICES[6] = { 1, 2, 0, 1, 3, 2 };



	// Constant ring vertices to copy to make new rings
	/*
	   7   6
	   |   |
	0--0---3--5
	   |   |
	1--1---2--4
	   |   |
	   2   3
	*/
	const Vertex RING_V0TO1 = {
		DirectX::XMFLOAT3(-0.5f, 0.0f, 0.5f),	// Position
		DirectX::XMFLOAT3(-1.0f, 0.0f, 0.0f),	// Normal
		DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f),	// Tangent (will be calculated automatically)
		DirectX::XMFLOAT2(0.0f, 1.0f)			// UV
	};
	const Vertex RING_V1TO0 = {
		DirectX::XMFLOAT3(-0.5f, 0.0f, -0.5f),
		DirectX::XMFLOAT3(-1.0f, 0.0f, 0.0f),
		DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f),
		DirectX::XMFLOAT2(0.25f, 1.0f)
	};
	const Vertex RING_V1TO2 = {
		DirectX::XMFLOAT3(-0.5f, 0.0f, -0.5f),
		DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f),
		DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f),
		DirectX::XMFLOAT2(0.25f, 1.0f)
	};
	const Vertex RING_V2TO1 = {
		DirectX::XMFLOAT3(0.5f, 0.0f, -0.5f),
		DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f),
		DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f),
		DirectX::XMFLOAT2(0.5f, 1.0f)
	};
	const Vertex RING_V2TO3 = {
		DirectX::XMFLOAT3(0.5f, 0.0f, -0.5f),
		DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f),
		DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f),
		DirectX::XMFLOAT2(0.5f, 1.0f)
	};
	const Vertex RING_V3TO2 = {
		DirectX::XMFLOAT3(0.5f, 0.0f, 0.5f),
		DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f),
		DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f),
		DirectX::XMFLOAT2(0.75f, 1.0f)
	};
	const Vertex RING_V3TO0 = {
		DirectX::XMFLOAT3(0.5f, 0.0f, 0.5f),
		DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f),
		DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f),
		DirectX::XMFLOAT2(0.75f, 1.0f)
	};
	const Vertex RING_V0TO3 = {
		DirectX::XMFLOAT3(-0.5f, 0.0f, 0.5f),
		DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f),
		DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f),
		DirectX::XMFLOAT2(1.0f, 1.0f)
	};
	// Indices for making a new quad between two rings
	const unsigned int SEGMENT_FACE_INDICES[6] = { 3, 0, 2, 3, 1, 0 };



	// Constant end cap vertices to copy to make new end caps
	/*
	   3
	 0-+-2
	   1
	*/
	const Vertex END_CAP_V0 = {
		DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f),	// Position
		DirectX::XMFLOAT3(-1.0f, 0.0f, 0.0f),	// Normal
		DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f),	// Tangent (will be calculated automatically)
		DirectX::XMFLOAT2(0.125f, 0.0f)			// UV
	};
	const Vertex END_CAP_V1 = {
		DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f),
		DirectX::XMFLOAT3(0.0f, 0.0f, -1.0f),
		DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f),
		DirectX::XMFLOAT2(0.375f, 0.0f)
	};
	const Vertex END_CAP_V2 = {
		DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f),
		DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f),
		DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f),
		DirectX::XMFLOAT2(0.625f, 0.0f)
	};
	const Vertex END_CAP_V3 = {
		DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f),
		DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f),
		DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f),
		DirectX::XMFLOAT2(0.875f, 0.0f)
	};
	// Indices for making a new end cap face between this node and the last ring
	const unsigned int END_CAP_FACE_INDICES[3] = { 2, 1, 0 };
};

