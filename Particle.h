#pragma once
#include <DirectXMath.h>

struct Particle {
	float emitTime;
	DirectX::XMFLOAT3 startPosition;
	DirectX::XMFLOAT3 startVelocity;
	DirectX::XMFLOAT4 startColor;
	DirectX::XMFLOAT4 finalColor;
};