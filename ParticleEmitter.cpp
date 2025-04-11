#include "ParticleEmitter.h"

ParticleEmitter::ParticleEmitter(const char* _name, std::shared_ptr<SimpleVertexShader> _vertexShader, std::shared_ptr<SimplePixelShader> _pixelShader, ParticleEmitterParams _params, int _particleCount)
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

const char* ParticleEmitter::GetName()
{
	return name;
}

void ParticleEmitter::SetVertexShader(std::shared_ptr<SimpleVertexShader> _vertexShader)
{
	vertexShader = _vertexShader;
}

void ParticleEmitter::SetPixelShader(std::shared_ptr<SimplePixelShader> _pixelShader)
{
	pixelShader = _pixelShader;
}

void ParticleEmitter::LockSamplerState()
{
	isSamplerStateLocked = true;
}

void ParticleEmitter::UnlockSamplerState()
{
	isSamplerStateLocked = false;
}

void ParticleEmitter::AddTextureSRV(std::string _name, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> _srv)
{
	// If SRV with same name was found, release it
	if (textureSRVs.find(_name) != textureSRVs.end()) {
		textureSRVs[_name] = nullptr;
		textureSRVs.erase(_name);
		RebuildTextureList();
	}
	textureSRVs.insert({ _name, _srv });
	textureList.push_back(_srv.Get());
}

void ParticleEmitter::AddSampler(std::string _name, Microsoft::WRL::ComPtr<ID3D11SamplerState> _sampler)
{
	if (!isSamplerStateLocked) {
		// If sampler with same name was found, release it first
		if (samplers.find(_name) != samplers.end()) {
			samplers[_name] = nullptr;
			samplers.erase(_name);
		}
		samplers.insert({ _name, _sampler });
	}
}

void ParticleEmitter::PrepareTextures()
{
	for (auto& t : textureSRVs) {
		pixelShader->SetShaderResourceView(t.first.c_str(), t.second);
	}
	for (auto& s : samplers) {
		pixelShader->SetSamplerState(s.first.c_str(), s.second);
	}
}

void ParticleEmitter::RebuildTextureList()
{
	// If textures have been removed or updated, rebuild textureList
	textureList.clear();
	// https://stackoverflow.com/a/8484055
	for (auto srv : textureSRVs) {
		textureList.push_back(srv.second.Get());
	}
}

