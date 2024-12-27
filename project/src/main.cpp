#include "pch.h"

#if defined(_DEBUG)
#include "vld.h"
#endif

#undef main
#include "Renderer.h"
#include "ConsoleTextSettings.h"

using namespace dae;

void ShutDown(SDL_Window* pWindow)
{
	SDL_DestroyWindow(pWindow);
	SDL_Quit();
}

void PrintInfo()
{
	std::cout << DARK_YELLOW_TXT;
	std::cout << "[Key Bindings - SHARED]\n";
	std::cout << "   [F1]  Toggle Rasterizer Mode (HARDWARE/SOFTWARE)\n";
	std::cout << "   [F2]  Toggle Vehicle Rotation (ON/OFF)\n";
	std::cout << "   [F9]  Cycle CullMode (BACK/FRONT/NONE)\n";
	std::cout << "   [F10] Toggle Uniform ClearColor (ON/OFF)\n";
	std::cout << "   [F11] Toggle Print FPS (ON/OFF)\n";
	std::cout << "\n";

	std::cout << DARK_GREEN_TXT;
	std::cout << "[Key Bindings - HARDWARE]\n";
	std::cout << "   [F3] Toggle FireFX (ON/OFF)\n";
	std::cout << "   [F4] Cycle Sampler State (POINT/LINEAR/ANISOTROPIC)\n";
	std::cout << "\n";

	std::cout << DARK_MAGENTA_TXT;
	std::cout << "[Key Bindings - SOFTWARE]\n";
	std::cout << "   [F5] Cycle Shading Mode (COMBINED/OBSERVED_AREA/DIFFUSE/SPECULAR)\n";
	std::cout << "   [F6] Toggle NormalMap (ON/OFF)\n";
	std::cout << "   [F7] Toggle DepthBuffer Visualization (ON/OFF)\n";
	std::cout << "   [F8] Toggle BoundingBox Visualization (ON/OFF)\n";
	std::cout << "\n";

	std::cout << DEFAULT << "\n";
}

int main(int argc, char* args[])
{
	std::cout << DEFAULT << "\n";
	PrintInfo();

	//Unreferenced parameters
	(void)argc;
	(void)args;

	//Create window + surfaces
	SDL_Init(SDL_INIT_VIDEO);

	const uint32_t width = 640;
	const uint32_t height = 480;

	SDL_Window* pWindow = SDL_CreateWindow(
		"DualRasterizer - ***Kobe Dereyne (2DAE10)***",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width, height, 0);

	if (!pWindow)
		return 1;

	//Initialize "framework"
	const auto pTimer = new Timer();
	const auto pRenderer = new Renderer(pWindow);

	//Start loop
	bool printFPS = false;
	pTimer->Start();
	float printTimer = 0.f;
	bool isLooping = true;
	while (isLooping)
	{
		//--------- Get input events ---------
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
			case SDL_QUIT:
				isLooping = false;
				break;
			case SDL_KEYUP:
				//Test for a key
				if (e.key.keysym.scancode == SDL_SCANCODE_F1)		// DONE
					pRenderer->ToggleRenderer();
				if (e.key.keysym.scancode == SDL_SCANCODE_F2)		// DONE
					pRenderer->ToggleMeshRotation();
				if (e.key.keysym.scancode == SDL_SCANCODE_F3)		// DONE
					pRenderer->ToggleFire();
				if (e.key.keysym.scancode == SDL_SCANCODE_F4)		// DONE
					pRenderer->CycleSamplingStates();
				if (e.key.keysym.scancode == SDL_SCANCODE_F5)		// DONE
					pRenderer->CycleShadingMode();
				if (e.key.keysym.scancode == SDL_SCANCODE_F6)		// DONE
					pRenderer->ToggleNormalMap();
				if (e.key.keysym.scancode == SDL_SCANCODE_F7)		// DONE
					pRenderer->ToggleDepthBufferVisualization();
				if (e.key.keysym.scancode == SDL_SCANCODE_F8)		// DONE
					pRenderer->ToggleBoundingBox();
				if (e.key.keysym.scancode == SDL_SCANCODE_F9)		// DONE
					pRenderer->CycleCullMode();
				if (e.key.keysym.scancode == SDL_SCANCODE_F10)		// DONE
					pRenderer->ToggleUniformColor();
				if (e.key.keysym.scancode == SDL_SCANCODE_F11)		// DONE
				{
					printFPS = !printFPS;
					std::cout << DARK_YELLOW_TXT << "**(SHARED) Print FPS " << (printFPS ? "ON" : "OFF") << "\n";
				}
				break;
			default: ;
			}
		}

		//--------- Update ---------
		pRenderer->Update(pTimer);

		//--------- Render ---------
		pRenderer->Render();

		//--------- Timer ---------
		pTimer->Update();
		if (printFPS)
		{
			printTimer += pTimer->GetElapsed();
			if (printTimer >= 1.f)
			{
				printTimer = 0.f;
				std::cout << BRIGHT_BLACK_TXT << "dFPS: " << pTimer->GetdFPS() << std::endl;
			}
		}
	}
	pTimer->Stop();

	//Shutdown "framework"
	delete pRenderer;
	delete pTimer;

	ShutDown(pWindow);
	std::cout << DEFAULT << "\n";
	return 0;
}