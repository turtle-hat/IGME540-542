#include "Foliage.h"

#include <stack>

using namespace std;
using namespace DirectX;

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

	vector<Vertex> vertices;		// Generated vertices
	vector<UINT> indices;			// Generated indices
	unsigned int vertexCount = 0;	// Counter for vertices
	unsigned int indexCount = 0;	// Counter for indices

	stack<FoliageNode> nodesToBuild;	// Stores the nodes in the list that can are still valid to build off of

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
		XMFLOAT4X4(
			rootRight.x,	rootRight.y,	rootRight.z,	0.0f,
			rootUp.x,		rootUp.y,		rootUp.z,		0.0f,
			rootForward.x,	rootForward.y,	rootForward.z,	0.0f,
			0.0f,			0.0f,			0.0f,			1.0f
		),
		0,
		0,
		0,
		params.segmentWidth,
		params.segmentLength,
		0.0f
	};
	nodes.push_back(root);

	// Add first ring, then ring above it
	AddNodeRingVerticesHardEdge(&vertices, &vertexCount, root);
	FoliageNode second = BuildNodeFromParent(root, &vertexCount, &indexCount);
	nodes.push_back(second);
	AddNodeRingVerticesHardEdge(&vertices, &vertexCount, second);

	// Connect the two rings by adding indices
	AddSegmentIndicesHardEdge(
		&vertices,
		&vertexCount,
		&indices,
		&indexCount,
		root.verticesStart,
		second.verticesStart
	);

	// Add a quad at the root
	AddNodeQuadVertices(&vertices, &vertexCount, &indices, &indexCount, root);

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
	FoliageNode result = _parent;

	// Get growth direction from parent's up vector
	XMFLOAT3 growthDirection(
		result.tfLocal._21,
		result.tfLocal._22,
		result.tfLocal._23
	);
	// Scale growth direction by segment length and translate by that vector
	XMStoreFloat4x4(&result.tfLocal, XMMatrixMultiply(
		XMLoadFloat4x4(&result.tfLocal),
		XMMatrixTranslationFromVector(XMVectorScale(XMLoadFloat3(&growthDirection), result.nextSegmentLength))
	));

	// Update other fields
	result.iteration++;
	result.verticesStart = *_vertexCount;
	result.indicesStart = *_indexCount;

	result.totalCost += params.segmentCost;

	return result;
}


void Foliage::AddNodeQuadVertices(std::vector<Vertex>* _vertices, unsigned int* _vertexCount, std::vector<UINT>* _indices, unsigned int* _indexCount, const FoliageNode& _node)
{
	unsigned int initialVertexCount = *_vertexCount;

	// Get new quad vertices
	Vertex v0 = QUAD_V0;
	Vertex v1 = QUAD_V1;
	Vertex v2 = QUAD_V2;
	Vertex v3 = QUAD_V3;

	// Transform all vertices' positions and normals by the vector
	// (there's probably a more efficient way to do this)
	TransformVectorByMatrix(&v0.Position, _node.tfLocal);
	TransformVectorByMatrix(&v1.Position, _node.tfLocal);
	TransformVectorByMatrix(&v2.Position, _node.tfLocal);
	TransformVectorByMatrix(&v3.Position, _node.tfLocal);
	TransformVectorByMatrix(&v0.Normal, _node.tfLocal);
	TransformVectorByMatrix(&v1.Normal, _node.tfLocal);
	TransformVectorByMatrix(&v2.Normal, _node.tfLocal);
	TransformVectorByMatrix(&v3.Normal, _node.tfLocal);

	// Add vertices to vertex vector and add 4 to vertexCount
	_vertices->push_back(v0);
	_vertices->push_back(v1);
	_vertices->push_back(v2);
	_vertices->push_back(v3);
	*_vertexCount += 4;

	// Add indices of the quad to the index vector and add 6 to indexCount
	for (int i = 0; i < 6; i++) {
		_indices->push_back(QUAD_INDICES[i] + initialVertexCount);
	}
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
	// Scaling the ring vertices' position by the node's width first
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
				// offset by 2 per face, get the parent's start vertex, and add the known index
				(face * 2) + _parentNodeFirstVertex + ringQuadIndex :
				// If index is 2 or 3 (i.e. on the child's ring),
				// offset by 2 per face, get the child's start vertex, and add the known index
				// (minus the two used to indicate this is the )
				(face * 2) + _childNodeFirstVertex + ringQuadIndex - 2
			);
		}
	}
	*_indexCount += 24;
}


