#include "pch.h"
#include "Renderer.h"
#include "Utils.h"
#include <algorithm>
#include <array>
#include <execution>
#include <iostream>
#include "ConsoleTextSettings.h"

namespace dae {

	Renderer::Renderer(SDL_Window* pWindow) :
		m_pWindow(pWindow)
	{
		//Initialize
		SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

		// Create Buffers
		m_pFrontBuffer = SDL_GetWindowSurface(pWindow);
		m_pBackBuffer = SDL_CreateRGBSurface(0, m_Width, m_Height, 32, 0, 0, 0, 0);
		m_pBackBufferPixels = (uint32_t*)m_pBackBuffer->pixels;

		m_pDepthBufferPixels = new float[m_Width * m_Height];

		//Initialize DirectX pipeline
		const HRESULT result = InitializeDirectX();
		if (result == S_OK)
		{
			m_IsInitialized = true;
			std::cout << "DirectX is initialized and ready!\n";
		}
		else
		{
			std::cout << "DirectX initialization failed!\n";
		}

		m_pVehicleEffect = new FullShadeEffect(m_pDevice, L"resources/PosCol3D.fx");
		m_vMeshes.push_back(new Mesh(m_pDevice, "resources/vehicle.obj", m_pVehicleEffect));
		m_vMeshes.back()->LoadDiffuseTexture("resources/vehicle_diffuse.png", m_pDevice);
		m_vMeshes.back()->LoadNormalMap("resources/vehicle_normal.png", m_pDevice);
		m_vMeshes.back()->LoadSpecularMap("resources/vehicle_specular.png", m_pDevice);
		m_vMeshes.back()->LoadGlossinessMap("resources/vehicle_gloss.png", m_pDevice);

		m_pFireEffect = new FlatShadeEffect(m_pDevice, L"resources/Fire.fx");
		m_vMeshes.push_back(new Mesh(m_pDevice, "resources/fireFX.obj", m_pFireEffect));
		m_vMeshes.back()->LoadDiffuseTexture("resources/fireFX_diffuse.png", m_pDevice);

		m_Camera.Initialize(45.f, { 0.f, 0.f , -10.f }, m_Width / static_cast<float>(m_Height), 0.1f, 1000.f);
	}

	Renderer::~Renderer()
	{
		for (auto& mesh : m_vMeshes)
		{
			delete mesh;
		}

		if (m_pRenderTargetView)		m_pRenderTargetView->Release();
		if (m_pRenderTargetBuffer)		m_pRenderTargetBuffer->Release();
		if (m_pDepthStencilView)		m_pDepthStencilView->Release();
		if (m_pDepthStencilBuffer)		m_pDepthStencilBuffer->Release();
		if (m_pSwapChain)				m_pSwapChain->Release();
		if (m_pDeviceContext)
		{
			m_pDeviceContext->ClearState();
			m_pDeviceContext->Flush();
			m_pDeviceContext->Release();
		}
		if (m_pDevice) m_pDevice->Release();

		if(m_pRasterizerStateFront) m_pRasterizerStateFront->Release();
		if (m_pRasterizerStateBack) m_pRasterizerStateBack->Release();
		if (m_pRasterizerStateNone) m_pRasterizerStateNone->Release();

		delete[] m_pDepthBufferPixels;

	}

	void Renderer::Update(const Timer* pTimer)
	{
		m_Camera.Update(pTimer);

		m_pVehicleEffect->SetWorldMatrix(m_vMeshes[0]->GetWorldMatrix());

		m_pVehicleEffect->SetCameraPosition(m_Camera.origin);

		if (m_RotateMesh)
		{
			for (auto& m : m_vMeshes)
			{
				constexpr float rotationSpeedRadians = 1;
				m->SetWorldMatrix(Matrix::CreateRotationY(pTimer->GetElapsed() * rotationSpeedRadians)
											* m->GetWorldMatrix());
			}
		}
		Matrix wvpm = m_vMeshes[0]->GetWorldMatrix() * m_Camera.GetViewMatrix() * m_Camera.GetProjectionMatrix();
		m_pVehicleEffect->SetWorldViewProjectionMatrix(wvpm);
		wvpm = m_vMeshes[1]->GetWorldMatrix() * m_Camera.GetViewMatrix() * m_Camera.GetProjectionMatrix();
		m_pFireEffect->SetWorldViewProjectionMatrix(wvpm);

	}


