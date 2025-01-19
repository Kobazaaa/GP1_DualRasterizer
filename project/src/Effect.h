#pragma once
#include "pch.h"
#include "Texture.h"

using namespace dae;

class Effect final
{
public:
	//--------------------------------------------------
	//    Constructors and Destructors
	//--------------------------------------------------
	Effect(ID3D11Device* pDevice, const std::wstring& assetFile);
	virtual ~Effect();

	Effect(const Effect& other) = delete;
	Effect(Effect&& other) noexcept = delete;
	Effect& operator=(const Effect& rhs) = delete;
	Effect& operator=(Effect&& rhs) noexcept = delete;

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
	void LoadTexture(const std::string& variableName, const Texture* pTexture) const;
	void SetMatrixByName(const std::string& variableName, const Matrix& m) const;
	void SetVector3ByName(const std::string& variableName, const Vector3& v) const;
	void SetShaderResourceView(const std::string& variableName, ID3D11ShaderResourceView* pSRV) const;

protected:
	ID3DX11Effect* m_pEffect{};
};
