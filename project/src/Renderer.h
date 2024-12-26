#pragma once
#include <vector>

#include "Effect.h"
#include "Camera.h"
#include "Mesh.h"
#include "Enums.h"

struct SDL_Window;
struct SDL_Surface;

namespace dae
{
	class Renderer final
	{
	public:
		//--------------------------------------------------
		//    Renderer
		//--------------------------------------------------
		Renderer(SDL_Window* pWindow);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;


		//--------------------------------------------------
		//    Renderer
		//--------------------------------------------------
		void Update(const Timer* pTimer);
		void Render();


		//--------------------------------------------------
		//    Rasterizer Shared
		//--------------------------------------------------
		void ToggleRenderMode();
		void ToggleMeshRotation();
		void ToggleUniformColor();

		//--------------------------------------------------
		//    Software Rasterizer
		//--------------------------------------------------
		void CycleShadingMode();
		void CycleCullMode();


		void ToggleDepthBufferVisualization();
		void ToggleNormalMap();
		void ToggleBoundingBox();


		//--------------------------------------------------
		//    DirectX Rasterizer
		//--------------------------------------------------
		void CycleSamplingStates();
		void ToggleFire();

	private:
		//--------------------------------------------------
		//    Window
		//--------------------------------------------------
		SDL_Window* m_pWindow{};

		int m_Width{};
		int m_Height{};

		SDL_Surface* m_pFrontBuffer{ nullptr };
		SDL_Surface* m_pBackBuffer{ nullptr };
		uint32_t* m_pBackBufferPixels{};

		float* m_pDepthBufferPixels{};

		//--------------------------------------------------
		//    Rasterizer Shared
		//--------------------------------------------------
		bool m_SoftwareRasterizer{ true };
		bool m_RotateMesh{ false };
		bool m_DoUniformColor{ false };
		const ColorRGB m_UNIFORM_COLOR = ColorRGB{ 0.1f, 0.1f, 0.1f };

		std::vector<Mesh*> m_vMeshes{};
		FullShadeEffect* m_pVehicleEffect;
		FlatShadeEffect* m_pFireEffect;

		Camera m_Camera{};

		//--------------------------------------------------
		//    Software Rasterizer
		//--------------------------------------------------
		void RenderCPU();
		void DrawBoundingBoxes(const Vector2& min, const Vector2& max);

		void ProjectMeshToNDC(Mesh* mesh) const;
		void RasterizeVertex(VertexOut& vertex) const;
		void InterpolateDepths(float& zDepth, float& wDepth, const std::array<VertexOut, 3>& triangle, const Vector3& weights);
		void InterpolateAllAttributes(const std::array<VertexOut, 3>& triangle, const Vector3& weights, const float wInterpolated, VertexOut& output);

		ColorRGB PixelShading(const VertexOut& v, Mesh* m);

		ShadingMode m_CurrentShadingMode{ ShadingMode::Combined };
		CullMode m_CurrentCullMode{ CullMode::BackFace };
		bool m_DepthBufferVisualization{ false };
		bool m_UseNormalMap{ true };
		bool m_BoundingBoxVisualization{ false };
		const ColorRGB m_SOFTWARE_COLOR = ColorRGB{ 0.39f, 0.39f, 0.39f };

		//--------------------------------------------------
		//    DirectX Rasterizer
		//--------------------------------------------------
		bool m_IsInitialized{ false };
		bool m_FireVisible{ true };
		HRESULT InitializeDirectX();

		const ColorRGB m_HARDWARE_COLOR = ColorRGB{ 0.f, 0.f, 0.3f };

		ID3D11Device* m_pDevice{ nullptr };
		ID3D11DeviceContext* m_pDeviceContext{ nullptr };
		IDXGISwapChain* m_pSwapChain{ nullptr };
		ID3D11Texture2D* m_pDepthStencilBuffer{ nullptr };
		ID3D11DepthStencilView* m_pDepthStencilView{ nullptr };
		ID3D11Resource* m_pRenderTargetBuffer{ nullptr };
		ID3D11RenderTargetView* m_pRenderTargetView{ nullptr };

		SamplerState m_CurrentSamplerState{ SamplerState::Point };

		// Rasterizer States
		ID3D11RasterizerState* m_pRasterizerStateFront = nullptr;
		ID3D11RasterizerState* m_pRasterizerStateBack = nullptr;
		ID3D11RasterizerState* m_pRasterizerStateNone = nullptr;
		ID3D11RasterizerState* m_pCurrentRasterizerState = nullptr;
};
}