	void Renderer::Render()
	{
		if (!m_IsInitialized)
			return;

		const ColorRGB fillColor = m_DoUniformColor ? m_UNIFORM_COLOR : (m_SoftwareRasterizer ? m_SOFTWARE_COLOR : m_HARDWARE_COLOR);
		if (m_SoftwareRasterizer)
		{
			// @START
			SDL_FillRect(m_pBackBuffer, NULL, SDL_MapRGB(m_pBackBuffer->format, 
				255 * fillColor.r,
				255 * fillColor.g,
				255 * fillColor.b));
			std::fill(&m_pDepthBufferPixels[0], &m_pDepthBufferPixels[m_Width * m_Height], 1);

			// Lock BackBuffer
			SDL_LockSurface(m_pBackBuffer);
		}
		else
		{
			// 1. CLEAR RTV & DSV
			const float color[4] = { fillColor.r, fillColor.g, fillColor.b, 1.f };
			m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, color);
			m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
		}


		if (m_SoftwareRasterizer)
		{
			RenderCPU();
		}
		else
		{
			m_pDeviceContext->RSSetState(m_pCurrentRasterizerState);
			// 2. SET PIPELINE + INVOKE DRAW CALLS (= RENDER)
			for (int meshIdx{}; meshIdx < (m_FireVisible + 1); ++meshIdx)
			{
				Mesh* currentMesh = m_vMeshes[meshIdx];
				currentMesh->Render(m_pDeviceContext, m_SoftwareRasterizer);
			}
		}

