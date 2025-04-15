#include "ParticleEmitter.h"

#include <algorithm>
#include "Graphics.h"
#include "Vertex.h"

using namespace DirectX;

// Heavily based on code written by Professor Chris Cascioli

// Helper macro for getting a float within a random range centered around 0
#define RandomRange(width) ((float)rand() / RAND_MAX * width - (width / 2.0f))

ParticleEmitter::ParticleEmitter(const char* _name, std::shared_ptr<Material> _material, ParticleEmitterParams _params, int _particleCount, std::shared_ptr<Transform> _transform)
{
	params = _params;
	particles = nullptr;
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
	particles = nullptr;
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
	for (unsigned int i = firstAlive; i < aliveCount; i++) {
		UpdateParticle(_totalTime, i % particleCount);
	}

	// Create as many Particles as the timeframe would allow
	while (lastEmitTimer > emitPeriod)
	{
		EmitParticle(_totalTime);
		lastEmitTimer -= emitPeriod;
	}
}

void ParticleEmitter::Draw(std::shared_ptr<Camera> _camera, float _totalTime)
{
	// Copy Particles to the GPU

	// Map the buffer, locking it on the GPU so we can write to it
	D3D11_MAPPED_SUBRESOURCE mapped = {};

	Graphics::Context->Map(particleDataBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

	// Copy only living particles
	// Written by Professor Chris Cascioli

	// How are living particles arranged in the buffer?
	if (firstAlive < firstDead)
	{
		// Only copy from FirstAlive -> FirstDead
		memcpy(
			mapped.pData,					// Destination = start of particle buffer
			particles + firstAlive,			// Source = particle array, offset to first living particle
			sizeof(Particle) * aliveCount);	// Amount = number of particles (measured in BYTES!)
	}
	else
	{
		// Copy from 0 -> FirstDead
		memcpy(
			mapped.pData,					// Destination = start of particle buffer
			particles,						// Source = start of particle array
			sizeof(Particle) * firstDead);	// Amount = particles up to first dead (measured in BYTES!)
		// ALSO copy from FirstAlive -> End
		memcpy(
			(void*)((Particle*)mapped.pData + firstDead),		// Destination = particle buffer, AFTER the data we copied in previous memcpy()
			particles + firstAlive,								// Source = particle array, offset to first living particle
			sizeof(Particle) * (particleCount - firstAlive));	// Amount = number of living particles at end of array (measured in BYTES!)
	}

	// Unmap (unlock) now that we're done with it
	Graphics::Context->Unmap(particleDataBuffer.Get(), 0);



	// Set buffers in the input assembler stage
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	ID3D11Buffer* nullBuffer = 0;
	Graphics::Context->IASetVertexBuffers(0, 1, &nullBuffer, &stride, &offset);
	Graphics::Context->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	// Prepare the material for drawing
	material->PrepareMaterial();

	// Get references to the emitter's shaders
	std::shared_ptr<SimpleVertexShader> vs = material->GetVertexShader();
	std::shared_ptr<SimplePixelShader> ps = material->GetPixelShader();

	// Set vertex and pixel shaders
	vs->SetShader();
	ps->SetShader();

	// Fill constant buffers with entity's data
	// VERTEX
	vs->SetMatrix4x4("tfView", _camera->GetViewMatrix());
	vs->SetMatrix4x4("tfProjection", _camera->GetProjectionMatrix());
	vs->SetFloat("lifetime", params.lifetime);
	vs->SetFloat3("acceleration", params.acceleration);
	vs->SetFloat("totalTime", _totalTime);
	vs->SetFloat3("padding", XMFLOAT3(0.0f, 0.0f, 0.0f));
	// PIXEL
	// None yet

	// COPY DATA TO CONSTANT BUFFERS
	vs->CopyAllBufferData();
	ps->CopyAllBufferData();

	Graphics::Context->DrawIndexed(
		particleCount * 6,	// The number of indices to use (we could draw a subset if we wanted)
		0,					// Offset to the first index we want to use
		0);					// Offset to add to each index when looking up vertices
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

int ParticleEmitter::GetFirstAlive()
{
	return firstAlive;
}

int ParticleEmitter::GetFirstDead()
{
	return firstDead;
}

unsigned int ParticleEmitter::GetAliveCount()
{
	return aliveCount;
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

	// Set the emit time of the particle
	particles[firstDead].emitTime = _totalTime;

	// Set randomized parameters
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
		std::clamp(params.startColor.x + RandomRange(params.startColorVariance.x), 0.0f, 1.0f),
		std::clamp(params.startColor.y + RandomRange(params.startColorVariance.y), 0.0f, 1.0f),
		std::clamp(params.startColor.z + RandomRange(params.startColorVariance.z), 0.0f, 1.0f),
		std::clamp(params.startColor.w + RandomRange(params.startColorVariance.w), 0.0f, 1.0f)
	);

	particles[firstDead].finalColor = XMFLOAT4(
		std::clamp(params.finalColor.x + RandomRange(params.finalColorVariance.x), 0.0f, 1.0f),
		std::clamp(params.finalColor.y + RandomRange(params.finalColorVariance.y), 0.0f, 1.0f),
		std::clamp(params.finalColor.z + RandomRange(params.finalColorVariance.z), 0.0f, 1.0f),
		std::clamp(params.finalColor.w + RandomRange(params.finalColorVariance.w), 0.0f, 1.0f)
	);

	// Recognize this particle as now alive
	firstDead++;
	firstDead %= particleCount;
}

/// <summary>
/// Creates or recreates references to data buffer and SRV
/// Modified from code by Professor Chris Cascioli
/// </summary>
void ParticleEmitter::RebuildDataBuffers()
{
	// Make an index buffer to hold particle geometry
	indexBuffer.Reset();
	// Temporary array to store indices during buffer creation
	unsigned int* indices = new unsigned int[particleCount * 6];
	int indexCount = 0;

	// Iterate through the indices of each quad (set of 4 vertices)
	for (unsigned int i = 0; i < particleCount * 4; i += 4) {
		indices[indexCount++] = i;
		indices[indexCount++] = i + 1;
		indices[indexCount++] = i + 2;
		indices[indexCount++] = i;
		indices[indexCount++] = i + 2;
		indices[indexCount++] = i + 3;
	}
	
	// Describe the buffer
	D3D11_BUFFER_DESC ibd = {};
	ibd.Usage				= D3D11_USAGE_DEFAULT;
	ibd.ByteWidth			= sizeof(unsigned int) * particleCount * 6;	// 3 = number of indices in the buffer
	ibd.BindFlags			= D3D11_BIND_INDEX_BUFFER;
	ibd.CPUAccessFlags		= 0;
	ibd.MiscFlags			= 0;
	ibd.StructureByteStride	= 0;

	// Specify the initial data for this buffer
	D3D11_SUBRESOURCE_DATA initialIndexData = {};
	initialIndexData.pSysMem = indices;

	// Actually create the buffer with the initial data
	// - Once we do this, we'll NEVER CHANGE THE BUFFER AGAIN
	Graphics::Device->CreateBuffer(&ibd, &initialIndexData, indexBuffer.GetAddressOf());
	delete[] indices;	// Clear memory now that indices are on GPU



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

