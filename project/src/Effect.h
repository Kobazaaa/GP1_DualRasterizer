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
	void SetWorldViewProjectionMatrix(const Matrix& worldViewProjectionMatrix) const;
	void LoadTexture(const std::string& variableName, const Texture* pTexture) const;

protected:
	ID3DX11Effect* m_pEffect{};

	//--------------------------------------------------
	//    Variables
	//--------------------------------------------------
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
	void SetWorldMatrix(const Matrix& worldMatrix) const;
	void SetCameraPosition(const Vector3& cameraPosition) const;

private:
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
	void SetWorldMatrix(const Matrix& worldMatrix) const;

private:
	//--------------------------------------------------
	//    Variables
	//--------------------------------------------------
	ID3DX11EffectMatrixVariable* m_pMatWorldVariable{};

};