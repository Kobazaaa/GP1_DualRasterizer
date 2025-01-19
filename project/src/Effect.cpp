#include "Effect.h"

#pragma region Effect
//--------------------------------------------------
//    Constructors and Destructors
//--------------------------------------------------
Effect::Effect(ID3D11Device* pDevice, const std::wstring& assetFile)
{
	m_pEffect = LoadEffect(pDevice, assetFile);
	if (!m_pEffect)
	{
		std::wcout << L"Effect was not loaded correctly!\n";
	}
}
Effect::~Effect()
{
	if (m_pEffect) m_pEffect->Release();
}

//--------------------------------------------------
//    Effect Loader
//--------------------------------------------------
ID3DX11Effect* Effect::LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
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
ID3DX11Effect* Effect::GetEffect() const
{
	return m_pEffect;
}
ID3DX11EffectTechnique* Effect::GetTechniqueByName(const std::string& name) const
{
	const auto technique = m_pEffect->GetTechniqueByName(name.c_str());

	if (!technique->IsValid())
		std::wcout << L"Requested Technique with name" << std::wstring{name.begin(), name.end()} << " is not valid\n";

	return technique;
}
ID3DX11EffectTechnique* Effect::GetTechniqueByIndex(int index) const
{
	const auto technique = m_pEffect->GetTechniqueByIndex(index);

	if (!technique->IsValid())
		std::wcout << L"Requested Technique with index" << index << " is not valid\n";

	return technique;
}

//--------------------------------------------------
//    Mutators
//--------------------------------------------------
void Effect::LoadTexture(const std::string& variableName, const Texture* pTexture) const
{
	auto variable = m_pEffect->GetVariableByName(variableName.c_str())->AsShaderResource();
	if (!variable->IsValid())
	{
		std::cout << "Variable " << variableName << " was not valid variable to load a texture in!\n";
		return;
	}

	variable->SetResource(pTexture->GetSRV());
}

void Effect::SetMatrixByName(const std::string& variableName, const Matrix& m) const
{
	auto variable = m_pEffect->GetVariableByName(variableName.c_str())->AsMatrix();
	if (!variable->IsValid())
	{
		std::cout << "Variable " << variableName << " was not valid variable!\n";
		return;
	}

	variable->SetMatrix(reinterpret_cast<const float*>(&m));
}

void Effect::SetVector3ByName(const std::string& variableName, const Vector3& v) const
{
	auto variable = m_pEffect->GetVariableByName(variableName.c_str())->AsVector();
	if (!variable->IsValid())
	{
		std::cout << "Variable " << variableName << " was not valid variable!\n";
		return;
	}

	variable->SetFloatVector(reinterpret_cast<const float*>(&v));
}

void Effect::SetShaderResourceView(const std::string& variableName, ID3D11ShaderResourceView* pSRV) const
{
	auto variable = m_pEffect->GetVariableByName(variableName.c_str())->AsShaderResource();
	if (!variable->IsValid())
	{
		std::cout << "Variable " << variableName << " was not valid variable to load a Shader Resource in!\n";
		return;
	}

	variable->SetResource(pSRV);
}
#pragma endregion
