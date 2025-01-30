#pragma once

#include <memory>
#include <DirectXMath.h>
#include "Input.h"
#include "Transform.h"

class Camera
{
public:
	// Constructors
	Camera(const char* _name, std::shared_ptr<Transform> _transform, float _aspect);
	Camera(const char* _name, std::shared_ptr<Transform> _transform, float _aspect, float _fov);
	Camera(const char* _name, std::shared_ptr<Transform> _transform, float _aspect, bool _isOrthographic);
	Camera(const char* _name, std::shared_ptr<Transform> _transform, float _aspect, bool _isOrthographic, float _orthoWidth);

	// Getters
	std::shared_ptr<Transform> GetTransform();
	const char* GetName();
	float GetFov();
	float GetOrthographicWidth();
	float GetMoveSpeed();
	float GetLookSpeed();
	float GetNearClip();
	float GetFarClip();
	bool GetProjectionMode();
	DirectX::XMFLOAT4X4 GetViewMatrix();
	DirectX::XMFLOAT4X4 GetProjectionMatrix();

	// Setters
	void SetAspect(float _aspect);
	void SetFov(float _fov);
	void SetOrthographicWidth(float _orthoWidth);
	void SetMoveSpeed(float _speed);
	void SetLookSpeed(float _speed);
	void SetNearClip(float _distance);
	void SetFarClip(float _distance);
	void SetProjectionMode(bool _isOrthographic);
	void ToggleProjectionMode();

	void Update(float dt);

private:
	std::shared_ptr<Transform> transform;
	DirectX::XMFLOAT4X4 view;
	DirectX::XMFLOAT4X4 projection;

	// Name for UI
	const char* name;
	// Aspect Ratio
	float aspect;
	// (Perspective Camera Only) Field of View, in radians
	float fov;
	// (Orthographic Camera Only) Width of the orthographic camera, in units
	float orthoWidth;
	// Whether the camera uses an orthographic projection instead of a perspective projection
	bool isOrthographic;
	// Near and far clip planes
	float nearDist;
	float farDist;
	// Move speed in units per second
	float moveSpeed;
	// Move speed in milliradians per pixel
	float lookSpeed;

	// Called in Update()
	void UpdateViewMatrix();
	// Called whenever aspect ratio, FOV, orthographic width, near clip plane, or far clip plane are changed
	void UpdateProjectionMatrix();
};

