#include "ParticleEmitter.h"

#include "Graphics.h"

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

	RebuildDataBuffers();
}

ParticleEmitter::~ParticleEmitter()
{
	delete[] particles;
	particles = nullptr;
}

void ParticleEmitter::Update(float _deltaTime, float _totalTime)
{
	lastEmitTimer += _deltaTime;
	while (lastEmitTimer > params.emitFrequency)
	{
		EmitParticle(_totalTime);
		lastEmitTimer -= params.emitFrequency;
	}


}

void ParticleEmitter::Draw()
{
	// Map the buffer, locking it on the GPU so we can write to it
	D3D11_MAPPED_SUBRESOURCE mapped = {};

	Graphics::Context->Map(particleDataBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

	// Copy
	memcpy(mapped.pData, particles, sizeof(Particle) * particleCount);

	// Unmap (unlock) now that we're done with it
	context->Unmap(particleDataBuffer.Get(), 0);
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

/// <summary>
/// Sets the number of Particles this Emitter can track.
/// Destroys existing Particles and recreates the buffer that stores them
/// </summary>
/// <param name="_particleCount">New maximum number of particles this Emitter can track</param>
void ParticleEmitter::SetParticleCount(unsigned int _particleCount)
{
	// Recreate Particle array
	delete[] particles;
	particles = new Particle[particleCount];

	// Release and 
	RebuildDataBuffers();
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

void ParticleEmitter::EmitParticle(float _totalTime)
{

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

/// <summary>
/// Recreates references to 
/// </summary>
void ParticleEmitter::RebuildDataBuffers()
{
	// Make a dynamic buffer to hold all particle data on GPU
	// Note: We'll be overwriting this every frame with new lifetime data
	D3D11_BUFFER_DESC desc = {};
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.Usage = D3D11_USAGE_DYNAMIC;						// Dynamic buffer, allows read/write
	desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;	// Structured buffer, stores particle structs
	desc.StructureByteStride = sizeof(Particle);
	desc.ByteWidth = sizeof(Particle) * particleCount;
	Graphics::Device->CreateBuffer(&desc, 0, particleDataBuffer.ReleaseAndGetAddressOf());

	// Create an SRV that points to a structured buffer of particles
	// so we can grab this data in a vertex shader
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = particleCount;
	Graphics::Device->CreateShaderResourceView(particleDataBuffer.Get(), &srvDesc, particleDataSRV.ReleaseAndGetAddressOf());
}

