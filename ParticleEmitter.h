#pragma once
#include <d3d12.h>
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

	// Texture Management
	void AddTexture(D3D12_CPU_DESCRIPTOR_HANDLE _srv, unsigned int _slot);
	void FinalizeTextures();

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

	// Texture Data
	bool finalized;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pso;
	D3D12_CPU_DESCRIPTOR_HANDLE textureSRVsBySlot[128]{};
	unsigned int highestTextureSlotInUse;
	D3D12_GPU_DESCRIPTOR_HANDLE finalGPUHandleForSRVs;

	// Name for UI
	const char* name;
};

