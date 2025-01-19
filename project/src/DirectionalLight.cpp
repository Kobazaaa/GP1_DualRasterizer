#include "DirectionalLight.h"
#include <cassert>
#include "Mesh.h"

//--------------------------------------------------
//    Constructors and Destructors
//--------------------------------------------------
void DirectionalLight::Initialize(ID3D11Device* pDevice, const Vector3& dir, float intensity, const ColorRGB& col)
{
    // 1.
    // Set Light Info
    // TODO current not used
    m_Direction = dir;
    m_Color = col;
    m_Intensity = intensity;

    // 2.
    // Create Effect
    m_pEffect = new Effect(pDevice, L"resources/LightMap.fx");

    // 3.
    // Create shadow map texture
    D3D11_TEXTURE2D_DESC shadowMapDesc = {};
    shadowMapDesc.Width = m_ShadowMapWidth;
    shadowMapDesc.Height = m_ShadowMapHeight;
    shadowMapDesc.MipLevels = 1;
    shadowMapDesc.ArraySize = 1;
    shadowMapDesc.Format = DXGI_FORMAT_R32_TYPELESS;
    shadowMapDesc.SampleDesc.Count = 1;
    shadowMapDesc.Usage = D3D11_USAGE_DEFAULT;
    shadowMapDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
    shadowMapDesc.CPUAccessFlags = 0;
    shadowMapDesc.MiscFlags = 0;

    auto result = pDevice->CreateTexture2D(&shadowMapDesc, nullptr, &m_pShadowMapTexture);
    if (FAILED(result)) {
        assert(false);
    }


    // 4.
    // Create Depth Stencil View for shadow map
    D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
    depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
    depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
    depthStencilViewDesc.Texture2D.MipSlice = 0;

    result = pDevice->CreateDepthStencilView(m_pShadowMapTexture, &depthStencilViewDesc, &m_pShadowMapDSV);
    if (FAILED(result)) {
        assert(false);
    }



    // 5.
	// Create SRV for shadow map
    D3D11_SHADER_RESOURCE_VIEW_DESC descSRV{};
    descSRV.Format = DXGI_FORMAT_R32_FLOAT;
    descSRV.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    descSRV.Texture2D.MipLevels = 1;
    descSRV.Texture2D.MostDetailedMip = 0;

    result = pDevice->CreateShaderResourceView(m_pShadowMapTexture, &descSRV, &m_pShadowMapSRV);
    if (FAILED(result)) {
        assert(false);
    }


    // 5.
	// Create Vertex & Input Layout
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

    D3DX11_PASS_DESC passDesc{};
    m_pEffect->GetTechniqueByIndex(0)->GetPassByIndex(0)->GetDesc(&passDesc);

    result = pDevice->CreateInputLayout(
        vertexDesc,
        numElements,
        passDesc.pIAInputSignature,
        passDesc.IAInputSignatureSize,
        &m_pInputLayout);

    if (FAILED(result))
        assert(false);
}
DirectionalLight::~DirectionalLight()
{
    if (m_pInputLayout)         m_pInputLayout->Release();
    if (m_pShadowMapSRV)        m_pShadowMapSRV->Release();
    if (m_pShadowMapDSV)        m_pShadowMapDSV->Release();
    if (m_pShadowMapTexture)    m_pShadowMapTexture->Release();

    delete m_pEffect;
}


//--------------------------------------------------
//    Mutators
//--------------------------------------------------
void DirectionalLight::SetDirection(const Vector3& direction)
{
    m_Direction = direction.Normalized();
}
void DirectionalLight::SetColor(const ColorRGB& color)
{
    m_Color = color;
}
void DirectionalLight::SetIntensity(float intensity)
{
    m_Intensity = intensity;
}

void DirectionalLight::UpdateViewProjection(const Vector3& target, const Vector3& up)
{
    const Vector3 lightDir = m_Direction.Normalized();
    const Vector3 lightPos = lightDir * (-100.f);

    m_ViewMatrix = Matrix::CreateLookAtLH(lightPos, target, up);

    // Calculate the orthographic projection matrix
    // todo look into fixing?
    constexpr float orthoWidth = 200;
    constexpr float orthoHeight = 200;
    constexpr float nearPlane = 0.1f;
    constexpr float farPlane = 1000.f;

    m_ProjMatrix = Matrix::CreateOrthographicLH(orthoWidth, orthoHeight, nearPlane, farPlane);
}
void DirectionalLight::RenderShadowMap(ID3D11DeviceContext* pDeviceContext, const std::map<const std::string, Mesh*>& meshes) const
{
    // 1.
    // Set Render Target to the Shadow Map
	pDeviceContext->OMSetRenderTargets(0, nullptr, m_pShadowMapDSV);
    pDeviceContext->ClearDepthStencilView(m_pShadowMapDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);
	m_pEffect->GetTechniqueByIndex(0)->GetPassByIndex(0)->Apply(0, pDeviceContext);

    // 2.
    // Set the viewport to a matching one for the shadow map
    D3D11_VIEWPORT viewport{};
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = static_cast<float>(m_ShadowMapWidth);
    viewport.Height = static_cast<float>(m_ShadowMapHeight);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    pDeviceContext->RSSetViewports(1, &viewport);

    // 2.
	// Set the Input Layout
	pDeviceContext->IASetInputLayout(m_pInputLayout);

    // 3.
    // Map the meshes
    for (const auto& m : meshes)
    {
        Mesh* currM = m.second;

        // Only shadow map meshes without transparency
        if (currM->HasTransparency()) continue;

        // 4.
        // Set the Primitive Topology
        switch (currM->GetPrimitiveTopology())
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

        // 5.
        // Set the Vertex Buffer
        constexpr UINT stride = sizeof(Vertex);
        constexpr UINT offset = 0;
        auto vertexBuffer = currM->GetVertexBuffer();
        pDeviceContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);

        // 6.
        // Set the Index Buffer
        pDeviceContext->IASetIndexBuffer(currM->GetIndexBuffer(), DXGI_FORMAT_R32_UINT, 0);


        // 7.
		// Set the World View Projection Matrix
        auto wvp = currM->GetWorldMatrix() * m_ViewMatrix * m_ProjMatrix;
        m_pEffect->SetMatrixByName("gWorldViewProj", wvp);
		m_pEffect->GetTechniqueByIndex(0)->GetPassByIndex(0)->Apply(0, pDeviceContext);


        // 7.
		// "Draw" to shadow map
	    pDeviceContext->DrawIndexed(currM->GetNumIndices(), 0, 0);
    }


    // 8.
    // When done, reset Render Target
    ID3D11RenderTargetView* nullRTV = nullptr;
    pDeviceContext->OMSetRenderTargets(1, &nullRTV, nullptr);
}


//--------------------------------------------------
//    Accessors
//--------------------------------------------------
const Matrix& DirectionalLight::GetViewMatrix() const
{
    return m_ViewMatrix;
}
const Matrix& DirectionalLight::GetProjectionMatrix() const
{
    return m_ProjMatrix;
}
ID3D11ShaderResourceView* DirectionalLight::GetShadowMapSRV() const
{
    return m_pShadowMapSRV;
}
ID3D11DepthStencilView* DirectionalLight::GetShadowMapDSV() const
{
    return m_pShadowMapDSV;
}

Vector3 DirectionalLight::GetDirection() const
{
    return m_Direction;
}
float DirectionalLight::GetIntensity() const
{
    return m_Intensity;
}

