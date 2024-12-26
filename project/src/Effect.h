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
	ID3DX11EffectTechnique* GetTechniqueByName(const std::string& name) const;
	ID3DX11EffectTechnique* GetTechniqueByIndex(int index) const;

	//--------------------------------------------------
	//    Mutators
	//--------------------------------------------------
	void SetWorldViewProjectionMatrix(const Matrix& worldViewProjectionMatrix);
	void LoadTexture(const std::string& variableName, const Texture* pTexture);

protected:
	ID3DX11Effect* m_pEffect{};

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

	void SetWorldMatrix(const Matrix& worldMatrix);
	void SetCameraPosition(const Vector3& cameraPosition);
private:
	//--------------------------------------------------
	//    Textures
	//--------------------------------------------------

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
	void SetWorldMatrix(const Matrix& worldMatrix);

private:
	//--------------------------------------------------
	//    Textures
	//--------------------------------------------------

	//--------------------------------------------------
	//    Variables
	//--------------------------------------------------
	ID3DX11EffectMatrixVariable* m_pMatWorldVariable{};

};