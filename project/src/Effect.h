#pragma once
#include "pch.h"
#include "Texture.h"

using namespace dae;

class BaseEffect
{
public:
	//--------------------------------------------------
	//    Constructors and Destructors
	//--------------------------------------------------
	BaseEffect(ID3D11Device* pDevice, const std::wstring& assetFile);
	virtual ~BaseEffect();

	BaseEffect(const BaseEffect& other) = delete;
	BaseEffect(BaseEffect&& other) noexcept = delete;
	BaseEffect& operator=(const BaseEffect& rhs) = delete;
	BaseEffect& operator=(BaseEffect&& rhs) noexcept = delete;

	//--------------------------------------------------
	//    Effect Loader
	//--------------------------------------------------
	static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile);

	//--------------------------------------------------
	//    Accessors
	//--------------------------------------------------
	ID3DX11Effect* GetEffect() const;
	ID3DX11EffectTechnique* GetTechnique() const;

	//--------------------------------------------------
	//    Mutators
	//--------------------------------------------------
	void SetWorldViewProjectionMatrix(const Matrix& worldViewProjectionMatrix);

protected:
	ID3DX11Effect* m_pEffect{};
	ID3DX11EffectTechnique* m_pTechnique{};

	ID3DX11EffectMatrixVariable* m_pMatWorldViewProjVariable{};
};

class FullShadeEffect final : public BaseEffect
{
public:
	//--------------------------------------------------
	//    Constructors and Destructors
	//--------------------------------------------------
	FullShadeEffect(ID3D11Device* pDevice, const std::wstring& assetFile);
	virtual ~FullShadeEffect() override;

	FullShadeEffect(const FullShadeEffect& other) = delete;
	FullShadeEffect(FullShadeEffect&& other) noexcept = delete;
	FullShadeEffect& operator=(const FullShadeEffect& rhs) = delete;
	FullShadeEffect& operator=(FullShadeEffect&& rhs) noexcept = delete;

	//--------------------------------------------------
	//    Mutators
	//--------------------------------------------------
	void SetDiffuseMap(const Texture* pDiffuseTexture);
	void SetNormalMap(const Texture* pNormalTexture);
	void SetSpecularMap(const Texture* pSpecularTexture);
	void SetGlossinessMap(const Texture* pGlossinessTexture);

	void SetWorldMatrix(const Matrix& worldMatrix);
	void SetCameraPosition(const Vector3& cameraPosition);
private:
	//--------------------------------------------------
	//    Textures
	//--------------------------------------------------
	ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable{};
	ID3DX11EffectShaderResourceVariable* m_pNormalMapVariable{};
	ID3DX11EffectShaderResourceVariable* m_pSpecularMapVariable{};
	ID3DX11EffectShaderResourceVariable* m_pGlossinessMapVariable{};

	//--------------------------------------------------
	//    Variables
	//--------------------------------------------------
	ID3DX11EffectMatrixVariable* m_pMatWorldVariable{};
	ID3DX11EffectVectorVariable* m_pVecCameraVariable{};
};

class FlatShadeEffect final : public BaseEffect
{
public:
	//--------------------------------------------------
	//    Constructors and Destructors
	//--------------------------------------------------
	FlatShadeEffect(ID3D11Device* pDevice, const std::wstring& assetFile);
	virtual ~FlatShadeEffect() override;

	FlatShadeEffect(const FlatShadeEffect& other) = delete;
	FlatShadeEffect(FlatShadeEffect&& other) noexcept = delete;
	FlatShadeEffect& operator=(const FlatShadeEffect& rhs) = delete;
	FlatShadeEffect& operator=(FlatShadeEffect&& rhs) noexcept = delete;

	//--------------------------------------------------
	//    Mutators
	//--------------------------------------------------
	void SetDiffuseMap(const Texture* pDiffuseTexture);
	void SetWorldMatrix(const Matrix& worldMatrix);

private:
	//--------------------------------------------------
	//    Textures
	//--------------------------------------------------
	ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable{};

	//--------------------------------------------------
	//    Variables
	//--------------------------------------------------
	ID3DX11EffectMatrixVariable* m_pMatWorldVariable{};

};