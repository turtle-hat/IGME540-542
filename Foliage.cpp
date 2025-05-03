#include "Foliage.h"

#include <queue>

using namespace std;
using namespace DirectX;

// Helper macro for getting a float within a random range centered around 0
// (Also used in ParticleEmitter.cpp, should probably merge those)
#define RandomRange(width) ((float)rand() / RAND_MAX * width - (width / 2.0f))

Foliage::Foliage(const char* _name, std::shared_ptr<Material> _branchMaterial, std::shared_ptr<Material> _leafMaterial, FoliageParams _params, std::shared_ptr<Transform> _transform)
{
	name = _name;
	branchMaterial = _branchMaterial;
	leafMaterial = _leafMaterial;
	params = _params;
	transform = _transform;

	GenerateBranchMesh();
}

Foliage::Foliage(const char* _name, std::shared_ptr<Material> _branchMaterial, std::shared_ptr<Material> _leafMaterial, FoliageParams _params) :
	Foliage(_name, _branchMaterial, _leafMaterial, _params, make_shared<Transform>()) {}

Foliage::~Foliage()
{
}

std::shared_ptr<Mesh> Foliage::GetMesh()
{
	return mesh;
}

std::shared_ptr<Material> Foliage::GetBranchMaterial()
{
	return branchMaterial;
}

std::shared_ptr<Material> Foliage::GetLeafMaterial()
{
	return leafMaterial;
}

std::shared_ptr<Transform> Foliage::GetTransform()
{
	return transform;
}

FoliageParams Foliage::GetParams()
{
	return params;
}

const char* Foliage::GetName()
{
	return name;
}

FoliageNode Foliage::GetRootNode()
{
	return nodes.size() > 0 ? nodes[0] : FoliageNode{};
}

FoliageNode Foliage::GetNode(unsigned int _index)
{
	return nodes.size() > _index ? nodes[_index] : FoliageNode{};
}

void Foliage::SetBranchMaterial(std::shared_ptr<Material> _material)
{
	branchMaterial = _material;
}

void Foliage::SetLeafMaterial(std::shared_ptr<Material> _material)
{
	leafMaterial = _material;
}

void Foliage::SetParams(FoliageParams _params)
{
	params = _params;
	GenerateBranchMesh();
}

void Foliage::GenerateBranchMesh()
{
	nodes.clear();
	srand(params.seed);

	vector<Vertex> vertices;		// Generated vertices
	vector<UINT> indices;			// Generated indices
	unsigned int vertexCount = 0;	// Counter for vertices
	unsigned int indexCount = 0;	// Counter for indices

	queue<FoliageNode> nodesToBuild;	// Stores the nodes in the list that can are still valid to build off of

	// Calculate basis vectors for root node
	XMFLOAT3 rootRight(1.0f, 0.0f, 0.0f);
	XMFLOAT3 rootUp(0.0f, 1.0f, 0.0f);
	XMFLOAT3 rootForward(0.0f, 0.0f, 1.0f);
	// Store XMVECTOR for up basis so it doesn't need to be loaded later
	XMVECTOR vecRootUp;

	// If growth direction is nonzero, normalize it and use it as the Up basis
	if (abs(params.growthDirection.x) + abs(params.growthDirection.y) + abs(params.growthDirection.z) > 0.0f)
	{
		vecRootUp = XMVector3Normalize(XMLoadFloat3(&params.growthDirection));
		XMStoreFloat3(&rootUp, vecRootUp);
	}
	else {
		vecRootUp = XMLoadFloat3(&rootUp);
	}

	// Then, determine what the second and third bases should be

	// If there is no X component, use (1, 0, 0) as the second basis
	// and calculate the third from a cross product
	if (abs(rootUp.x) == 0.0f) {
		// Calculate forward basis as the cross product of the other two and store
		XMStoreFloat3(&rootForward, XMVector3Cross(
			XMLoadFloat3(&rootRight),
			vecRootUp
		));
	}
	// Otherwise, use (0, 0, 1) as the second basis
	else {
		// XMVECTOR of the forward vector
		XMVECTOR vecRootForward = XMLoadFloat3(&rootForward);

		// If there is a z component to the up vector, orthonormalize the forward basis
		// to make it orthogonal to the up vector
		if (abs(rootUp.z) > 0.0f) {
			// Gram-Schmidt orthonormalize
			vecRootForward = XMVector3Normalize(
				vecRootForward - vecRootUp * XMVector3Dot(vecRootUp, vecRootForward));
			
			// Store forward basis
			XMStoreFloat3(&rootForward, vecRootForward);
		}
		
		// Calculate right basis as the cross product of the other two and store
		XMStoreFloat3(&rootRight, XMVector3Cross(
			vecRootUp,
			vecRootForward
		));
	}

	// Create root node
	FoliageNode root = {
		XMFLOAT4X4(				// tfLocal
			rootRight.x,	rootRight.y,	rootRight.z,	0.0f,
			rootUp.x,		rootUp.y,		rootUp.z,		0.0f,
			rootForward.x,	rootForward.y,	rootForward.z,	0.0f,
			0.0f,			0.0f,			0.0f,			1.0f
		),
		0,						// iteration
		0,						// verticesStart
		0,						// indicesStart
		params.segmentWidth,	// width
		0.0f,					// totalLength
		false					// isFinal
	};
	nodes.push_back(root);

	// Add first ring, then ring above it
	AddNodeRingVerticesHardEdge(&vertices, &vertexCount, root);

	nodesToBuild.push(root);

	// While there are still nodes to build from, add a child node on top
	while (!nodesToBuild.empty()) {
		FoliageNode parent = nodesToBuild.front();
		nodesToBuild.pop();

		FoliageNode child = BuildNodeFromParent(parent, &vertexCount, &indexCount);
		
		// If child is final, add end cap
		if (child.isFinal) {
			// Add vertices for end cap
			AddNodeEndCapVerticesHardEdge(&vertices, &vertexCount, child);

			// Connect the end cap to the ring by adding indices
			AddEndCapIndicesHardEdge(
				&vertices,
				&vertexCount,
				&indices,
				&indexCount,
				parent.verticesStart,
				child.verticesStart
			);

			// DO NOT push the child back onto the queue

		// Otherwise, add vertex ring
		} else {
			// Add vertices for ring
			AddNodeRingVerticesHardEdge(&vertices, &vertexCount, child);

			// Connect the two rings by adding indices
			AddSegmentIndicesHardEdge(
				&vertices,
				&vertexCount,
				&indices,
				&indexCount,
				parent.verticesStart,
				child.verticesStart
			);

			// Push the child 
			nodesToBuild.push(child);
		}
	}

	// Add a quad at the root for testing
	//AddNodeQuadVertices(&vertices, &vertexCount, &indices, &indexCount, root);

	// FINAL STEP: Make Mesh object
	mesh = make_shared<Mesh>("M_Foliage_Generated", vertices.data(), vertexCount, indices.data(), indexCount);
}

