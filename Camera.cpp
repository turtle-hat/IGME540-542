#include "Camera.h"

using namespace DirectX;

/// <summary>
/// Constructs a new perspective Camera. Defaults:
/// - Near Clip Plane: 0.01f
/// - Far Clip Plane: 1000.0f
/// - Move Speed: 5.0f
/// - Look Speed: 2.0f
/// - Field of View: pi/2 radians
/// - Orthographic Width: 10.0f
/// </summary>
/// <param name="_name">The internal name for the Camera</param>
/// <param name="_transform">The Camera's Transform object</param>
/// <param name="_aspect">The aspect ratio of the Camera, usually that of the Window</param>
Camera::Camera(const char* _name, std::shared_ptr<Transform> _transform, float _aspect) :
	fov(XM_PIDIV2),
	orthoWidth(10.0f),
	isOrthographic(false),
	nearDist(0.01f),
	farDist(1000.0f),
	moveSpeed(5.0f),
	lookSpeed(2.0f)
{
	name = _name;
	transform = _transform;
	aspect = _aspect;

	UpdateViewMatrix();
	UpdateProjectionMatrix();
}

/// <summary>
/// Constructs a new perspective Camera. Defaults:
/// - Near Clip Plane: 0.01f
/// - Far Clip Plane: 1000.0f
/// - Move Speed: 5.0f
/// - Look Speed: 2.0f
/// - Orthographic Width: 10.0f
/// </summary>
/// <param name="_name">The internal name for the Camera</param>
/// <param name="_transform">The Camera's Transform object</param>
/// <param name="_aspect">The aspect ratio of the Camera, usually that of the Window</param>
/// <param name="_fov">The Camera's field of view in radians</param>
Camera::Camera(const char* _name, std::shared_ptr<Transform> _transform, float _aspect, float _fov) :
	orthoWidth(10.0f),
	isOrthographic(false),
	nearDist(0.01f),
	farDist(1000.0f),
	moveSpeed(5.0f),
	lookSpeed(2.0f)
{
	name = _name;
	transform = _transform;
	aspect = _aspect;
	fov = _fov;

	UpdateViewMatrix();
	UpdateProjectionMatrix();
}

/// <summary>
/// Constructs a new orthographic Camera. Defaults:
/// - Near Clip Plane: 0.01f
/// - Far Clip Plane: 1000.0f
/// - Move Speed: 5.0f
/// - Look Speed: 2.0f
/// - Field of View: pi/2 radians
/// - Orthographic Width: 10.0f
/// </summary>
/// <param name="_name">The internal name for the Camera</param>
/// <param name="_transform">The Camera's Transform object</param>
/// <param name="_aspect">The aspect ratio of the Camera, usually that of the Window</param>
/// <param name="_isOrthographic">Whether the camera uses an orthographic projection instead of a perspective projection</param>
Camera::Camera(const char* _name, std::shared_ptr<Transform> _transform, float _aspect, bool _isOrthographic) :
	fov(XM_PIDIV2),
	orthoWidth(10.0f),
	nearDist(0.01f),
	farDist(1000.0f),
	moveSpeed(5.0f),
	lookSpeed(2.0f)
{
	name = _name;
	transform = _transform;
	aspect = _aspect;
	isOrthographic = _isOrthographic;

	UpdateViewMatrix();
	UpdateProjectionMatrix();
}

/// <summary>
/// Constructs a new orthographic Camera. Defaults:
/// - Near Clip Plane: 0.01f
/// - Far Clip Plane: 1000.0f
/// - Move Speed: 5.0f
/// - Look Speed: 2.0f
/// - Field of View: pi/2 radians
/// </summary>
/// <param name="_name">The internal name for the Camera</param>
/// <param name="_transform">The Camera's Transform object</param>
/// <param name="_aspect">The aspect ratio of the Camera, usually that of the Window</param>
/// <param name="_isOrthographic">Whether the camera uses an orthographic projection instead of a perspective projection</param>
/// <param name="_orthoWidth">The width of the Camera's view box, in world units</param>
Camera::Camera(const char* _name, std::shared_ptr<Transform> _transform, float _aspect, bool _isOrthographic, float _orthoWidth) :
	fov(XM_PIDIV2),
	nearDist(0.01f),
	farDist(1000.0f),
	moveSpeed(5.0f),
	lookSpeed(2.0f)
{
	name = _name;
	transform = _transform;
	aspect = _aspect;
	// I would have just set isOrthographic to true automatically but the function
	// signatures needed to be different to overload the constructor
	isOrthographic = _isOrthographic;
	orthoWidth = _orthoWidth;

	UpdateViewMatrix();
	UpdateProjectionMatrix();
}

