#pragma once
#include <vector>

#include "Effect.h"
#include "Camera.h"

class Mesh;
struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Update(const Timer* pTimer);
		void Render() const;

	private:
		//--------------------------------------------------
		//    Window
		//--------------------------------------------------
		SDL_Window* m_pWindow{};

		int m_Width{};
		int m_Height{};


		//--------------------------------------------------
		//    Rasterizer General
		//--------------------------------------------------
		bool m_IsInitialized{ false };
		bool m_SoftwareRasterizer{ false };

		std::vector<Mesh*> m_vMeshes{};
		FullShadeEffect* m_pVehicleEffect;
		FlatShadeEffect* m_pFireEffect;

		Camera m_Camera{};

		//--------------------------------------------------
		//    Software Rasterizer
		//--------------------------------------------------


		//--------------------------------------------------
		//    DirectX Rasterizer
		//--------------------------------------------------
		HRESULT InitializeDirectX();

		ID3D11Device* m_pDevice{ nullptr };
		ID3D11DeviceContext* m_pDeviceContext{ nullptr };
		IDXGISwapChain* m_pSwapChain{ nullptr };
		ID3D11Texture2D* m_pDepthStencilBuffer{ nullptr };
		ID3D11DepthStencilView* m_pDepthStencilView{ nullptr };
		ID3D11Resource* m_pRenderTargetBuffer{ nullptr };
		ID3D11RenderTargetView* m_pRenderTargetView{ nullptr };
	};
}