void Foliage::TransformVectorByMatrix(DirectX::XMFLOAT3* _vector, DirectX::XMFLOAT4X4 _matrix)
{
	XMStoreFloat3(_vector, XMVector3Transform(XMLoadFloat3(_vector), XMLoadFloat4x4(&_matrix)));
}

void Foliage::ScaleAndTransformVectorByMatrix(DirectX::XMFLOAT3* _vector, DirectX::XMFLOAT4X4 _matrix, float _scale)
{
	XMStoreFloat3(_vector, XMVector3Transform(
		XMVectorScale(XMLoadFloat3(_vector), _scale),
		XMLoadFloat4x4(&_matrix)));
}

FoliageNode Foliage::BuildNodeFromParent(const FoliageNode& _parent, unsigned int* _vertexCount, unsigned int* _indexCount)
{
	// Use parent's data as a template
	FoliageNode result = _parent;

	// Get growth direction from parent's up vector
	XMFLOAT3 growthDirection(
		result.tfLocal._21,
		result.tfLocal._22,
		result.tfLocal._23
	);

	// Add iteration
	result.iteration += 1;
	// If at or somehow above maximum, set as final
	if (result.iteration >= params.maxIterations) {
		result.isFinal = true;
	}

	// Calculate the length
	float segmentLength = (params.segmentLength + RandomRange(params.segmentLengthVariance)) * powf(params.segmentLengthMultiplier, (float)result.iteration);
	// If over maximum, set as final and clamp to fill the rest of the space
	float freeLength = params.maxLength - result.totalLength;
	if (segmentLength > freeLength) {
		result.isFinal = true;
		segmentLength = freeLength;
	}
	result.totalLength += segmentLength;

	// Calculate the width
	float segmentWidth = (params.segmentWidth + RandomRange(params.segmentWidthVariance)) * powf(params.segmentWidthMultiplier, (float)result.iteration);
	// If 0 or lower, set as final and clamp to 0
	if (segmentWidth <= 0.0f) {
		result.isFinal = true;
		segmentWidth = 0.0f;
	}
	result.width = segmentWidth;

	// Scale growth direction by segment length and translate by that vector
	XMStoreFloat4x4(&result.tfLocal, XMMatrixMultiply(
		XMLoadFloat4x4(&result.tfLocal),
		XMMatrixTranslationFromVector(XMVectorScale(XMLoadFloat3(&growthDirection), segmentLength))
	));

	// Update other fields
	result.verticesStart = *_vertexCount;
	result.indicesStart = *_indexCount;

	return result;
}


