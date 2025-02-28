#pragma once

#include <d3d12.h>
#include <wrl/client.h>

#include "Graphics.h"
#include "Vertex.h"

class Mesh
{
public:
	// Constructors/Destructor
	Mesh(const char* _name, Vertex* _vertices, size_t _vertexCount, unsigned int* _indices, size_t _indexCount);
	Mesh(const char* _name, const wchar_t* _path);
	~Mesh();
	// Accessors for Mesh info
	Microsoft::WRL::ComPtr<ID3D12Resource> GetVertexBuffer();
	Microsoft::WRL::ComPtr<ID3D12Resource> GetIndexBuffer();
	D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView();
	D3D12_INDEX_BUFFER_VIEW GetIndexBufferView();
	int GetVertexCount();
	int GetIndexCount();
	const char* GetName();

private:
	// Vertex and index buffers, as well as the size of each

	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW vbView{};
	unsigned int vertexCount;

	Microsoft::WRL::ComPtr<ID3D12Resource> indexBuffer;
	D3D12_INDEX_BUFFER_VIEW ibView{};
	unsigned int indexCount;

	// Name for UI
	const char* name;

	// Code for calculating tangents
	void CalculateTangents(Vertex* verts, int numVerts, unsigned int* indices, int numIndices);
	// Code for creating vertex and index buffers
	void InitializeBuffers(Vertex* _vertices, unsigned int _vertexCount, unsigned int* _indices, unsigned int _indexCount);
};

