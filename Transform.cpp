#include "Transform.h"

using namespace DirectX;

/// <summary>
/// Constructs a Transformation with no translation, no rotation, normal scale
/// </summary>
Transform::Transform() :
	position(0.0f, 0.0f, 0.0f),
	rotation(0.0f, 0.0f, 0.0f),
	scale(1.0f, 1.0f, 1.0f),
	right(1.0f, 0.0f, 0.0f),
	up(0.0f, 1.0f, 0.0f),
	forward(0.0f, 0.0f, 1.0f)
{
	XMStoreFloat4x4(&world, XMMatrixIdentity());
	XMStoreFloat4x4(&worldInverseTranspose,
		XMMatrixInverse(0, XMMatrixTranspose(XMMatrixIdentity()))
	);
	areMatricesDirty = false;
	areVerticesDirty = false;
}

/// <summary>
/// Gets the Transform's position
/// </summary>
/// <returns>The Transform's x, y, and z positions</returns>
DirectX::XMFLOAT3 Transform::GetPosition()
{
	return position;
}

/// <summary>
/// Gets the Transform's rotation
/// </summary>
/// <returns>The Transform's rotations about the X, Y, and Z axes</returns>
DirectX::XMFLOAT3 Transform::GetRotation()
{
	return rotation;
}

/// <summary>
/// Gets the Transform's scale
/// </summary>
/// <returns>The Transform's scale along the x, y, and z axes</returns>
DirectX::XMFLOAT3 Transform::GetScale()
{
	return scale;
}

/// <summary>
/// Gets the Transform's world matrix.
/// Rebuilds matrices if they have been mutated
/// </summary>
/// <returns>The world matrix representing the Transform</returns>
DirectX::XMFLOAT4X4 Transform::GetWorld()
{
	if (areMatricesDirty) {
		RebuildMatrices();
	}
	return world;
}

/// <summary>
/// Sets the Transform's world inverse transpose matrix.
/// Rebuilds matrices if they have been mutated
/// </summary>
/// <returns>The transposed then inverted world matrix representing the Transform</returns>
DirectX::XMFLOAT4X4 Transform::GetWorldInverseTranspose()
{
	if (areMatricesDirty) {
		RebuildMatrices();
	}
	return worldInverseTranspose;
}

/// <summary>
/// Gets the Transform's forward vector
/// </summary>
/// <returns>The Transform's forward vector</returns>
DirectX::XMFLOAT3 Transform::GetForward()
{
	if (areVerticesDirty) {
		RebuildVertices();
	}
	return forward;
}

/// <summary>
/// Gets the Transform's right vector
/// </summary>
/// <returns>The Transform's right vector</returns>
DirectX::XMFLOAT3 Transform::GetRight()
{
	if (areVerticesDirty) {
		RebuildVertices();
	}
	return right;
}

/// <summary>
/// Gets the Transform's up vector
/// </summary>
/// <returns>The Transform's up vector</returns>
DirectX::XMFLOAT3 Transform::GetUp()
{
	if (areVerticesDirty) {
		RebuildVertices();
	}
	return up;
}

/// <summary>
/// Sets the Transform's position
/// </summary>
/// <param name="_x">The Transform's new X position</param>
/// <param name="_y">The Transform's new Y position</param>
/// <param name="_z">The Transform's new Z position</param>
void Transform::SetPosition(float _x, float _y, float _z)
{
	position = XMFLOAT3(_x, _y, _z);
	areMatricesDirty = true;
}

/// <summary>
/// Sets the Transform's position
/// </summary>
/// <param name="_xyz">The Transform's new X, Y, and Z positions</param>
void Transform::SetPosition(DirectX::XMFLOAT3 _xyz)
{
	position = _xyz;
	areMatricesDirty = true;
}

