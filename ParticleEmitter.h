#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <memory>

#include "Particle.h"
#include "SimpleShader.h"

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
	ParticleEmitter(const char* _name, std::shared_ptr<SimpleVertexShader> _vertexShader, std::shared_ptr<SimplePixelShader> _pixelShader, ParticleEmitterParams _params, int _particleCount);
	~ParticleEmitter();
	void Update(float _deltaTime, float _totalTime);
	void Draw();

	ParticleEmitterParams GetParams();
	void SetParams(ParticleEmitterParams _params);

	unsigned int GetParticleCount();
	void SetParticleCount(unsigned int _particleCount);

	const char* GetName();

	// Texture Management
	void SetVertexShader(std::shared_ptr<SimpleVertexShader> _vertexShader);
	void SetPixelShader(std::shared_ptr<SimplePixelShader> _pixelShader);
	void LockSamplerState();
	void UnlockSamplerState();
	void AddTextureSRV(std::string _name, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> _srv);
	void AddSampler(std::string _name, Microsoft::WRL::ComPtr<ID3D11SamplerState> _sampler);
	void PrepareTextures();

private:
	void RebuildTextureList();

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

	// Shaders
	std::shared_ptr<SimpleVertexShader> vertexShader;
	std::shared_ptr<SimplePixelShader> pixelShader;
	
	// Texture Management
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> textureSRVs;
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11SamplerState>> samplers;
	// Returned with GetTextures so it doesn't have to be rebuilt each time
	std::vector<ID3D11ShaderResourceView*> textureList;
	// Locks the sampler state so it isn't affected by changes to the global sampler state
	bool isSamplerStateLocked;
};

