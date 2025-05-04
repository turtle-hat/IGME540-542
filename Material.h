#pragma once

#include <memory>
#include <DirectXMath.h>

#include "SimpleShader.h"

class Material
{
public:
	Material(const char* _name, std::shared_ptr<SimpleVertexShader> _vertexShader, std::shared_ptr<SimplePixelShader> _pixelShader, DirectX::XMFLOAT4 _colorTint, float _roughness);
	Material(const char* _name, std::shared_ptr<SimpleVertexShader> _vertexShader, std::shared_ptr<SimplePixelShader> _pixelShader, DirectX::XMFLOAT4 _colorTint, float _roughness, bool _useGlobalEnvironmentMap);
	Material(const char* _name, std::shared_ptr<SimpleVertexShader> _vertexShader, std::shared_ptr<SimplePixelShader> _pixelShader, DirectX::XMFLOAT4 _colorTint, float _roughness, float _metalness);

	std::shared_ptr<SimpleVertexShader> GetVertexShader();
	std::shared_ptr<SimplePixelShader> GetPixelShader();
	DirectX::XMFLOAT4 GetColorTint();
	float GetRoughness();
	float GetMetalness();
	float GetAlphaThreshold();
	const char* GetName();
	DirectX::XMFLOAT2 GetUVPosition();
	DirectX::XMFLOAT2 GetUVScale();
	std::vector<ID3D11ShaderResourceView*> GetTextures();

	void SetVertexShader(std::shared_ptr<SimpleVertexShader> _vertexShader);
	void SetPixelShader(std::shared_ptr<SimplePixelShader> _pixelShader);
	void SetColorTint(DirectX::XMFLOAT4 _colorTint);
	void SetRoughness(float _roughness);
	void SetMetalness(float _metalness);
	void SetAlphaThreshold(float _alphaThreshold);
	void SetUVPosition(DirectX::XMFLOAT2 _position);
	void SetUVScale(DirectX::XMFLOAT2 _scale);
	void LockSamplerState();
	void UnlockSamplerState();

	void AddTextureSRV(std::string _name, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> _srv);
	void AddSampler(std::string _name, Microsoft::WRL::ComPtr<ID3D11SamplerState> _sampler);
	void PrepareMaterial();
	// Whether this material uses the global environment map. Allows a function in Game to change the shader's MapCube SRV to match the environment
	bool useGlobalEnvironmentMap;
	// Whether this material uses PBR shaders and thus ignores roughness
	bool isPBR;
	bool useAlphaThreshold;

private:
	void RebuildTextureList();

	std::shared_ptr<SimpleVertexShader> vertexShader;
	std::shared_ptr<SimplePixelShader> pixelShader;
	DirectX::XMFLOAT4 colorTint;
	float roughness;
	float metalness;
	float alphaThreshold;

	// Texture settings

	// The UV coordinate marking the top-left corner of the textures
	DirectX::XMFLOAT2 uvPosition;
	// Scales the texture's UV coordinates
	DirectX::XMFLOAT2 uvScale;
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> textureSRVs;
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11SamplerState>> samplers;
	// Returned with GetTextures so it doesn't have to be rebuilt each time
	std::vector<ID3D11ShaderResourceView*> textureList;
	// Locks the sampler state so it isn't affected by changes to the global sampler state
	bool isSamplerStateLocked;

	const char* name;
};

