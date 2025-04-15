#include "ParticleEmitter.h"

#include "Graphics.h"

using namespace DirectX;

// Helper macro for getting a float within a random range centered around 0
#define RandomRange(width) ((float)rand() / RAND_MAX * width - (width / 2.0f))

ParticleEmitter::ParticleEmitter(const char* _name, std::shared_ptr<Material> _material, ParticleEmitterParams _params, int _particleCount, std::shared_ptr<Transform> _transform)
{
	params = _params;
	// Sets particle count, builds particle array, and builds data buffers
	SetParticleCount(_particleCount);
	emitPeriod = 1.0f / params.emitFrequency;

	material = _material;
	transform = _transform;

	name = _name;
}

ParticleEmitter::ParticleEmitter(const char* _name, std::shared_ptr<Material> _material, ParticleEmitterParams _params, int _particleCount)
{
	params = _params;
	// Sets particle count, builds particle array, and builds data buffers
	SetParticleCount(_particleCount);
	emitPeriod = 1.0f / params.emitFrequency;
	lastEmitTimer = 0.0f;

	material = _material;
	transform = std::make_shared<Transform>();

	name = _name;
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
	
	// Update each individual living particle
	for (int i = firstAlive; i < aliveCount; i++) {

	}

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
	firstAlive = 0;
	firstDead = 0;
	aliveCount = 0;
	lastEmitTimer = 0.0f;
	particleCount = max(1, _particleCount);

	// Discard old Particle array if necessary
	if (particles) {
		delete[] particles;
	}
	// Create Particle array
	particles = new Particle[particleCount];
	// Resets memory of particle array to 0
	ZeroMemory(particles, sizeof(Particle) * particleCount);
	aliveCount = 0;

	// Release and recreate data buffers
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

void ParticleEmitter::UpdateParticle(float _totalTime, int index)
{
	// If this particle has surpassed its lifetime, kill it
	if (_totalTime - particles[index].emitTime >= params.lifetime) {
		aliveCount--;

		firstAlive++;
		firstAlive %= particleCount;
	}
}

void ParticleEmitter::EmitParticle(float _totalTime)
{
	// Do not emit if there's already the maximum number of particles
	if (aliveCount >= particleCount) {
		return;
	}

	// Recognize another particle as alive
	aliveCount++;

	XMFLOAT3 emitterPosition = transform->GetPosition();

	particles[firstDead].emitTime = _totalTime;

	// Set 
	particles[firstDead].startPosition = XMFLOAT3(
		emitterPosition.x + params.startPositionOffset.x + RandomRange(params.startPositionVariance.x),
		emitterPosition.y + params.startPositionOffset.y + RandomRange(params.startPositionVariance.y),
		emitterPosition.z + params.startPositionOffset.z + RandomRange(params.startPositionVariance.z)
	);

	particles[firstDead].startVelocity = XMFLOAT3(
		params.startVelocity.x + RandomRange(params.startVelocityVariance.x),
		params.startVelocity.y + RandomRange(params.startVelocityVariance.y),
		params.startVelocity.z + RandomRange(params.startVelocityVariance.z)
	);

	particles[firstDead].startColor = XMFLOAT4(
		params.startColor.x + RandomRange(params.startColorVariance.x),
		params.startColor.y + RandomRange(params.startColorVariance.y),
		params.startColor.z + RandomRange(params.startColorVariance.z),
		params.startColor.w + RandomRange(params.startColorVariance.w)
	);

	particles[firstDead].finalColor = XMFLOAT4(
		params.finalColor.x + RandomRange(params.finalColorVariance.x),
		params.finalColor.y + RandomRange(params.finalColorVariance.y),
		params.finalColor.z + RandomRange(params.finalColorVariance.z),
		params.finalColor.w + RandomRange(params.finalColorVariance.w)
	);

	// Recognize this particle as now alive
	firstDead++;
	firstDead %= particleCount;
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