/// <summary>
/// Sets the Transform's rotation
/// </summary>
/// <param name="_pitch">The Transform's new rotation about the X axis</param>
/// <param name="_yaw">The Transform's new rotation about the Y axis</param>
/// <param name="_roll">The Transform's new rotation about the Z axis</param>
void Transform::SetRotation(float _pitch, float _yaw, float _roll)
{
	rotation = XMFLOAT3(_pitch, _yaw, _roll);
	areMatricesDirty = true;
	areVerticesDirty = true;
}

/// <summary>
/// Sets the Transform's rotation
/// </summary>
/// <param name="_pitchYawRoll">The Transform's new rotation about the X, Y, and Z axes</param>
void Transform::SetRotation(DirectX::XMFLOAT3 _pitchYawRoll)
{
	rotation = _pitchYawRoll;
	areMatricesDirty = true;
	areVerticesDirty = true;
}

/// <summary>
/// Sets the Transform's scale
/// </summary>
/// <param name="_x">The Transform's new scale about the X axis</param>
/// <param name="_y">The Transform's new scale about the Y axis</param>
/// <param name="_z">The Transform's new scale about the Z axis</param>
void Transform::SetScale(float _x, float _y, float _z)
{
	scale = XMFLOAT3(_x, _y, _z);
	areMatricesDirty = true;
}

/// <summary>
/// Sets the Transform's scale
/// </summary>
/// <param name="_xyz">The Transform's new scale about the X, Y, and Z axes</param>
void Transform::SetScale(DirectX::XMFLOAT3 _xyz)
{
	scale = _xyz;
	areMatricesDirty = true;
}

/// <summary>
/// Applies a translation to the Transform relative to the world
/// </summary>
/// <param name="_x">How far to translate the Transform along the world's X axis</param>
/// <param name="_y">How far to translate the Transform along the world's Y axis</param>
/// <param name="_z">How far to translate the Transform along the world's Z axis</param>
void Transform::MoveAbsolute(float _x, float _y, float _z)
{
	// Build translation vector and pass to overload
	XMFLOAT3 translation(_x, _y, _z);
	MoveAbsolute(translation);
}

/// <summary>
/// Applies a translation to the Transform relative to the world
/// </summary>
/// <param name="_xyz">How far to translate the Transform along the X, Y, and Z axes</param>
void Transform::MoveAbsolute(DirectX::XMFLOAT3 _xyz)
{
	// Add translation vector to position and store the result back
	XMStoreFloat3(&position, XMLoadFloat3(&position) + XMLoadFloat3(&_xyz));
	areMatricesDirty = true;
}

/// <summary>
/// Applies a translation to the Transform relative to itself
/// </summary>
/// <param name="_x">How far to translate the Transform along its X axis</param>
/// <param name="_y">How far to translate the Transform along its Y axis</param>
/// <param name="_z">How far to translate the Transform along its Z axis</param>
void Transform::MoveRelative(float _x, float _y, float _z)
{
	XMFLOAT3 translation(_x, _y, _z);
	MoveRelative(translation);
}

/// <summary>
/// Applies a translation to the Transform relative to itself
/// </summary>
/// <param name="_xyz">How far to translate the Transform along its X, Y, and Z axes</param>
void Transform::MoveRelative(DirectX::XMFLOAT3 _xyz)
{
	// Rotates the movement vector by the transform's rotation before adding it to the position
	XMStoreFloat3(
		&position, 
		XMLoadFloat3(&position) + XMVector3Rotate(
			XMLoadFloat3(&_xyz),
			XMQuaternionRotationRollPitchYaw(rotation.x, rotation.y, rotation.z)
		)
	);
	areMatricesDirty = true;
}

/// <summary>
/// Adds an amount to the Transform's current rotation along each world axis
/// </summary>
/// <param name="_pitch">How much to add to the Transform's rotation about the world's X axis</param>
/// <param name="_yaw">How much to add to the Transform's rotation about the world's Y axis</param>
/// <param name="_roll">How much to add to the Transform's rotation about the world's Z axis</param>
void Transform::Rotate(float _pitch, float _yaw, float _roll)
{
	// Build rotation vector and pass to overload
	XMFLOAT3 rotation(_pitch, _yaw, _roll);
	Rotate(rotation);
}

