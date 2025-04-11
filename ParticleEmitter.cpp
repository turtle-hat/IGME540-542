#include "ParticleEmitter.h"

ParticleEmitter::ParticleEmitter(const char* _name, ParticleEmitterParams _params, int _particleCount)
{
	params = _params;
	particleCount = _particleCount;
	particles = new Particle[particleCount];

	// Set emitter data trackers to initial values
	firstAlive = -1;
	firstDead = -1;
	aliveCount = 0;
	emitPeriod = 1.0f / params.emitFrequency;
	lastEmitTimer = -1;

	// Texture info
	finalized = false;
	highestTextureSlotInUse = 0;
	finalGPUHandleForSRVs = (D3D12_GPU_DESCRIPTOR_HANDLE)0;
	textureSRVsBySlot[0] = (D3D12_CPU_DESCRIPTOR_HANDLE)0;

	name = _name;
}

ParticleEmitter::~ParticleEmitter()
{
	delete[] particles;
	particles = nullptr;
}

void ParticleEmitter::Update(float _deltaTime, float _totalTime)
{
}

void ParticleEmitter::Draw()
{
}

ParticleEmitterParams ParticleEmitter::GetParams()
{
	return params;
}

void ParticleEmitter::SetParams(ParticleEmitterParams _params)
{
	params = _params;
}

unsigned int ParticleEmitter::GetParticleCount()
{
	return particleCount;
}

void ParticleEmitter::SetParticleCount(unsigned int _particleCount)
{
	delete[] particles;
	particles = new Particle[particleCount];
}
