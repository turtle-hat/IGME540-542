#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <memory>

#include "Particle.h"
#include "Camera.h"
#include "Material.h"
#include "Transform.h"

struct ParticleEmitterParams {
	float emitFrequency;
	float lifetime;
	DirectX::XMFLOAT3 acceleration;
	DirectX::XMFLOAT3 startPositionOffset;
	DirectX::XMFLOAT3 startPositionVariance;
	DirectX::XMFLOAT3 startVelocity;
	DirectX::XMFLOAT3 startVelocityVariance;
	DirectX::XMFLOAT4 startColor;
	DirectX::XMFLOAT4 startColorVariance;
	DirectX::XMFLOAT4 finalColor;
	DirectX::XMFLOAT4 finalColorVariance;
};

class ParticleEmitter
{
public:
	ParticleEmitter(const char* _name, std::shared_ptr<Material> _material, ParticleEmitterParams _params, int _particleCount, std::shared_ptr<Transform> _transform);
	ParticleEmitter(const char* _name, std::shared_ptr<Material> _material, ParticleEmitterParams _params, int _particleCount);
	~ParticleEmitter();
	void Update(float _deltaTime, float _totalTime);
	void Draw(std::shared_ptr<Camera> _camera, float _totalTime);

	ParticleEmitterParams GetParams();
	void SetParams(ParticleEmitterParams _params);

	unsigned int GetParticleCount();
	void SetParticleCount(unsigned int _particleCount);

	std::shared_ptr<Material> GetMaterial();
	void SetMaterial(std::shared_ptr<Material> _material);

	std::shared_ptr<Transform> GetTransform();

	const char* GetName();

private:
	void UpdateParticle(float _totalTime, int index);
	void EmitParticle(float _totalTime);
	void RebuildDataBuffers();

	Particle* particles;
	// Total number of particles this emitter tracks
	unsigned int particleCount;
	// The first particle in the particles ring array that's alive
	int firstAlive;
	// The first particle in the particles ring array that's dead
	int firstDead;
	// The number of particles in this emitter that are alive
	int aliveCount;
	// Tracks when the last particle was emitted
	float lastEmitTimer;
	// 
	float emitPeriod;

	// Bundles emitter-specific parameters into a single structure for ease of creation
	ParticleEmitterParams params;

	// Name for UI
	const char* name;

	// GPU buffer references
	Microsoft::WRL::ComPtr<ID3D11Buffer> particleDataBuffer;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> particleDataSRV;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

	// Transform for positional information and material for 
	// Only parts of their functionality is used
	std::shared_ptr<Transform> transform;
	std::shared_ptr<Material> material;
};

