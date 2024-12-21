#include "Texture.h"
#include <iostream>
#include <SDL_image.h>

Texture::Texture(SDL_Surface* pSurface, ID3D11Device* pDevice)
	: m_pSurface{ pSurface }
	, m_pSurfacePixels{ static_cast<uint32_t*>(pSurface->pixels) }
{
	DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM;
	D3D11_TEXTURE2D_DESC desc{};
	desc.Width = pSurface->w;
	desc.Height = pSurface->h;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = format;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA initData;
	initData.pSysMem = pSurface->pixels;
	initData.SysMemPitch = static_cast<UINT>(pSurface->pitch);
	initData.SysMemSlicePitch = static_cast<UINT>(pSurface->h * pSurface->pitch);

	HRESULT hr = pDevice->CreateTexture2D(&desc, &initData, &m_pResource);
	if (FAILED(hr))
	{
		std::wcout << L"Could not create Texture2D!";
		return;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC descSRV{};
	descSRV.Format = format;
	descSRV.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	descSRV.Texture2D.MipLevels = 1;
	hr = pDevice->CreateShaderResourceView(m_pResource, &descSRV, &m_pSRV);
	if (FAILED(hr))
	{
		std::wcout << L"Could not create SRV!";
		return;
	}
}

Texture::~Texture()
{
	if (m_pSurface)
	{
		SDL_FreeSurface(m_pSurface);
		m_pSurface = nullptr;
	}

	if (m_pSRV)			m_pSRV->Release();
	if (m_pResource)	m_pResource->Release();
}

Texture* Texture::LoadFromFile(const std::string& path, ID3D11Device* pDevice)
{
	SDL_Surface* pSurface = IMG_Load(path.c_str());
	if (pSurface == nullptr)
	{
		std::cerr << "Texture::LoadFromFile > Failed to load texture: " << path << " Error: " << IMG_GetError() << "\n";
		throw std::runtime_error("Failed to load texture");
	}

	return new Texture(pSurface, pDevice);
}

ID3D11ShaderResourceView* Texture::GetSRV() const
{
	return m_pSRV;
}

ColorRGB Texture::Sample(const Vector2& uv) const
{
	// Set the default return color to black
	ColorRGB returnColor{ 0, 0, 0 };

	// Wrap the UV coordinates
	float u = uv.x - std::floor(uv.x);
	float v = uv.y - std::floor(uv.y);

	// Since our UV coordinates are of values between [0; 1] and SDL requests pixel indexes,
	// we multiply the UV with width and height of the background
	int x = u * m_pSurface->w;
	int y = v * m_pSurface->h;

	// Bytes per pixel
	const Uint8 bytesPerPixel = m_pSurface->format->BytesPerPixel;

	// Retrieve the address of the pixel we need (startAddress + y * width + x * bpp)
	Uint8* pPixelAddr = (Uint8*)m_pSurfacePixels + y * m_pSurface->pitch + x * bytesPerPixel;
	// Convert the pixelAddress to a Uint32, since that is what the SDL_GetRGB expects
	Uint32 pixelData = *(Uint32*)pPixelAddr;

	SDL_Color Color = { 0x00, 0x00, 0x00 };

	// Retrieve the RGB values of the specific pixel
	SDL_GetRGB(pixelData, m_pSurface->format, &Color.r, &Color.g, &Color.b);

	// Set our returnColor to the color SDL gave us
	returnColor.r = Color.r;
	returnColor.g = Color.g;
	returnColor.b = Color.b;
	// SDL uses colors in ranges 0-255, we use ranges 0-1, therefore we divide our returnColor by 255
	returnColor /= 255.f;

	// return the background color of the pixel
	return returnColor;
}