/// <summary>
/// Gets the Camera's Transform object
/// </summary>
/// <returns>The Camera's Transform object</returns>
std::shared_ptr<Transform> Camera::GetTransform()
{
	return transform;
}

/// <summary>
/// Gets the Camera's internal name
/// </summary>
/// <returns>The Camera's internal name</returns>
const char* Camera::GetName()
{
	return name;
}

/// <summary>
/// Gets the Camera's field of view
/// </summary>
/// <returns>The Camera's field of view, in radians</returns>
float Camera::GetFov()
{
	return fov;
}

/// <summary>
/// Gets the Camera's orthographic width
/// </summary>
/// <returns>The width of the Camera's view box, in world units</returns>
float Camera::GetOrthographicWidth()
{
	return orthoWidth;
}

/// <summary>
/// Gets the Camera's movement speed
/// </summary>
/// <returns>The Camera's movement speed, in units per second</returns>
float Camera::GetMoveSpeed()
{
	return moveSpeed;
}

/// <summary>
/// Gets the Camera's look speed
/// </summary>
/// <returns>The Camera's look speed, in milliradians per pixel of mouse movement</returns>
float Camera::GetLookSpeed()
{
	return lookSpeed;
}

/// <summary>
/// Gets the distance of the Camera's near clip plane
/// </summary>
/// <returns>The distance of the Camera's near clip plane</returns>
float Camera::GetNearClip()
{
	return nearDist;
}

/// <summary>
/// Gets the distance of the Camera's far clip plane
/// </summary>
/// <returns>The distance of the Camera's far clip plane</returns>
float Camera::GetFarClip()
{
	return farDist;
}

/// <summary>
/// Gets the Camera's projection mode
/// </summary>
/// <returns>Returns true if the Camera is orthographic and false if the Camera is perspective</returns>
bool Camera::GetProjectionMode()
{
	return isOrthographic;
}

/// <summary>
/// Gets the Camera's View matrix
/// </summary>
/// <returns>The Camera's View matrix</returns>
XMFLOAT4X4 Camera::GetViewMatrix()
{
	return view;
}

/// <summary>
/// Gets the Camera's Projection matrix
/// </summary>
/// <returns>The Camera's Projection matrix</returns>
XMFLOAT4X4 Camera::GetProjectionMatrix()
{
	return projection;
}

/// <summary>
/// Sets the Camera's aspect ratio, rebuilding its projection matrix
/// </summary>
/// <param name="_aspect">The Camera's new aspect ratio</param>
void Camera::SetAspect(float _aspect)
{
	aspect = _aspect;
	UpdateProjectionMatrix();
}

/// <summary>
/// Sets the Camera's field of view, rebuilding its projection matrix if it's in perspective mode
/// </summary>
/// <param name="_aspect">The Camera's new field of view, in radians</param>
void Camera::SetFov(float _fov)
{
	fov = _fov;
	if (!isOrthographic) {
		UpdateProjectionMatrix();
	}
}

/// <summary>
/// Sets the Camera's orthographic width, rebuilding its projection matrix if it's in orthographic mode
/// </summary>
/// <param name="_orthoWidth">The new width of the Camera's view box, in world units</param>
void Camera::SetOrthographicWidth(float _orthoWidth)
{
	orthoWidth = _orthoWidth;
	if (isOrthographic) {
		UpdateProjectionMatrix();
	}
}

/// <summary>
/// Sets the Camera's movement speed
/// </summary>
/// <param name="_speed">The Camera's new movement speed, in units per second</param>
void Camera::SetMoveSpeed(float _speed)
{
	moveSpeed = _speed;
}

/// <summary>
/// Sets the Camera's look speed
/// </summary>
/// <param name="_speed">The Camera's new look speed, in milliradians per pixel of mouse movement</param>
void Camera::SetLookSpeed(float _speed)
{
	lookSpeed = _speed;
}

/// <summary>
/// Sets the distance of the Camera's near clip plane 
/// </summary>
/// <param name="_distance">The distance of the Camera's near clip plane</param>
void Camera::SetNearClip(float _distance)
{
	nearDist = _distance;
	UpdateProjectionMatrix();
}

/// <summary>
/// Sets the distance of the Camera's far clip plane 
/// </summary>
/// <param name="_distance">The distance of the Camera's far clip plane</param>
void Camera::SetFarClip(float _distance)
{
	farDist = _distance;
	UpdateProjectionMatrix();
}