void Foliage::AddNodeQuadVertices(std::vector<Vertex>* _vertices, unsigned int* _vertexCount, std::vector<UINT>* _indices, unsigned int* _indexCount, const FoliageNode& _node)
{
	// Get new quad vertices
	Vertex v0 = QUAD_V0;
	Vertex v1 = QUAD_V1;
	Vertex v2 = QUAD_V2;
	Vertex v3 = QUAD_V3;

	// Transform all vertices' positions and normals by the vector,
	// scaling the quad vertices' position by the node's width first
	// (there's probably a more efficient way to do this)
	ScaleAndTransformVectorByMatrix(&v0.Position, _node.tfLocal, _node.width);
	ScaleAndTransformVectorByMatrix(&v1.Position, _node.tfLocal, _node.width);
	ScaleAndTransformVectorByMatrix(&v2.Position, _node.tfLocal, _node.width);
	ScaleAndTransformVectorByMatrix(&v3.Position, _node.tfLocal, _node.width);
	TransformVectorByMatrix(&v0.Normal, _node.tfLocal);
	TransformVectorByMatrix(&v1.Normal, _node.tfLocal);
	TransformVectorByMatrix(&v2.Normal, _node.tfLocal);
	TransformVectorByMatrix(&v3.Normal, _node.tfLocal);

	// Add vertices to vertex vector and add 4 to vertexCount
	_vertices->push_back(v0);
	_vertices->push_back(v1);
	_vertices->push_back(v2);
	_vertices->push_back(v3);

	// Add indices of the quad to the index vector and add 6 to indexCount
	unsigned int initialVertexCount = *_vertexCount;
	for (int i = 0; i < 6; i++) {
		_indices->push_back(QUAD_INDICES[i] + initialVertexCount);
	}
	*_vertexCount += 4;
	*_indexCount += 6;
}

void Foliage::AddNodeRingVerticesHardEdge(std::vector<Vertex>* _vertices, unsigned int* _vertexCount, const FoliageNode& _node)
{
	// Get new quad vertices
	Vertex v0to1 = RING_V0TO1;
	Vertex v1to0 = RING_V1TO0;
	Vertex v1to2 = RING_V1TO2;
	Vertex v2to1 = RING_V2TO1;
	Vertex v2to3 = RING_V2TO3;
	Vertex v3to2 = RING_V3TO2;
	Vertex v3to0 = RING_V3TO0;
	Vertex v0to3 = RING_V0TO3;

	// Transform all vertices' positions and normals by the vector,
	// scaling the ring vertices' position by the node's width first
	ScaleAndTransformVectorByMatrix(&v0to1.Position, _node.tfLocal, _node.width);
	ScaleAndTransformVectorByMatrix(&v1to0.Position, _node.tfLocal, _node.width);
	ScaleAndTransformVectorByMatrix(&v1to2.Position, _node.tfLocal, _node.width);
	ScaleAndTransformVectorByMatrix(&v2to1.Position, _node.tfLocal, _node.width);
	ScaleAndTransformVectorByMatrix(&v2to3.Position, _node.tfLocal, _node.width);
	ScaleAndTransformVectorByMatrix(&v3to2.Position, _node.tfLocal, _node.width);
	ScaleAndTransformVectorByMatrix(&v3to0.Position, _node.tfLocal, _node.width);
	ScaleAndTransformVectorByMatrix(&v0to3.Position, _node.tfLocal, _node.width);
	TransformVectorByMatrix(&v0to1.Normal, _node.tfLocal);
	TransformVectorByMatrix(&v1to0.Normal, _node.tfLocal);
	TransformVectorByMatrix(&v1to2.Normal, _node.tfLocal);
	TransformVectorByMatrix(&v2to1.Normal, _node.tfLocal);
	TransformVectorByMatrix(&v2to3.Normal, _node.tfLocal);
	TransformVectorByMatrix(&v3to2.Normal, _node.tfLocal);
	TransformVectorByMatrix(&v3to0.Normal, _node.tfLocal);
	TransformVectorByMatrix(&v0to3.Normal, _node.tfLocal);

	// Three conditions can decrease the texture V coordinate of a node's vertices (i.e. move up the texture):
	// 1. width equaling 0.0f (if true, always set V to 0.0f)
	// 2. iteration approaching maxIteration
	// 3. totalLength approaching maxLength
	// Whichever is the highest gets subtracted from 1.0f to become the new V coordinate
	float vIteration = (float)_node.iteration / (float)params.maxIterations; // 2.
	float vLength = (float)_node.totalLength / (float)params.maxLength; // 3.
	float vFinal = _node.width > 0.0f ? // 1.
		1.0f - max(max(vIteration, vLength), 1.0f)
		: 0.0f;

	v0to1.UV.y = vFinal;
	v1to0.UV.y = vFinal;
	v1to2.UV.y = vFinal;
	v2to1.UV.y = vFinal;
	v2to3.UV.y = vFinal;
	v3to2.UV.y = vFinal;
	v3to0.UV.y = vFinal;
	v0to3.UV.y = vFinal;

	// Add vertices to vertex vector and add 4 to vertexCount
	_vertices->push_back(v0to1);
	_vertices->push_back(v1to0);
	_vertices->push_back(v1to2);
	_vertices->push_back(v2to1);
	_vertices->push_back(v2to3);
	_vertices->push_back(v3to2);
	_vertices->push_back(v3to0);
	_vertices->push_back(v0to3);
	*_vertexCount += 8;
}

