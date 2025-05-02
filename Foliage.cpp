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

	// Use whichever axis (Z or X) has the lowest component as second basis
	// and orthonormalize it with the up vector;
	// then calculate the other.
	if (abs(rootUp.z) <= abs(rootUp.x)) {
		// XMVECTOR of the forward vector
		XMVECTOR vecRootForward = XMLoadFloat3(&rootForward);

		// If there is a z component originally, orthonormalize it to ensure
		// the forward basis is orthogonal to the up vector
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
	else {
		// XMVECTOR of the right vector
		XMVECTOR vecRootRight = XMLoadFloat3(&rootRight);

		// If there is an x component originally, orthonormalize it to ensure
		// the right basis is orthogonal to the up vector
		if (abs(rootUp.x) > 0.0f) {
			// Gram-Schmidt orthonormalize
			vecRootRight = XMVector3Normalize(
				vecRootRight - vecRootUp * XMVector3Dot(vecRootUp, vecRootRight));
			
			// Store right basis
			XMStoreFloat3(&rootRight, vecRootRight);
		}

		XMVECTOR cross = XMVector3Cross(
			vecRootRight,
			vecRootUp
		);
		// Calculate forward basis as the cross product of the other two and store
		XMStoreFloat3(&rootForward, cross);
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
		0.0f
	};
	
	nodes.push_back(root);

	AddNodeQuadVertices(&vertices, &vertexCount, &indices, &indexCount, root);

	mesh = make_shared<Mesh>("M_Foliage_Generated", vertices.data(), vertexCount, indices.data(), indexCount);
}

void Foliage::AddNodeRingVertices(std::vector<Vertex>* _vertices, unsigned int* _vertexCount, std::vector<UINT>* _indices, unsigned int* _indexCount, const FoliageNode& _node)
{
}

void Foliage::AddNodeQuadVertices(std::vector<Vertex>* _vertices, unsigned int* _vertexCount, std::vector<UINT>* _indices, unsigned int* _indexCount, const FoliageNode& _node)
{
	// Get new quad vertices
	Vertex v1 = QUAD_V1;
	Vertex v2 = QUAD_V2;
	Vertex v3 = QUAD_V3;
	Vertex v4 = QUAD_V4;

	// Transform all vertices' positions and normals by the vector
	// (there's probably a more efficient way to do this)
	TransformVectorByMatrix(&v1.Position, _node.tfLocal);
	TransformVectorByMatrix(&v2.Position, _node.tfLocal);
	TransformVectorByMatrix(&v3.Position, _node.tfLocal);
	TransformVectorByMatrix(&v4.Position, _node.tfLocal);
	TransformVectorByMatrix(&v1.Normal, _node.tfLocal);
	TransformVectorByMatrix(&v2.Normal, _node.tfLocal);
	TransformVectorByMatrix(&v3.Normal, _node.tfLocal);
	TransformVectorByMatrix(&v4.Normal, _node.tfLocal);

	// Add vertices to vertex vector and add 4 to vertexCount
	_vertices->push_back(v1);
	_vertices->push_back(v2);
	_vertices->push_back(v3);
	_vertices->push_back(v4);
	*_vertexCount += 4;

	// Add indices of the quad to the index vector and add 6 to indexCount
	for (int i = 0; i < 6; i++) {
		_indices->push_back(QUAD_INDICES[i] + *_indexCount);
	}
	*_indexCount += 6;
}

void Foliage::TransformVectorByMatrix(DirectX::XMFLOAT3* _vector, DirectX::XMFLOAT4X4 _matrix)
{
	XMStoreFloat3(_vector, XMVector3Transform(XMLoadFloat3(_vector), XMLoadFloat4x4(&_matrix)));
}