/// <summary>
/// Sets the Camera's projection mode
/// </summary>
/// <param name="_isOrthographic">True sets the Camera to orthographic mode, false sets the Camera to perspective mode</param>
void Camera::SetProjectionMode(bool _isOrthographic)
{
	if (isOrthographic != _isOrthographic) {
		isOrthographic = _isOrthographic;
		UpdateProjectionMatrix();
	}
}

/// <summary>
/// Toggles the Camera's projection mode
/// </summary>
void Camera::ToggleProjectionMode()
{
	isOrthographic = !isOrthographic;
	UpdateProjectionMatrix();
}

/// <summary>
/// Handles Camera movement.
/// </summary>
/// <param name="dt">Delta time, the amount of time that has elapsed since the last frame</param>
void Camera::Update(float dt)
{
	// Process keyboard input

	// Stores relative movement
	XMFLOAT3 movementRel(0.0f, 0.0f, 0.0f);
	// Stores absolute movement along the Y axis
	float movementYAbs = 0.0f;

	if (Input::KeyDown('D') && !Input::KeyDown('A')) { movementRel.x = moveSpeed * dt; }
	else if (Input::KeyDown('A') && !Input::KeyDown('D')) { movementRel.x = -moveSpeed * dt; }

	if (Input::KeyDown('W') && !Input::KeyDown('S')) { movementRel.z = moveSpeed * dt; }
	else if (Input::KeyDown('S') && !Input::KeyDown('W')) { movementRel.z = -moveSpeed * dt; }

	// Two different vertical movement styles:
	// - E and Q move relative to camera
	// - Space and Shift move relative to the world, like in Minecraft
	if (Input::KeyDown('E') && !Input::KeyDown('Q')) { movementRel.y = moveSpeed * dt; }
	else if (Input::KeyDown('Q') && !Input::KeyDown('E')) { movementRel.y = -moveSpeed * dt; }

	if (Input::KeyDown(VK_SPACE) && !Input::KeyDown(VK_SHIFT)) { movementYAbs = moveSpeed * dt; }
	else if (Input::KeyDown(VK_SHIFT) && !Input::KeyDown(VK_SPACE)) { movementYAbs = -moveSpeed * dt; }

	// If movement was detected, move the camera's transform
	if (movementRel.x != 0.0f || movementRel.y != 0.0f || movementRel.z != 0.0f) {
		transform->MoveRelative(movementRel);
	}
	if (movementYAbs != 0.0f) {
		transform->MoveAbsolute(0.0f, movementYAbs, 0.0f);
	}

	// Process mouse input if mouse is down
	if (Input::MouseLeftDown()) {
		// Find mouse rotation amount in radians
		XMFLOAT2 mouseRot(
			// Get mouse input since last frame (px) and multiply it by lookSpeed (mrad/px), then divide by 1000 (mrad/rad)
			Input::GetMouseXDelta() * (lookSpeed / 1000.0f),
			Input::GetMouseYDelta() * (lookSpeed / 1000.0f)
		);

		// Rotate the Camera
		transform->Rotate(mouseRot.y, mouseRot.x, 0.0f);

		// Clamp the final rotation within pi/2 to -pi/2
		XMFLOAT3 finalRot = transform->GetRotation();
		if (finalRot.x > XM_PIDIV2 - 0.001f) {
			transform->SetRotation(XM_PIDIV2 - 0.001f, finalRot.y, finalRot.z);
		} else if (finalRot.x < -XM_PIDIV2 + 0.001f) {
			transform->SetRotation(-XM_PIDIV2 + 0.001f, finalRot.y, finalRot.z);
		}
	}

	UpdateViewMatrix();
}

/// <summary>
/// Updates the Camera's view matrix.
/// Called once per frame in Update()
/// </summary>
void Camera::UpdateViewMatrix()
{
	XMFLOAT3 eyePos = transform->GetPosition();
	XMFLOAT3 lookDir = transform->GetForward();
	XMFLOAT3 worldUp(0.0f, 1.0f, 0.0f);

	XMStoreFloat4x4(&view, XMMatrixLookToLH(
		XMLoadFloat3(&eyePos),
		XMLoadFloat3(&lookDir),
		XMLoadFloat3(&worldUp)
	));
}

/// <summary>
/// Updates the Camera's projection matrix.
/// Called once per frame when the Camera's aspect ratio or FOV changes
/// </summary>
void Camera::UpdateProjectionMatrix()
{
	// Sets the Camera's projection matrix based on whether it's orthographic or not
	XMStoreFloat4x4(&projection, isOrthographic ?
		XMMatrixOrthographicLH(orthoWidth, orthoWidth / aspect, nearDist, farDist) :
		XMMatrixPerspectiveFovLH(fov, aspect, nearDist, farDist)
	);
}
