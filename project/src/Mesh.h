#pragma once
#include "pch.h"
#include "Vector3.h"
#include "ColorRGB.h"
#include "Effect.h"

#include <vector>
#include "Enums.h"

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
	ColorRGB color		{	   colors::White };
	Vector2 uv			{			0.f, 0.f };
	Vector3 normal		{	   0.f, 0.f, 0.f };
	Vector3 tangent		{	   0.f, 0.f, 0.f };
	Vector3 viewDir		{	   0.f, 0.f, 0.f };
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
	Mesh(ID3D11Device* pDevice, const std::string& objFilePath, BaseEffect* pEffect);
	~Mesh();

	Mesh(const Mesh&) = delete;
	Mesh(Mesh&&) noexcept = delete;
	Mesh& operator=(const Mesh&) = delete;
	Mesh& operator=(Mesh&&) noexcept = delete;

	//--------------------------------------------------
	//    Rendering
	//--------------------------------------------------
	void Render(ID3D11DeviceContext* pDeviceContext, bool renderWithCPU);


	//--------------------------------------------------
	//    Software
	//--------------------------------------------------
	ColorRGB SampleDiffuse(const Vector2& interpUV) const;
	ColorRGB SamplePhong(const Vector3& dirToLight, const Vector3& viewDir, const Vector3& interpNormal, const Vector2& interpUV, float shininess) const;
	Vector3 SampleNormalMap(const Vector3& interpNormal, const Vector3& interpTangent, const Vector2& interpUV) const;

	std::vector<Vertex>& GetVerticesByReference();
	std::vector<VertexOut>& GetVerticesOutByReference();
	std::vector<uint32_t>& GetIndicesByReference();

	void SetPrimitiveTopology(const PrimitiveTopology& primitiveTopology);
	PrimitiveTopology GetPrimitiveTopology() const;

	//--------------------------------------------------
	//    DirectX
	//--------------------------------------------------
	void SetTextureSamplingState(SamplerState samplerState);

	//--------------------------------------------------
	//    Shared
	//--------------------------------------------------
	void LoadDiffuseTexture(const std::string& path, ID3D11Device* pDevice);
	void LoadNormalMap(const std::string& path, ID3D11Device* pDevice);
	void LoadGlossinessMap(const std::string& path, ID3D11Device* pDevice);
	void LoadSpecularMap(const std::string& path, ID3D11Device* pDevice);

	void SetWorldMatrix(const Matrix& newWorldMatrix);
	const Matrix& GetWorldMatrix() const;

private:
	//--------------------------------------------------
	//    Rendering
	//--------------------------------------------------
	void RenderCPU();
	void RenderGPU(ID3D11DeviceContext* pDeviceContext);


	//--------------------------------------------------
	//    Mesh Data
	//--------------------------------------------------
	Matrix m_WorldMatrix{ };
	std::vector<Vertex> m_vVertices{};
	std::vector<VertexOut> m_vVerticesOut{};
	std::vector<uint32_t> m_vIndices{};
	uint32_t m_NumIndices{};
	PrimitiveTopology m_PrimitiveTopology{ PrimitiveTopology::TriangleList };

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
	BaseEffect* m_pEffect{};
	ID3DX11EffectTechnique* m_pTechnique{};

	ID3D11InputLayout* m_pInputLayout{};
	ID3D11Buffer* m_pVertexBuffer{};
	ID3D11Buffer* m_pIndexBuffer{};
};