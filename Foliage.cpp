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

const char* Foliage::GetName()
{
	return name;
}

void Foliage::SetBranchMaterial(std::shared_ptr<Material> _material)
{
	branchMaterial = _material;
}

void Foliage::SetLeafMaterial(std::shared_ptr<Material> _material)
{
	leafMaterial = _material;
}

void Foliage::GenerateBranchMesh()
{
	vector<Vertex> vertices;	// Generated vertices
	vector<UINT> indices;		// Generated indices
	int vertexCounter = 0;		// Counter for vertices
	int indexCounter = 0;		// Counter for indices

	stack<FoliageNode> nodesToBuild;	// Stores the nodes in the list that can are still valid to build off of

	// Normalize growth direction
	XMFLOAT3 growthDirection;
	XMStoreFloat3(&growthDirection, XMVector3Normalize(XMLoadFloat3(&params.growthDirection)));

	// Create root node
	nodes.push_back({
		DirectX::XMFLOAT4X4(
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, 1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f
		),
		0,
		0,
		0.0f
	});


}
