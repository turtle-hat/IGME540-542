#pragma once

#include <DirectXMath.h>

// Represents a transformation in 3D space
class Transform
{
public:
	// Constructor
	Transform();

	// Getters
	DirectX::XMFLOAT3 GetPosition();
	DirectX::XMFLOAT3 GetRotation();
	DirectX::XMFLOAT3 GetScale();
	DirectX::XMFLOAT4X4 GetWorld();
	DirectX::XMFLOAT4X4 GetWorldInverseTranspose();
	DirectX::XMFLOAT3 GetForward();
	DirectX::XMFLOAT3 GetRight();
	DirectX::XMFLOAT3 GetUp();

	// Setters
	void SetPosition(float _x, float _y, float _z);
	void SetPosition(DirectX::XMFLOAT3 _xyz);
	void SetRotation(float _pitch, float _yaw, float _roll);
	void SetRotation(DirectX::XMFLOAT3 _pitchYawRoll);
	void SetScale(float _x, float _y, float _z);
	void SetScale(DirectX::XMFLOAT3 _xyz);

	// Mutators
	void MoveAbsolute(float _x, float _y, float _z);
	void MoveAbsolute(DirectX::XMFLOAT3 _xyz);
	void MoveRelative(float _x, float _y, float _z);
	void MoveRelative(DirectX::XMFLOAT3 _xyz);
	void Rotate(float _pitch, float _yaw, float _roll);
	void Rotate(DirectX::XMFLOAT3 _pitchYawRoll);
	void Scale(float _x, float _y, float _z);
	void Scale(DirectX::XMFLOAT3 _xyz);

private:
	// Translation
	DirectX::XMFLOAT3 position;
	// Rotation
	DirectX::XMFLOAT3 rotation;
	// Scale
	DirectX::XMFLOAT3 scale;
	// Final World Matrix
	DirectX::XMFLOAT4X4 world;
	// Final Inverse Transpose World Matrix
	DirectX::XMFLOAT4X4 worldInverseTranspose;
	// Forward, right, and up vectors for the Transform
	DirectX::XMFLOAT3 forward;
	DirectX::XMFLOAT3 right;
	DirectX::XMFLOAT3 up;

	// Whether World and World Inverse Transpose matrices needs to be rebuilt
	bool areMatricesDirty;
	// Whether Forward, Right, and Up vertices need to be rebuilt
	bool areVerticesDirty;

	// Rebuilds World and WorldInverseTranspose
	void RebuildMatrices();
	// Rebuilds Forward, Right, and Up vertices
	void RebuildVertices();
};
