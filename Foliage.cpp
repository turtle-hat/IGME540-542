#include "Foliage.h"

using namespace std;
using namespace DirectX;

Foliage::Foliage(const char* _name, std::shared_ptr<Material> _branchMaterial, std::shared_ptr<Material> _leafMaterial, FoliageParams _params, std::shared_ptr<Transform> _transform)
{
	name = _name;
	branchMaterial = _branchMaterial;
	leafMaterial = _leafMaterial;
	params = _params;
	transform = _transform;

	GenerateMesh();
}

Foliage::Foliage(const char* _name, std::shared_ptr<Material> _branchMaterial, std::shared_ptr<Material> _leafMaterial, FoliageParams _params)
{
	name = _name;
	branchMaterial = _branchMaterial;
	leafMaterial = _leafMaterial;
	params = _params;
	transform = make_shared<Transform>();
	
	GenerateMesh();
}

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

void Foliage::RegenerateMesh()
{
}

void Foliage::GenerateMesh()
{
	
}
