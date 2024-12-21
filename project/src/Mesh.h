#pragma once
#include "pch.h"
#include "Vector3.h"
#include "ColorRGB.h"
#include "Effect.h"

#include <vector>

using namespace dae;

struct Vertex
{
	Vector3 position	{ 0.f, 0.f, 0.f };
	ColorRGB color		{ colors::White };
	Vector2 uv			{	   0.f, 0.f };
	Vector3 normal		{ 0.f, 0.f, 0.f };
	Vector3 tangent		{ 0.f, 0.f, 0.f };
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
	//    Accessors and Mutators
	//--------------------------------------------------
	void SetWorldMatrix(const Matrix& newWorldMatrix);
	const Matrix& GetWorldMatrix() const;

	//--------------------------------------------------
	//    Software
	//--------------------------------------------------

	//--------------------------------------------------
	//    DirectX
	//--------------------------------------------------


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
	std::vector<uint32_t> m_vIndices{};
	uint32_t m_NumIndices{};


	//--------------------------------------------------
	//    DirectX
	//--------------------------------------------------
	BaseEffect* m_pEffect{};

	ID3D11InputLayout* m_pInputLayout{};
	ID3D11Buffer* m_pVertexBuffer{};
	ID3D11Buffer* m_pIndexBuffer{};
};