		if (m_SoftwareRasterizer)
		{
			// @END
			SDL_UnlockSurface(m_pBackBuffer);
			SDL_BlitSurface(m_pBackBuffer, 0, m_pFrontBuffer, 0);
			SDL_UpdateWindowSurface(m_pWindow);
		}
		else
		{
			// 3. PRESENT BACKBUFFER (SWAP)
			m_pSwapChain->Present(0, 0);
		}
	}

	void Renderer::ToggleRenderMode()
	{
		m_SoftwareRasterizer = !m_SoftwareRasterizer;
		std::cout << DARK_YELLOW_TXT << "**(SHARED) Rasterizer Mode = " << (m_SoftwareRasterizer ? "SOFTWARE" : "HARDWARE") << "\n";
	}

	void Renderer::ToggleMeshRotation()
	{
		m_RotateMesh = !m_RotateMesh;
		std::cout << DARK_YELLOW_TXT << "**(SHARED) Vehicle Rotations = " << (m_RotateMesh ? "ON" : "OFF") << "\n";
	}

	void Renderer::ToggleUniformColor()
	{
		m_DoUniformColor = !m_DoUniformColor;
		std::cout << DARK_YELLOW_TXT << "**(SHARED) Uniform ClearColor = " << (m_DoUniformColor ? "ON" : "OFF") << "\n";
	}

	void Renderer::CycleShadingMode()
	{
		switch (m_CurrentShadingMode)
		{
		case ShadingMode::ObservedArea:
			m_CurrentShadingMode = ShadingMode::Diffuse;
			std::cout << DARK_MAGENTA_TXT << "**(SOFTWARE) Shading Mode = " << "DIFFUSE" << "\n";
			break;
		case ShadingMode::Diffuse:
			m_CurrentShadingMode = ShadingMode::Specular;
			std::cout << DARK_MAGENTA_TXT << "**(SOFTWARE) Shading Mode = " << "SPECULAR" << "\n";
			break;
		case ShadingMode::Specular:
			m_CurrentShadingMode = ShadingMode::Combined;
			std::cout << DARK_MAGENTA_TXT << "**(SOFTWARE) Shading Mode = " << "COMBINED" << "\n";
			break;
		case ShadingMode::Combined:
			m_CurrentShadingMode = ShadingMode::ObservedArea;
			std::cout << DARK_MAGENTA_TXT << "**(SOFTWARE) Shading Mode = " << "OBSERVED_AREA" << "\n";
			break;
		default:
			break;
		}
	}

	void Renderer::CycleCullMode()
	{
		switch (m_CurrentCullMode)
		{
		case CullMode::BackFace:
			m_CurrentCullMode = CullMode::FrontFace;
			m_pCurrentRasterizerState = m_pRasterizerStateFront;
			std::cout << DARK_YELLOW_TXT << "**(SHARED) CullMode = " << "FRONT" << "\n";
			break;
		case CullMode::FrontFace:
			m_CurrentCullMode = CullMode::None;
			m_pCurrentRasterizerState = m_pRasterizerStateNone;
			std::cout << DARK_YELLOW_TXT << "**(SHARED) CullMode = " << "NONE" << "\n";
			break;
		case CullMode::None:
			m_CurrentCullMode = CullMode::BackFace;
			std::cout << DARK_YELLOW_TXT << "**(SHARED) CullMode = " << "BACK" << "\n";
			m_pCurrentRasterizerState = m_pRasterizerStateBack;
			break;
		default:
			break;
		}
	}

	void Renderer::ToggleDepthBufferVisualization()
	{
		m_DepthBufferVisualization = !m_DepthBufferVisualization;
		std::cout << DARK_MAGENTA_TXT << "**(SOFTWARE) DepthBuffer Visualization = " << (m_DepthBufferVisualization ? "ON" : "OFF") << "\n";
	}

	void Renderer::ToggleNormalMap()
	{
		m_UseNormalMap = !m_UseNormalMap;
		std::cout << DARK_MAGENTA_TXT << "**(SOFTWARE) NormalMap = " << (m_UseNormalMap ? "ON" : "OFF") << "\n";
	}

	void Renderer::ToggleBoundingBox()
	{
		m_BoundingBoxVisualization = !m_BoundingBoxVisualization;
		std::cout << DARK_MAGENTA_TXT << "**(SOFTWARE) BoundingBox Visualization = " << (m_BoundingBoxVisualization ? "ON" : "OFF") << "\n";
	}

	void Renderer::CycleSamplingStates()
	{
		switch (m_CurrentSamplerState)
		{
		case SamplerState::Point:
			m_CurrentSamplerState = SamplerState::Linear;
			std::cout << DARK_GREEN_TXT << "**(HARDWARE) Sampler Filter = " << "LINEAR" << "\n";
			break;
		case SamplerState::Linear:
			m_CurrentSamplerState = SamplerState::Anisotropic;
			std::cout << DARK_GREEN_TXT << "**(HARDWARE) Sampler Filter = " << "ANISOTROPIC" << "\n";
			break;
		case SamplerState::Anisotropic:
			m_CurrentSamplerState = SamplerState::Point;
			std::cout << DARK_GREEN_TXT << "**(HARDWARE) Sampler Filter = " << "POINT" << "\n";
			break;
		default:
			m_CurrentSamplerState = SamplerState::Linear;
			break;
		}	

		for (const auto& m : m_vMeshes)
		{
			m->SetTextureSamplingState(m_CurrentSamplerState);
		}
	}

	void Renderer::ToggleFire()
	{
		m_FireVisible = !m_FireVisible;
		std::cout << DARK_GREEN_TXT << "**(HARDWARE) FireFX = " << (m_FireVisible ? "ON" : "OFF") << "\n";
	}

	void Renderer::RenderCPU()
	{
		// predefine a triangle we can reuse
		std::array<VertexOut, 3> triangleNDC{};
		std::array<VertexOut, 3> triangleRasterVertices{};

		for (int triangleMeshIndex{}; triangleMeshIndex < 1; ++triangleMeshIndex)
		{
			Mesh* currentMesh = m_vMeshes[triangleMeshIndex];
			auto& verticesOut = currentMesh->GetVerticesOutByReference();
			auto& vertices = currentMesh->GetVerticesByReference();
			auto& indices = currentMesh->GetIndicesByReference();
			auto& worldMatrix = currentMesh->GetWorldMatrix();
			auto primitiveTopology = currentMesh->GetPrimitiveTopology();

			int indexJump = 0;
			int triangleCount = 0;
			bool triangleStripMethod = false;

			// Determine the triangle count and index jump depending on the PrimitiveTopology
			if (primitiveTopology == PrimitiveTopology::TriangleList)
			{
				indexJump = 3;
				triangleCount = indices.size() / 3;
				triangleStripMethod = false;
			}
			else if (primitiveTopology == PrimitiveTopology::TriangleStrip)
			{
				indexJump = 1;
				triangleCount = indices.size() - 2;
				triangleStripMethod = true;
			}

			// Project the entire mesh to NDC coordinates
			ProjectMeshToNDC(currentMesh);

			// Loop over all the triangles
			for (int triangleIndex{}; triangleIndex < triangleCount; ++triangleIndex)
			{
				uint32_t indexPos0 = indices[indexJump * triangleIndex + 0];
				uint32_t indexPos1 = indices[indexJump * triangleIndex + 1];
				uint32_t indexPos2 = indices[indexJump * triangleIndex + 2];
				// Skip if duplicate indices
				if (indexPos0 == indexPos1 or indexPos0 == indexPos2 or indexPos1 == indexPos2) continue;
				// If the triangle strip method is in use, swap the indices of odd indexed triangles
				if (triangleStripMethod and (triangleIndex & 1)) std::swap(indexPos1, indexPos2);

				// Define triangle in NDC
				triangleNDC[0] = verticesOut[indexPos0];
				triangleNDC[1] = verticesOut[indexPos1];
				triangleNDC[2] = verticesOut[indexPos2];
				// Calculate the minimum depth if the current triangle, which we will use later for early-depth test
				const float minDepth = std::min({triangleNDC[0].position.z, triangleNDC[1].position.z, triangleNDC[2].position.z});

				// Cull the triangle if one or more of the NDC vertices are outside the frustum
				if (!IsNDCTriangleInFrustum(triangleNDC[0])) continue;
				if (!IsNDCTriangleInFrustum(triangleNDC[1])) continue;
				if (!IsNDCTriangleInFrustum(triangleNDC[2])) continue;

				// Rasterize the vertices
				RasterizeVertex(verticesOut[indexPos0]);
				RasterizeVertex(verticesOut[indexPos1]);
				RasterizeVertex(verticesOut[indexPos2]);

				// Define triangle in RasterSpace
				triangleRasterVertices[0] = verticesOut[indexPos0];
				triangleRasterVertices[1] = verticesOut[indexPos1];
				triangleRasterVertices[2] = verticesOut[indexPos2];
				const Vector2& v0 = triangleRasterVertices[0].position.GetXY();
				const Vector2& v1 = triangleRasterVertices[1].position.GetXY();
				const Vector2& v2 = triangleRasterVertices[2].position.GetXY();

				//if (m_DrawWireFrames)
				//{
				//	ColorRGB wireFrameColor = colors::White * Remap01(minDepth, 0.998f, 1.f);

				//	DrawLine(v0.x, v0.y, v1.x, v1.y, wireFrameColor);
				//	DrawLine(v1.x, v1.y, v2.x, v2.y, wireFrameColor);
				//	DrawLine(v2.x, v2.y, v0.x, v0.y, wireFrameColor);

				//	continue;
				//}

				// Define the triangle's bounding box
				Vector2 min = { FLT_MAX,  FLT_MAX };
				Vector2 max = { -FLT_MAX, -FLT_MAX };
				{
					// Minimums
					min = Vector2::Min(min, v0);
					min = Vector2::Min(min, v1);
					min = Vector2::Min(min, v2);
					// Clamp between screen min and max, but also make sure that, due to floating point -> int rounding happens correct
					min.x = std::clamp(std::floor(min.x), 0.f, m_Width - 1.f);
					min.y = std::clamp(std::floor(min.y), 0.f, m_Height - 1.f);

					// Maximums
					max = Vector2::Max(max, v0);
					max = Vector2::Max(max, v1);
					max = Vector2::Max(max, v2);
					// Clamp between screen min and max, but also make sure that, due to floating point -> int rounding happens correct
					max.x = std::clamp(std::ceil(max.x), 0.f, m_Width - 1.f);
					max.y = std::clamp(std::ceil(max.y), 0.f, m_Height - 1.f);
				}

				if (m_BoundingBoxVisualization)
				{
					DrawBoundingBoxes(min, max);
					continue;
				}

				// Pre-calculate the inverse area of the triangle so this doesn't need to happen for
				// every pixels once we calculate the barycentric coordinates (as the triangle area won't change)
				float area = Vector2::Cross(v1 - v0, v2 - v0);
				area = abs(area);
				float invArea = 1.f / area;


				// For every pixel (within the bounding box)
				for (int py{ int(min.y) }; py < int(max.y); ++py)
				{
					for (int px{ int(min.x) }; px < int(max.x); ++px)
					{
						// Do an early depth test!!
						// If the minimum depth of our triangle is already bigger than what is stored in the depth buffer (at a current pixel),
						// there is no chance that that pixel inside the triangle will be closer, so we just skip to the next pixel
						if (minDepth > m_pDepthBufferPixels[m_Width * py + px]) continue;

						// Declare finalColor of the pixel
						ColorRGB finalColor{};

						// Declare wInterpolated and zBufferValue of this pixel
						float wInterpolated{ FLT_MAX };
						float zBufferValue{ FLT_MAX };

						// Calculate the barycentric coordinates of that pixel in relationship to the triangle,
						// these barycentric coordinates CAN be invalid (point outside triangle)
						Vector2 pixelCoord = Vector2(px + 0.5f, py + 0.5f);
						Vector3 barycentricCoords = CalculateBarycentricCoordinates(
							v0, v1, v2, pixelCoord, invArea);

						// Check if our barycentric coordinates are valid, if not, skip to the next pixel
						if (!AreBarycentricValid(barycentricCoords, m_CurrentCullMode)) continue;

						// Now we interpolated both our Z and W depths
						InterpolateDepths(zBufferValue, wInterpolated, triangleRasterVertices, barycentricCoords);
						if (zBufferValue < 0 or zBufferValue > 1) continue; // if z-depth is outside of frustum, skip to next pixel
						if (wInterpolated < 0) continue; // if w-depth is negative (behind camera), skip to next pixel

						// If out current value in the zBuffer is smaller than our new one, skip to the next pixel
						if (zBufferValue > m_pDepthBufferPixels[m_Width * py + px]) continue;

						// Now that we are sure our z-depth is smaller than the one in the zBuffer, we can update the zBuffer and interpolate the attributes
						m_pDepthBufferPixels[m_Width * py + px] = zBufferValue;

						// Correctly interpolated attributes
						VertexOut interpolatedAttributes{};
						InterpolateAllAttributes(triangleRasterVertices, barycentricCoords, wInterpolated, interpolatedAttributes);
						interpolatedAttributes.position.z = zBufferValue;
						interpolatedAttributes.position.w = wInterpolated;

						finalColor = PixelShading(interpolatedAttributes, currentMesh);

						if (m_DepthBufferVisualization)
						{
							const float remappedZ = Remap01(m_pDepthBufferPixels[m_Width * py + px], 0.998f, 1);
							finalColor = ColorRGB{ remappedZ , remappedZ , remappedZ };
						}

						// Make sure our colors are within the correct 0-1 range (while keeping relative differences)
						finalColor.MaxToOne();


						//Update Color in Buffer
						m_pBackBufferPixels[m_Width * py + px] = SDL_MapRGB(m_pBackBuffer->format,
							static_cast<uint8_t>(finalColor.r * 255),
							static_cast<uint8_t>(finalColor.g * 255),
							static_cast<uint8_t>(finalColor.b * 255));
					}
				}
			}
		}
	}

	void Renderer::DrawBoundingBoxes(const Vector2& min, const Vector2& max)
	{
		for (int py{ int(min.y) }; py < int(max.y); ++py)
		{
			for (int px{ int(min.x) }; px < int(max.x); ++px)
			{
				m_pBackBufferPixels[m_Width * py + px] = SDL_MapRGB(m_pBackBuffer->format,
					255, 255, 255);
			}
		}
	}

	HRESULT Renderer::InitializeDirectX()
	{
		// 1. Create Device & DeviceContext
		//=====
		D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_1;
		uint32_t createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		HRESULT result = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, createDeviceFlags, &featureLevel,
			1, D3D11_SDK_VERSION, &m_pDevice, nullptr, &m_pDeviceContext);
		if (FAILED(result))
			return result;

		// Create DXGI Factory
		IDXGIFactory1* pDxgiFactory{};
		result = CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&pDxgiFactory));
		if (FAILED(result))
			return result;

		// 2. Create Swapchain
		//=====
		DXGI_SWAP_CHAIN_DESC swapChainDesc{};
		swapChainDesc.BufferDesc.Width = m_Width;
		swapChainDesc.BufferDesc.Height = m_Height;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 1;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 60;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 1;
		swapChainDesc.Windowed = true;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swapChainDesc.Flags = 0;

		// Get the handle (HWND) from the SDL Backbuffer
		SDL_SysWMinfo sysWMInfo{};
		SDL_GetVersion(&sysWMInfo.version);
		SDL_GetWindowWMInfo(m_pWindow, &sysWMInfo);
		swapChainDesc.OutputWindow = sysWMInfo.info.win.window;

		// Create SwapChain
		result = pDxgiFactory->CreateSwapChain(m_pDevice, &swapChainDesc, &m_pSwapChain);
		if (FAILED(result))
			return result;

		// 3. Create DepthStencil (DS) & DepthStencilView (DSV)
		// Resource
		D3D11_TEXTURE2D_DESC depthStencilDesc{};
		depthStencilDesc.Width = m_Width;
		depthStencilDesc.Height = m_Height;
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthStencilDesc.CPUAccessFlags = 0;
		depthStencilDesc.MiscFlags = 0;

		// View
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
		depthStencilViewDesc.Format = depthStencilDesc.Format;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;

		result = m_pDevice->CreateTexture2D(&depthStencilDesc, nullptr, &m_pDepthStencilBuffer);
		if (FAILED(result))
			return result;

		result = m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer, &depthStencilViewDesc, &m_pDepthStencilView);
		if (FAILED(result))
			return result;

		// 4. Create RenderTarget (RT) & RenderTargetView (RTV)
		//=====

		// Resource
		result = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_pRenderTargetBuffer));
		if (FAILED(result))
			return result;

		// View
		result = m_pDevice->CreateRenderTargetView(m_pRenderTargetBuffer, nullptr, &m_pRenderTargetView);
		if (FAILED(result))
			return result;

		// 5. Bind RTV & DSV to Output Merger Stage
		//=====
		m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

		// 6. Set Viewport
		//=====
		D3D11_VIEWPORT viewport{};
		viewport.Width = static_cast<float>(m_Width);
		viewport.Height = static_cast<float>(m_Height);
		viewport.TopLeftX = 0.f;
		viewport.TopLeftY = 0.f;
		viewport.MinDepth = 0.f;
		viewport.MaxDepth = 1.f;
		m_pDeviceContext->RSSetViewports(1, &viewport);


		// 7. Rasterizer States
		//=====
		D3D11_RASTERIZER_DESC rasterDesc = {};
		rasterDesc.FillMode = D3D11_FILL_SOLID;
		rasterDesc.CullMode = D3D11_CULL_FRONT;

		// Front Face Culling Rasterizer State
		result = m_pDevice->CreateRasterizerState(&rasterDesc, &m_pRasterizerStateFront);
		if (FAILED(result))
			return result;

		// Back Face Culling Rasterizer State
		rasterDesc.CullMode = D3D11_CULL_BACK; // Cull back faces
		result = m_pDevice->CreateRasterizerState(&rasterDesc, &m_pRasterizerStateBack);
		if (FAILED(result))
			return result;

		// No Culling Rasterizer State
		rasterDesc.CullMode = D3D11_CULL_NONE; // No culling
		result = m_pDevice->CreateRasterizerState(&rasterDesc, &m_pRasterizerStateNone);
		if (FAILED(result))
			return result;

		return S_OK;
	}
}
void Renderer::ProjectMeshToNDC(Mesh* mesh) const
{
	auto& verticesOut = mesh->GetVerticesOutByReference();
	auto& vertices = mesh->GetVerticesByReference();
	auto& worldMatrix = mesh->GetWorldMatrix();

	verticesOut.resize(vertices.size());

	// Calculate the transformation matrix
	Matrix worldViewProjectionMatrix = worldMatrix * m_Camera.viewMatrix * m_Camera.projectionMatrix;

	for (int index{}; index < verticesOut.size(); ++index)
	{
		// Transform every vertex
		Vector4 transformedPosition = worldViewProjectionMatrix.TransformPoint(vertices[index].position.ToPoint4());
		verticesOut[index].position = transformedPosition;

		if (verticesOut[index].position.w <= 0) continue;

		// Perform the perspective divide
		float invW = 1.f / transformedPosition.w;
		verticesOut[index].position.x *= invW;
		verticesOut[index].position.y *= invW;
		verticesOut[index].position.z *= invW;


		// Update the other attributes
		verticesOut[index].color = vertices[index].color;
		verticesOut[index].uv = vertices[index].uv;

		verticesOut[index].normal = worldMatrix.TransformVector(vertices[index].normal).Normalized();
		verticesOut[index].tangent = worldMatrix.TransformVector(vertices[index].tangent).Normalized();
		verticesOut[index].viewDir = (worldMatrix.TransformPoint(verticesOut[index].position)
			- m_Camera.origin.ToPoint4()).Normalized();
	}
}
void Renderer::RasterizeVertex(VertexOut& vertex) const
{
	vertex.position.x = (1.f + vertex.position.x) * 0.5f * m_Width;
	vertex.position.y = (1.f - vertex.position.y) * 0.5f * m_Height;
}

