#pragma once

#include <memory>
#include "Transform.h"
#include "Material.h"
#include "Mesh.h"
#include "Camera.h"

class Entity
{
public:
	// Constructor
	Entity(const char* _name, std::shared_ptr<Mesh> _mesh, std::shared_ptr<Material> _material);
	Entity(const char* _name, std::shared_ptr<Mesh> _mesh, std::shared_ptr<Material> _material, std::shared_ptr<Transform> _transform);

	// Getters
	std::shared_ptr<Mesh> GetMesh();
	std::shared_ptr<Material> GetMaterial();
	std::shared_ptr<Transform> GetTransform();
	const char* GetName();

	// Setters
	void SetMesh(std::shared_ptr<Mesh> _mesh);
	void SetMaterial(std::shared_ptr<Material> _material);

private:
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<Transform> transform;
	std::shared_ptr<Material> material;

	// Name for UI
	const char* name;
};
