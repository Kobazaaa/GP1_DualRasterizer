#pragma once
#include "pch.h"
#include <SDL_surface.h>
#include <string>
#include "ColorRGB.h"

using namespace dae;

class Texture final
{
public:
	//--------------------------------------------------
	//    Destructor
	//--------------------------------------------------
	~Texture();

	Texture(const Texture& other) = delete;
	Texture(Texture&& other) noexcept = delete;
	Texture& operator=(const Texture& other) = delete;
	Texture& operator=(Texture&& other) noexcept = delete;

	//--------------------------------------------------
	//    Texture Loader
	//--------------------------------------------------
	static Texture* LoadFromFile(const std::string& path, ID3D11Device* pDevice = nullptr);

	//--------------------------------------------------
	//    Accessors
	//--------------------------------------------------
	ID3D11ShaderResourceView* GetSRV() const;
	ColorRGB Sample(const Vector2& uv, bool sampleAlpha = false, float* alpha = nullptr) const;

private:
	//--------------------------------------------------
	//    Constructor
	//--------------------------------------------------
	Texture(SDL_Surface* pSurface, ID3D11Device* pDevice);

	//--------------------------------------------------
	//    Texture Data
	//--------------------------------------------------
	SDL_Surface* m_pSurface{ nullptr };
	uint32_t* m_pSurfacePixels{ nullptr };

	//--------------------------------------------------
	//    DirectX Data
	//--------------------------------------------------
	ID3D11ShaderResourceView* m_pSRV{};
	ID3D11Texture2D* m_pResource{};
};
