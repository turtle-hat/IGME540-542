#pragma once

#include <d3d11.h>
#include "Particle.h"
#include <DirectXMath.h>

struct ParticleEmitterParams {
	float emitFrequency;
	float particleLifetime;
	DirectX::XMFLOAT3 startPosition;
	DirectX::XMFLOAT3 startPositionVariance;
	DirectX::XMFLOAT3 startVelocity;
	DirectX::XMFLOAT3 startVelocityVariance;
	DirectX::XMFLOAT3 startColor;
	DirectX::XMFLOAT3 startColorVariance;
	DirectX::XMFLOAT3 finalColor;
	DirectX::XMFLOAT3 finalColorVariance;
	DirectX::XMFLOAT3 acceleration;
};

class ParticleEmitter
{
public:
	ParticleEmitter(const char* _name, ParticleEmitterParams _params, int _particleCount);
	~ParticleEmitter();
	void Update(float _deltaTime, float _totalTime);
	void Draw();

	ParticleEmitterParams GetParams();
	void SetParams(ParticleEmitterParams _params);

	unsigned int GetParticleCount();
	void SetParticleCount(unsigned int _particleCount);

	const char* GetName();

private:
	Particle* particles;
	// Total number of particles this emitter tracks
	unsigned int particleCount;
	// The first particle in the particles ring array that's alive
	int firstAlive;
	// The first particle in the particles ring array that's dead
	int firstDead;
	// The number of particles in this emitter that are alive
	int aliveCount;

	// Bundles emitter-specific parameters into a single structure for ease of creation
	ParticleEmitterParams params;
	float emitPeriod;
	float lastEmitTimer;

	// Name for UI
	const char* name;
};

