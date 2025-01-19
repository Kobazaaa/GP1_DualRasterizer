#pragma once
#include <map>

#include "pch.h"
#include "Effect.h"
#include "Mesh.h"

class DirectionalLight final
{
public:
	//--------------------------------------------------
	//    Constructors and Destructors
	//--------------------------------------------------
	void Initialize(ID3D11Device* pDevice, const Vector3& dir, float intensity, const ColorRGB& col = colors::White);
	DirectionalLight() = default;
	~DirectionalLight();

	DirectionalLight(const DirectionalLight&) = delete;
	DirectionalLight(DirectionalLight&&) noexcept = delete;
	DirectionalLight& operator=(const DirectionalLight&) = delete;
	DirectionalLight& operator=(DirectionalLight&&) noexcept = delete;

	//--------------------------------------------------
	//    Mutators
	//--------------------------------------------------
	void SetDirection(const Vector3& direction);
	void SetColor(const ColorRGB& color);
	void SetIntensity(float intensity);

	void UpdateViewProjection(const Vector3& target, const Vector3& up = { 0.0f, 1.0f, 0.0f });
	void RenderShadowMap(ID3D11DeviceContext* pDeviceContext, const std::map<const std::string, Mesh*>& meshes) const;

	//--------------------------------------------------
	//    Accessors
	//--------------------------------------------------
	const Matrix& GetViewMatrix() const;
	const Matrix& GetProjectionMatrix() const;
	ID3D11ShaderResourceView* GetShadowMapSRV() const;
	ID3D11DepthStencilView* GetShadowMapDSV() const;

	Vector3 GetDirection() const;
	float GetIntensity() const;
private:
	//--------------------------------------------------
	//    Light Data
	//--------------------------------------------------
	Vector3 m_Direction{ 0.577f , -0.577f , 0.577f };
	ColorRGB m_Color{ colors::White };
	float m_Intensity{ 7.f };

	Matrix m_ViewMatrix{};
	Matrix m_ProjMatrix{};

	//--------------------------------------------------
	//    Shadow Data
	//--------------------------------------------------
	uint32_t m_ShadowMapWidth = 2048;
	uint32_t m_ShadowMapHeight = 2048;
	// Shadow map resources
	ID3D11Texture2D* m_pShadowMapTexture{};
	ID3D11DepthStencilView* m_pShadowMapDSV{};
	ID3D11ShaderResourceView* m_pShadowMapSRV{};

	//--------------------------------------------------
	//    DirectX
	//--------------------------------------------------
	Effect* m_pEffect{};
	ID3D11InputLayout* m_pInputLayout{};
};
