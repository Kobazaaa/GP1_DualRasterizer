#include "Mesh.h"
#include "Utils.h"
#include <cassert>

//--------------------------------------------------
//    Constructors and Destructors
//--------------------------------------------------
Mesh::Mesh(ID3D11Device* pDevice, const std::string& objFilePath, const std::string& effectPath, const bool hasTransparency)
{
	m_Transparency = hasTransparency;

	// Parse the OBJ Mesh
	Utils::ParseOBJ(objFilePath, m_vVertices, m_vIndices);

	// Get the Effect and Technique
	m_pEffect = new Effect(pDevice, {effectPath.begin(), effectPath.end()});
	if (m_pEffect->GetEffect()->IsValid()) m_pCurrentTechnique = m_pEffect->GetTechniqueByIndex(0);

	// Create Vertex Layout
	static constexpr uint32_t numElements{ 5 };
	D3D11_INPUT_ELEMENT_DESC vertexDesc[numElements]{};

	vertexDesc[0].SemanticName = "POSITION";
	vertexDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[0].AlignedByteOffset = offsetof(Vertex, position);
	vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[1].SemanticName = "COLOR";
	vertexDesc[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[1].AlignedByteOffset = offsetof(Vertex, color);
	vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[2].SemanticName = "TEXCOORD";
	vertexDesc[2].SemanticIndex = 0;
	vertexDesc[2].Format = DXGI_FORMAT_R32G32_FLOAT;
	vertexDesc[2].AlignedByteOffset = offsetof(Vertex, uv);
	vertexDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[3].SemanticName = "NORMAL";
	vertexDesc[3].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[3].AlignedByteOffset = offsetof(Vertex, normal);
	vertexDesc[3].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	vertexDesc[4].SemanticName = "TANGENT";
	vertexDesc[4].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexDesc[4].AlignedByteOffset = offsetof(Vertex, tangent);
	vertexDesc[4].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	// Create Input Layout
	D3DX11_PASS_DESC passDesc{};
	m_pEffect->GetTechniqueByIndex(0)->GetPassByIndex(0)->GetDesc(&passDesc);

	const HRESULT resultIL = pDevice->CreateInputLayout(
		vertexDesc,
		numElements,
		passDesc.pIAInputSignature,
		passDesc.IAInputSignatureSize,
		&m_pInputLayout);

	if (FAILED(resultIL))
		assert(false);

	// Create Vertex Buffer
	D3D11_BUFFER_DESC bd{};
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(Vertex) * static_cast<uint32_t>(m_vVertices.size());
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData{};
	initData.pSysMem = m_vVertices.data();

	HRESULT result = pDevice->CreateBuffer(&bd, &initData, &m_pVertexBuffer);
	if (FAILED(result))
		assert(false);

	// Create Index Buffer
	m_NumIndices = static_cast <uint32_t>(m_vIndices.size());
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.ByteWidth = sizeof(uint32_t) * m_NumIndices;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags = 0;
	initData.pSysMem = m_vIndices.data();
	result = pDevice->CreateBuffer(&bd, &initData, &m_pIndexBuffer);
	if (FAILED(result))
		assert(false);
}
Mesh::~Mesh()
{
	// Release resources
	if(m_pIndexBuffer)		m_pIndexBuffer->Release();
	if(m_pVertexBuffer)		m_pVertexBuffer->Release();
	if(m_pInputLayout)		m_pInputLayout->Release();


	delete m_pEffect;
}

//--------------------------------------------------
//    Rendering
//--------------------------------------------------
void Mesh::RenderGPU(ID3D11DeviceContext* pDeviceContext) const
{
	//1. Set Primitive Topology
	switch (m_PrimitiveTopology)
	{
	case PrimitiveTopology::TriangleList:
		pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		break;
	case PrimitiveTopology::TriangleStrip:
		pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
		break;
	default:
		pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	//2. Set Input Layout
	pDeviceContext->IASetInputLayout(m_pInputLayout);

	//3. Set Vertex Buffer
	constexpr UINT stride = sizeof(Vertex);
	constexpr UINT offset = 0;
	pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

	//4. Set Index Buffer
	pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	//5. Draw
	D3DX11_TECHNIQUE_DESC techDesc{};
	m_pCurrentTechnique->GetDesc(&techDesc);
	for (UINT p = 0; p < techDesc.Passes; ++p)
	{
		m_pCurrentTechnique->GetPassByIndex(p)->Apply(0, pDeviceContext);
		pDeviceContext->DrawIndexed(m_NumIndices, 0, 0);
	}
}


//--------------------------------------------------
//    Software
//--------------------------------------------------

// Sampling
ColorRGB Mesh::SampleDiffuse(const Vector2& interpUV, float* alpha) const
{
	if (m_upDiffuseTxt == nullptr) return {};
	return m_upDiffuseTxt->Sample(interpUV, m_Transparency, alpha);
}
ColorRGB Mesh::SamplePhong(const Vector3& dirToLight, const Vector3& viewDir, const Vector3& interpNormal, const Vector2& interpUV, const float shininess) const
{
	if (m_upSpecularTxt == nullptr) return {};
	if (m_upGlossTxt == nullptr) return {};

	const float ks = m_upSpecularTxt->Sample(interpUV).b;
	const float exp = m_upGlossTxt->Sample(interpUV).b * shininess;

	const Vector3 reflect = Vector3::Reflect(dirToLight, interpNormal);
	const float cosAlpha{ std::max(Vector3::Dot(reflect, viewDir), 0.f) };
	return ColorRGB(1, 1, 1) * ks * std::pow(cosAlpha, exp);
}
Vector3 Mesh::SampleNormalMap(const Vector3& interpNormal, const Vector3& interpTangent, const Vector2& interpUV) const
{
	if (m_upNormalTxt == nullptr) return { interpNormal };

	// Calculate the tangent space matrix
	Vector3 binormal = Vector3::Cross(interpNormal, interpTangent);
	Matrix tangentSpaceAxis = Matrix(
		interpTangent,
		binormal,
		interpNormal,
		Vector3::Zero
	);

	// Sample the normal map
	ColorRGB nrmlMap = m_upNormalTxt->Sample(interpUV);

	// normal's X & Y are in range [0; 1], while Z is in range [0.5; 1]
	Vector3 normal{ nrmlMap.r, nrmlMap.g, nrmlMap.b };
	// remap normal X & Y to [-1; 1] and Z to [0; 1]
	normal = 2.f * normal - Vector3(1.f, 1.f, 1.f);

	normal = tangentSpaceAxis.TransformVector(normal);

	return normal.Normalized();
}

// Accessors
std::vector<Vertex>& Mesh::GetVerticesByReference()			{ return m_vVertices; }
std::vector<VertexOut>& Mesh::GetVerticesOutByReference()	{ return m_vVerticesOut; }
std::vector<uint32_t>& Mesh::GetIndicesByReference()		{ return m_vIndices; }
PrimitiveTopology Mesh::GetPrimitiveTopology() const		{ return m_PrimitiveTopology; }
bool Mesh::HasTransparency() const							{ return m_Transparency; }

ID3D11Buffer* Mesh::GetVertexBuffer() const
{
	return m_pVertexBuffer;
}

ID3D11Buffer* Mesh::GetIndexBuffer() const
{
	return m_pIndexBuffer;
}

uint32_t Mesh::GetNumIndices() const
{
	return m_NumIndices;
}

// Mutators
void Mesh::SetPrimitiveTopology(const PrimitiveTopology& primitiveTopology) { m_PrimitiveTopology = primitiveTopology; }


//--------------------------------------------------
//    DirectX
//--------------------------------------------------

// Mutators
void Mesh::SetTextureSamplingState(SamplerState samplerState)
{
	switch (samplerState)
	{
	case SamplerState::Point:
		m_pCurrentTechnique = m_pEffect->GetTechniqueByName("PointSamplingTechnique");
		break;
	case SamplerState::Linear:
		m_pCurrentTechnique = m_pEffect->GetTechniqueByName("LinearSamplingTechnique");
		break;
	case SamplerState::Anisotropic:
		m_pCurrentTechnique = m_pEffect->GetTechniqueByName("AnisotropicSamplingTechnique");
		break;
	default:
		m_pCurrentTechnique = m_pEffect->GetTechniqueByIndex(0);
		break;
	}
}


//--------------------------------------------------
//    Shared
//--------------------------------------------------

// Loaders
void Mesh::LoadDiffuseTexture(const std::string& path, ID3D11Device* pDevice)
{
	Texture* texture = Texture::LoadFromFile(path, pDevice);
	m_upDiffuseTxt.reset(texture);
	m_pEffect->LoadTexture("gDiffuseMap", texture);
}
void Mesh::LoadNormalMap(const std::string& path, ID3D11Device* pDevice)
{
	Texture* texture = Texture::LoadFromFile(path, pDevice);
	m_upNormalTxt.reset(texture);
	m_pEffect->LoadTexture("gNormalMap", texture);
}
void Mesh::LoadGlossinessMap(const std::string& path, ID3D11Device* pDevice)
{
	Texture* texture = Texture::LoadFromFile(path, pDevice);
	m_upGlossTxt.reset(texture);
	m_pEffect->LoadTexture("gGlossinessMap", texture);
}
void Mesh::LoadSpecularMap(const std::string& path, ID3D11Device* pDevice)
{
	Texture* texture = Texture::LoadFromFile(path, pDevice);
	m_upSpecularTxt.reset(texture);
	m_pEffect->LoadTexture("gSpecularMap", texture);
}

// Mutators
void Mesh::SetWorldMatrix(const Matrix& newWorldMatrix) { m_WorldMatrix = newWorldMatrix; }

// Accessors
const Matrix& Mesh::GetWorldMatrix() const { return m_WorldMatrix; }
Effect* Mesh::GetEffect() const
{
	return m_pEffect;
}