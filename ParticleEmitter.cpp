#include "ParticleEmitter.h"

#include "Graphics.h"

ParticleEmitter::ParticleEmitter(const char* _name, std::shared_ptr<Material> _material, std::shared_ptr<Transform> _transform, ParticleEmitterParams _params, int _particleCount)
{
	params = _params;
	SetParticleCount(_particleCount);
	emitPeriod = 1.0f / params.emitFrequency;
	lastEmitTimer = 0.0f;

	material = _material;
	transform = _transform;

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
	// Add to timer
	lastEmitTimer += _deltaTime;
	
	// Kill Particles that have surpassed their lifetime


	// Create as many Particles as the timeframe would allow
	while (lastEmitTimer > emitPeriod)
	{
		EmitParticle(_totalTime);
		lastEmitTimer -= emitPeriod;
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
	Graphics::Context->Unmap(particleDataBuffer.Get(), 0);
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
	// Set emitter data trackers to initial values
	firstAlive = -1;
	firstDead = -1;
	aliveCount = 0;
	particleCount = _particleCount;

	// Discard old Particle array if necessary
	if (particles) {
		delete[] particles;
	}
	// Create Particle array
	particles = new Particle[particleCount];
	aliveCount = 0;

	// Release and 
	RebuildDataBuffers();
}

std::shared_ptr<Material> ParticleEmitter::GetMaterial()
{
	return material;
}

void ParticleEmitter::SetMaterial(std::shared_ptr<Material> _material)
{
	material = _material;
}

std::shared_ptr<Transform> ParticleEmitter::GetTransform()
{
	return transform;
}

const char* ParticleEmitter::GetName()
{
	return name;
}

void ParticleEmitter::EmitParticle(float _totalTime)
{
	// Do not emit if there's already the maximum number of particles
	if (aliveCount >= particleCount) {
		return;
	}

	aliveCount++;
}

/// <summary>
/// Creates or recreates references to data buffer and SRV
/// </summary>
void ParticleEmitter::RebuildDataBuffers()
{
	// Make an index buffer to hold 

	// Make a dynamic buffer to hold all particle data on GPU
	// Note: We'll be overwriting this every frame with new lifetime data
	D3D11_BUFFER_DESC desc = {};
	desc.BindFlags				= D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags			= D3D11_CPU_ACCESS_WRITE;
	desc.Usage					= D3D11_USAGE_DYNAMIC;						// Dynamic buffer, allows read/write
	desc.MiscFlags				= D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;	// Structured buffer, stores particle structs
	desc.StructureByteStride	= sizeof(Particle);
	desc.ByteWidth				= sizeof(Particle) * particleCount;
	Graphics::Device->CreateBuffer(&desc, 0, particleDataBuffer.ReleaseAndGetAddressOf());

	// Create an SRV that points to a structured buffer of particles
	// so we can grab this data in a vertex shader
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension		= D3D11_SRV_DIMENSION_BUFFER;
	srvDesc.Format				= DXGI_FORMAT_UNKNOWN;
	srvDesc.Buffer.FirstElement	= 0;
	srvDesc.Buffer.NumElements	= particleCount;
	Graphics::Device->CreateShaderResourceView(particleDataBuffer.Get(), &srvDesc, particleDataSRV.ReleaseAndGetAddressOf());
}