void Renderer::InterpolateDepths(float& zDepth, float& wDepth, const std::array<VertexOut, 3>& triangle, const Vector3& weights)
{
	// Now we interpolated both our Z and W depths
	const float& Z0 = triangle[0].position.z;
	const float& Z1 = triangle[1].position.z;
	const float& Z2 = triangle[2].position.z;
	zDepth = InterpolateDepth(Z0, Z1, Z2, weights);

	const float& W0 = triangle[0].position.w;
	const float& W1 = triangle[1].position.w;
	const float& W2 = triangle[2].position.w;
	wDepth = InterpolateDepth(W0, W1, W2, weights);
}
void Renderer::InterpolateAllAttributes(const std::array<VertexOut, 3>& triangle, const Vector3& weights, const float wInterpolated, VertexOut& output)
{
	// Get W components
	const float& W0 = triangle[0].position.w;
	const float& W1 = triangle[1].position.w;
	const float& W2 = triangle[2].position.w;

	// Correctly interpolated position
	const Vector4& P0 = triangle[0].position;
	const Vector4& P1 = triangle[1].position;
	const Vector4& P2 = triangle[2].position;
	output.position = InterpolateAttribute(P0, P1, P2, W0, W1, W2, wInterpolated, weights);

	// Correctly interpolated color
	const ColorRGB& C0 = triangle[0].color;
	const ColorRGB& C1 = triangle[1].color;
	const ColorRGB& C2 = triangle[2].color;
	output.color = InterpolateAttribute(C0, C1, C2, W0, W1, W2, wInterpolated, weights);

	// Correctly interpolated uv
	const Vector2& UV0 = triangle[0].uv;
	const Vector2& UV1 = triangle[1].uv;
	const Vector2& UV2 = triangle[2].uv;
	output.uv = InterpolateAttribute(UV0, UV1, UV2, W0, W1, W2, wInterpolated, weights);

	// Correctly interpolated normal
	const Vector3& N0 = triangle[0].normal;
	const Vector3& N1 = triangle[1].normal;
	const Vector3& N2 = triangle[2].normal;
	output.normal = InterpolateAttribute(N0, N1, N2, W0, W1, W2, wInterpolated, weights);
	output.normal.Normalize();

	// Correctly interpolated tangent
	const Vector3& T0 = triangle[0].tangent;
	const Vector3& T1 = triangle[1].tangent;
	const Vector3& T2 = triangle[2].tangent;
	output.tangent = InterpolateAttribute(T0, T1, T2, W0, W1, W2, wInterpolated, weights);
	output.tangent.Normalize();

	// Correctly interpolated viewDirection
	const Vector3& VD0 = triangle[0].viewDir;
	const Vector3& VD1 = triangle[1].viewDir;
	const Vector3& VD2 = triangle[2].viewDir;
	output.viewDir = InterpolateAttribute(VD0, VD1, VD2, W0, W1, W2, wInterpolated, weights);
	output.viewDir.Normalize();
}

