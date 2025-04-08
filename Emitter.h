#pragma once
#include "Particle.h"
#include <DirectXMath.h>

struct EmitterParams {
	float emitRate;
	DirectX::XMFLOAT3 startPosition;
	DirectX::XMFLOAT3 startPositionVariance;
	DirectX::XMFLOAT3 startVelocity;
	DirectX::XMFLOAT3 startVelocityVariance;
	DirectX::XMFLOAT3 startAcceleration;
	DirectX::XMFLOAT3 startAccelerationVariance;
	DirectX::XMFLOAT3 startColorTint;
	DirectX::XMFLOAT3 startColorTintVariance;
	DirectX::XMFLOAT3 finalColorTint;
	DirectX::XMFLOAT3 finalColorTintVariance;

};

class Emitter
{
public:
	void Update(float _deltaTime);

private:
	Particle particles[256];
	int firstAlive;
	int firstDead;
	int aliveCount;

	float lastEmitTimer;
};

