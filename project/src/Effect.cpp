#include "Effect.h"

#pragma region BaseEffect
//--------------------------------------------------
//    Constructors and Destructors
//--------------------------------------------------
BaseEffect::BaseEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
{
	m_pEffect = LoadEffect(pDevice, assetFile);
	if (m_pEffect)
	{
		m_pMatWorldViewProjVariable = m_pEffect->GetVariableByName("gWorldViewProj")->AsMatrix();
		if (!m_pMatWorldViewProjVariable->IsValid())
			std::wcout << L"m_pMatWorldViewProjVariable not valid!\n";

	}
}
BaseEffect::~BaseEffect()
{
	if (m_pMatWorldViewProjVariable) m_pMatWorldViewProjVariable->Release();
	if (m_pEffect)					 m_pEffect->Release();
}

//--------------------------------------------------
//    Effect Loader
//--------------------------------------------------
ID3DX11Effect* BaseEffect::LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
{
	HRESULT result;
	ID3D10Blob* pErrorBlob = nullptr;
	ID3DX11Effect* pEffect;

	DWORD shaderFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
	shaderFlags |= D3DCOMPILE_DEBUG;
	shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	result = D3DX11CompileEffectFromFile(assetFile.c_str(),
		nullptr,
		nullptr,
		shaderFlags,
		0,
		pDevice,
		&pEffect,
		&pErrorBlob);

	if (FAILED(result))
	{
		if (pErrorBlob != nullptr)
		{
			const char* pErrors = static_cast<char*>(pErrorBlob->GetBufferPointer());

			std::wstringstream ss;
			for (unsigned int i{}; i < pErrorBlob->GetBufferSize(); ++i)
				ss << pErrors[i];

			OutputDebugStringW(ss.str().c_str());
			pErrorBlob->Release();
			pErrorBlob = nullptr;

			std::wcout << ss.str() << "\n";
			return nullptr;
		}
		else
		{
			std::wstringstream ss;
			ss << "EffectLoader: Failed to CreateEffectFromFile!\nPath: " << assetFile;
			std::wcout << ss.str() << "\n";
			return nullptr;
		}
	}

	return pEffect;
}

//--------------------------------------------------
//    Accessors
//--------------------------------------------------
ID3DX11Effect* BaseEffect::GetEffect() const
{
	return m_pEffect;
}
ID3DX11EffectTechnique* BaseEffect::GetTechniqueByName(const std::string& name) const
{
	const auto technique = m_pEffect->GetTechniqueByName(name.c_str());

	if (!technique->IsValid())
		std::wcout << L"Requested Technique with name" << std::wstring{name.begin(), name.end()} << " is not valid\n";

	return technique;
}
ID3DX11EffectTechnique* BaseEffect::GetTechniqueByIndex(int index) const
{
	const auto technique = m_pEffect->GetTechniqueByIndex(index);

	if (!technique->IsValid())
		std::wcout << L"Requested Technique with index" << index << " is not valid\n";

	return technique;
}

//--------------------------------------------------
//    Mutators
//--------------------------------------------------
void BaseEffect::SetWorldViewProjectionMatrix(const Matrix& worldViewProjectionMatrix) const
{
	m_pMatWorldViewProjVariable->SetMatrix(reinterpret_cast<const float*>(&worldViewProjectionMatrix));
}
void BaseEffect::LoadTexture(const std::string& variableName, const Texture* pTexture) const
{
	auto variable = m_pEffect->GetVariableByName(variableName.c_str())->AsShaderResource();
	if (!variable->IsValid())
	{
		std::cout << "Variable " << variableName << " was not valid variable to load a texture in!\n";
		return;
	}

	variable->SetResource(pTexture->GetSRV());
}
#pragma endregion

#pragma region FullShadeEffect
//--------------------------------------------------
//    Constructors and Destructors
//--------------------------------------------------
FullShadeEffect::FullShadeEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
	: BaseEffect(pDevice, assetFile)
{
	if (m_pEffect)
	{
		m_pMatWorldVariable = m_pEffect->GetVariableByName("gWorldMatrix")->AsMatrix();
		if (!m_pMatWorldVariable->IsValid())
			std::wcout << L"m_pMatWorldVariable not valid!\n";
		m_pVecCameraVariable = m_pEffect->GetVariableByName("gCameraPos")->AsVector();
		if (!m_pVecCameraVariable->IsValid())
			std::wcout << L"m_pVecCameraVariable not valid!\n";
	}
}
FullShadeEffect::~FullShadeEffect()
{
	if (m_pVecCameraVariable)		m_pVecCameraVariable->Release();
	if (m_pMatWorldVariable)		m_pMatWorldVariable->Release();
}

//--------------------------------------------------
//    Mutators
//--------------------------------------------------
void FullShadeEffect::SetWorldMatrix(const Matrix& worldMatrix) const
{
	m_pMatWorldVariable->SetMatrix(reinterpret_cast<const float*>(&worldMatrix));
}
void FullShadeEffect::SetCameraPosition(const Vector3& cameraPosition) const
{
	m_pVecCameraVariable->SetFloatVector(reinterpret_cast<const float*>(&cameraPosition));
}
#pragma endregion

#pragma region FlatShadeEffect
//--------------------------------------------------
//    Constructors and Destructors
//--------------------------------------------------
FlatShadeEffect::FlatShadeEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
	: BaseEffect(pDevice, assetFile)
{
	if (m_pEffect)
	{
		m_pMatWorldVariable = m_pEffect->GetVariableByName("gWorldMatrix")->AsMatrix();
		if (!m_pMatWorldVariable->IsValid())
			std::wcout << L"m_pMatWorldVariable not valid!\n";
	}
}
FlatShadeEffect::~FlatShadeEffect()
{
	if (m_pMatWorldVariable) m_pMatWorldVariable->Release();
}

//--------------------------------------------------
//    Mutators
//--------------------------------------------------
void FlatShadeEffect::SetWorldMatrix(const Matrix& worldMatrix) const
{
	m_pMatWorldVariable->SetMatrix(reinterpret_cast<const float*>(&worldMatrix));
}
#pragma endregion
