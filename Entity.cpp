#include "Entity.h"

using namespace std;
using namespace DirectX;

/// <summary>
/// Constructs a new Entity at XYZ (0, 0, 0)
/// </summary>
/// <param name="_name">The internal name for the Entity</param>
/// <param name="_mesh">The mesh the Entity will be drawn with</param>
Entity::Entity(const char* _name, shared_ptr<Mesh> _mesh)
{
    // I forget how to make this constructor call the one with more parameters :P
    name = _name;
    mesh = _mesh;
    transform = make_shared<Transform>();
}

/// <summary>
/// Constructs a new Entity
/// </summary>
/// <param name="_name">The internal name for the Entity</param>
/// <param name="_mesh">The mesh the Entity will be drawn with</param>
/// <param name="_transform">The Entity's Transform object</param>
Entity::Entity(const char* _name, shared_ptr<Mesh> _mesh, shared_ptr<Transform> _transform)
{
    name = _name;
    mesh = _mesh;
    transform = _transform;
}

/// <summary>
/// Gets the Entity's Mesh
/// </summary>
/// <returns>The Entity's Mesh</returns>
std::shared_ptr<Mesh> Entity::GetMesh()
{
    return mesh;
}

/// <summary>
/// Gets the Entity's Transform
/// </summary>
/// <returns>The Entity's Transform</returns>
std::shared_ptr<Transform> Entity::GetTransform()
{
    return transform;
}

/// <summary>
/// Gets the Entity's internal name
/// </summary>
/// <returns>The Entity's internal name</returns>
const char* Entity::GetName()
{
    return name;
}

/// <summary>
/// Sets the Entity's mesh
/// </summary>
/// <param name="_mesh">A new Mesh that the Entity will use</param>
void Entity::SetMesh(std::shared_ptr<Mesh> _mesh)
{
    mesh = _mesh;
}