/// <summary>
/// Adds an amount to the Transform's current rotation along each world axis
/// </summary>
/// <param name="_pitchYawRoll">How much to add to the Transform's rotation about the world's X, Y, and Z axes</param>
void Transform::Rotate(DirectX::XMFLOAT3 _pitchYawRoll)
{
	// Add rotation vector to rotation and store the result back
	// (I'm not worrying about gimbal lock)
	XMStoreFloat3(&rotation, XMLoadFloat3(&rotation) + XMLoadFloat3(&_pitchYawRoll));
	areMatricesDirty = true;
	areVerticesDirty = true;
}

/// <summary>
/// Multiplies the Transform's scale by an amount along each of its axes
/// </summary>
/// <param name="_x">How much to multiply to the Transform's scale along its X axis</param>
/// <param name="_y">How much to multiply to the Transform's scale along its Y axis</param>
/// <param name="_z">How much to multiply to the Transform's scale along its Z axis</param>
void Transform::Scale(float _x, float _y, float _z)
{
	// Because DirectXMath doesn't seem to include element-wise multiplication,
	// scaling multiplication isn't performed SIMD
	scale = XMFLOAT3(
		scale.x * _x,
		scale.y * _y,
		scale.z * _z
	);
	areMatricesDirty = true;
}

/// <summary>
/// Multiplies the Transform's scale by an amount along each of its axes
/// </summary>
/// <param name="_xyz">How much to multiply to the Transform's scale along its X, Y, and Z axes</param>
void Transform::Scale(DirectX::XMFLOAT3 _xyz)
{
	// Probably more performant to repeat code here
	// rather than make new function call(?)
	scale = XMFLOAT3(
		scale.x * _xyz.x,
		scale.y * _xyz.y,
		scale.z * _xyz.z
	);
	areMatricesDirty = true;
}

/// <summary>
/// Recalculates the Transform's World and World Inverse Transpose matrices,
/// then marks them as no longer dirty
/// </summary>
void Transform::RebuildMatrices()
{
	// Multiply Scale to Rotation to Translation to calculate the new World matrix
	XMMATRIX newWorld =
		XMMatrixScalingFromVector(XMLoadFloat3(&scale)) *
		XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&rotation)) *
		XMMatrixTranslationFromVector(XMLoadFloat3(&position));
	
	// Store World matrix
	XMStoreFloat4x4(&world, newWorld);

	// Transpose and invert World matrix to calculate World Inverse Transpose and store it
	XMStoreFloat4x4(&worldInverseTranspose, 
		XMMatrixInverse(0, XMMatrixTranspose(newWorld))
	);

	// Mark matrices as clean
	areMatricesDirty = false;
}

/// <summary>
/// Recalculates the Transform's Forward, Right, and Up vertices,
/// then marks them as no longer dirty
/// </summary>
void Transform::RebuildVertices()
{
	XMVECTOR rotQuat = XMQuaternionRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
	// Set all three vertices
	XMFLOAT3 worldRight = XMFLOAT3(1.0f, 0.0f, 0.0f);
	XMFLOAT3 worldUp = XMFLOAT3(0.0f, 1.0f, 0.0f);
	XMFLOAT3 worldForward = XMFLOAT3(0.0f, 0.0f, 1.0f);
	// Rotate each vertex by the Transform's rotation values
	XMStoreFloat3(&right, XMVector3Rotate(XMLoadFloat3(&worldRight), rotQuat));
	XMStoreFloat3(&up, XMVector3Rotate(XMLoadFloat3(&worldUp), rotQuat));
	XMStoreFloat3(&forward, XMVector3Rotate(XMLoadFloat3(&worldForward), rotQuat));
	areVerticesDirty = false;
}