void Foliage::AddSegmentIndicesHardEdge(std::vector<Vertex>* _vertices, unsigned int* _vertexCount, std::vector<UINT>* _indices, unsigned int* _indexCount, unsigned int _parentNodeFirstVertex, unsigned int _childNodeFirstVertex)
{
	// Loop through each index of each face
	for (unsigned int face = 0; face < 4; face++) {
		for (unsigned int index = 0; index < 6; index++) {
			// Get index from constant array
			unsigned int ringQuadIndex = SEGMENT_FACE_INDICES[index];
			_indices->push_back(ringQuadIndex < 2 ? 
				// If index is 0 or 1 (i.e. on the parent's ring),
				// get the parent's start vertex, offset by 2 per face, and add the known index
				_parentNodeFirstVertex + (face * 2) + ringQuadIndex :
				// If index is 2 or 3 (i.e. on the child's ring),
				// get the child's start vertex, offset by 2 per face, and add the known index
				// (minus the two used to indicate this is the child node)
				_childNodeFirstVertex + (face * 2) + ringQuadIndex - 2
			);
		}
	}
	*_indexCount += 24;
}

void Foliage::AddNodeEndCapVerticesHardEdge(std::vector<Vertex>* _vertices, unsigned int* _vertexCount, const FoliageNode& _node)
{
	// Get new quad vertices
	Vertex v0 = END_CAP_V0;
	Vertex v1 = END_CAP_V1;
	Vertex v2 = END_CAP_V2;
	Vertex v3 = END_CAP_V3;

	// Transform all vertices' positions and normals by the vector,
	// scaling the quad vertices' position by the node's width first
	// (there's probably a more efficient way to do this)
	TransformVectorByMatrix(&v0.Position, _node.tfLocal);
	TransformVectorByMatrix(&v1.Position, _node.tfLocal);
	TransformVectorByMatrix(&v2.Position, _node.tfLocal);
	TransformVectorByMatrix(&v3.Position, _node.tfLocal);
	TransformVectorByMatrix(&v0.Normal, _node.tfLocal);
	TransformVectorByMatrix(&v1.Normal, _node.tfLocal);
	TransformVectorByMatrix(&v2.Normal, _node.tfLocal);
	TransformVectorByMatrix(&v3.Normal, _node.tfLocal);

	// Three conditions can decrease the texture V coordinate of a node's vertices (i.e. move up the texture):
	// 1. width equaling 0.0f (if true, always set V to 0.0f)
	// 2. iteration approaching maxIteration
	// 3. totalLength approaching maxLength
	// Whichever is the highest gets subtracted from 1.0f to become the new V coordinate
	float vIteration = (float)_node.iteration / params.maxIterations; // 2.
	float vLength = (float)_node.totalLength / params.maxLength; // 3.
	float vFinal = _node.width > 0.0f ? // 1.
		1.0f - max(max(vIteration, vLength), 1.0f)
		: 0.0f;

	// Add vertices to vertex vector and add 4 to vertexCount
	_vertices->push_back(v0);
	_vertices->push_back(v1);
	_vertices->push_back(v2);
	_vertices->push_back(v3);
	*_vertexCount += 4;
}

void Foliage::AddEndCapIndicesHardEdge(std::vector<Vertex>* _vertices, unsigned int* _vertexCount, std::vector<UINT>* _indices, unsigned int* _indexCount, unsigned int _parentNodeFirstVertex, unsigned int _childNodeFirstVertex)
{
	// Loop through each index of each face
	for (unsigned int face = 0; face < 4; face++) {
		for (unsigned int index = 0; index < 3; index++) {
			// Get index from constant array
			unsigned int endCapQuadIndex = END_CAP_FACE_INDICES[index] + face;
			_indices->push_back(endCapQuadIndex < 2 ?
				// If index is 0 or 1 (i.e. on the parent's ring),
				// get the parent's start vertex, offset by 2 per face, and add the known index
				_parentNodeFirstVertex + (face * 2) + endCapQuadIndex :
				// If index is 2 (i.e. on the child's ring),
				// get the child's start vertex and offset by 1 per face
				_childNodeFirstVertex + face
			);
		}
	}
	*_indexCount += 12;
}