ColorRGB Renderer::PixelShading(const VertexOut& v, Mesh* m)
{
	// Ambient Color
	constexpr ColorRGB ambient = { 0.03f, 0.03f, 0.03f };

	// Set up the light
	const Vector3 lightDirection = { 0.577f , -0.577f , 0.577f };
	const Vector3 directionToLight = -lightDirection.Normalized();

	// Sample the normal
	Vector3 sampledNormal;
	if (m_UseNormalMap)		sampledNormal = m->SampleNormalMap(v.normal, v.tangent, v.uv);
	else					sampledNormal = v.normal;

	// Calculate the observed area
	const float observedArea = Vector3::Dot(sampledNormal, directionToLight);
	// Skipping if observedArea < 0 happens when we check for the Current Shading Mode, as we really only want to skip (return black) if
	// We are actually in a mode that uses observedArea (ObservedArea and Combined)

	// Calculate the lambert diffuse color
	const ColorRGB cd = m->SampleDiffuse(v.uv);
	constexpr float kd = 7.f;
	const ColorRGB lambertDiffuse = (cd * kd) * ONE_DIV_PI;

	// Calculate the specular
	constexpr float shininess = 25.f;
	const ColorRGB specular = m->SamplePhong(directionToLight, v.viewDir, sampledNormal, v.uv, shininess);


	switch (m_CurrentShadingMode)
	{
	case ShadingMode::ObservedArea:
		if (observedArea <= 0.f) return{};
		return ColorRGB{ observedArea, observedArea, observedArea };
		break;
	case ShadingMode::Diffuse:
		return lambertDiffuse;
		break;
	case ShadingMode::Specular:
		return specular;
		break;
	case ShadingMode::Combined:
		if (observedArea <= 0.f) return{};
		return (lambertDiffuse + specular + ambient) * observedArea;
		break;
	default:
		return (lambertDiffuse + specular + ambient) * observedArea;
		break;
	}
}
