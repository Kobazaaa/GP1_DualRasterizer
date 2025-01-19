#pragma once
#include "pch.h"
#include "Vector3.h"
#include "ColorRGB.h"
#include "Effect.h"

#include <vector>
#include "RenderStates.h"

using namespace dae;

struct Vertex
{
	Vector3 position	{ 0.f, 0.f, 0.f };
	ColorRGB color		{ colors::White };
	Vector2 uv			{	   0.f, 0.f };
	Vector3 normal		{ 0.f, 0.f, 0.f };
	Vector3 tangent		{ 0.f, 0.f, 0.f };
};
struct VertexOut
{
	Vector4 position	{ 0.f, 0.f, 0.f, 0.f };
	Vector3 worldPos	{	   0.f, 0.f, 0.f };
	ColorRGB color		{	   colors::White };
	Vector2 uv			{			0.f, 0.f };
	Vector3 normal		{	   0.f, 0.f, 0.f };
	Vector3 tangent		{	   0.f, 0.f, 0.f };
};
enum class PrimitiveTopology
{
	TriangleList,
	TriangleStrip
};

class Mesh final
{
public:
	//--------------------------------------------------
	//    Constructors and Destructors
	//--------------------------------------------------
	Mesh(ID3D11Device* pDevice, const std::string& objFilePath, const std::string& effectPath, const bool hasTransparency = false);
	~Mesh();

	Mesh(const Mesh&) = delete;
	Mesh(Mesh&&) noexcept = delete;
	Mesh& operator=(const Mesh&) = delete;
	Mesh& operator=(Mesh&&) noexcept = delete;

	//--------------------------------------------------
	//    Rendering
	//--------------------------------------------------
	void RenderGPU(ID3D11DeviceContext* pDeviceContext) const;


	//--------------------------------------------------
	//    Software
	//--------------------------------------------------

	// Sampling
	ColorRGB SampleDiffuse(const Vector2& interpUV, float* alpha) const;
	ColorRGB SamplePhong(const Vector3& dirToLight, const Vector3& viewDir, const Vector3& interpNormal, const Vector2& interpUV, float shininess) const;
	Vector3 SampleNormalMap(const Vector3& interpNormal, const Vector3& interpTangent, const Vector2& interpUV) const;

	// Accessors
	std::vector<Vertex>& GetVerticesByReference();
	std::vector<VertexOut>& GetVerticesOutByReference();
	std::vector<uint32_t>& GetIndicesByReference();
	PrimitiveTopology GetPrimitiveTopology() const;
	bool HasTransparency() const;
	ID3D11Buffer* GetVertexBuffer() const;
	ID3D11Buffer* GetIndexBuffer() const;
	uint32_t GetNumIndices() const;

	// Mutators
	void SetPrimitiveTopology(const PrimitiveTopology& primitiveTopology);

	//--------------------------------------------------
	//    DirectX
	//--------------------------------------------------
	// Mutators
	void SetTextureSamplingState(SamplerState samplerState);

	//--------------------------------------------------
	//    Shared
	//--------------------------------------------------
	// Loaders
	void LoadDiffuseTexture(const std::string& path, ID3D11Device* pDevice);
	void LoadNormalMap(const std::string& path, ID3D11Device* pDevice);
	void LoadGlossinessMap(const std::string& path, ID3D11Device* pDevice);
	void LoadSpecularMap(const std::string& path, ID3D11Device* pDevice);

	// Mutators
	void SetWorldMatrix(const Matrix& newWorldMatrix);

	// Accessors
	const Matrix& GetWorldMatrix() const;
	Effect* GetEffect() const;

private:
	//--------------------------------------------------
	//    Mesh Data
	//--------------------------------------------------
	Matrix m_WorldMatrix{ };

	std::vector<Vertex> m_vVertices{};
	std::vector<VertexOut> m_vVerticesOut{};

	std::vector<uint32_t> m_vIndices{};
	uint32_t m_NumIndices{};

	PrimitiveTopology m_PrimitiveTopology{ PrimitiveTopology::TriangleList };
	bool m_Transparency{ false };

	//--------------------------------------------------
	//    Software
	//--------------------------------------------------
	std::unique_ptr<Texture> m_upDiffuseTxt;
	std::unique_ptr<Texture> m_upNormalTxt;
	std::unique_ptr<Texture> m_upGlossTxt;
	std::unique_ptr<Texture> m_upSpecularTxt;


	//--------------------------------------------------
	//    DirectX
	//--------------------------------------------------
	Effect* m_pEffect{};
	ID3DX11EffectTechnique* m_pCurrentTechnique{};

	ID3D11InputLayout* m_pInputLayout{};
	ID3D11Buffer* m_pVertexBuffer{};
	ID3D11Buffer* m_pIndexBuffer{};
};

