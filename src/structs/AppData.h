#ifndef DL_SDL3_APPDATA_H
#define DL_SDL3_APPDATA_H
#include "SDL3/SDL_gpu.h"
#include <SDL3/SDL.h>
#include "Camera3d.h"
#include "AssetManager.h"
#include "Board.h"

struct PlayerInput{
	enum : unsigned char{
		W_PRESSED = 0b0000'0001,
		S_PRESSED = 0b0000'0010,
		A_PRESSED = 0b0000'0100,
		D_PRESSED = 0b0000'1000,
		UP_PRESSED = 0b0001'0000,
		DOWN_PRESSED = 0b0010'0000,
		LEFT_PRESSED = 0b0100'0000,
		RIGHT_PRESSED = 0b1000'0000
	};
	enum : unsigned char {
		LCLICK = 0b0001,
		RCLICK = 0b0010
	};
	unsigned char pressFlags{};
	unsigned char clickFlags {};
	float mouseX{};
	float mouseY{};
};

struct AppData {
	int width {1280};
	int height {720};

	SDL_Window* window {nullptr};
	SDL_GPUDevice* device{nullptr};

	AssetManager assets{};

	Camera3d camera{};
	Camera3d lightSource{};

	Uint64 lastTime {0};
	double deltaTime {0.0f};

	PlayerInput input{};
	Board board{};

	bool InitWindowAndDevice()
	{
		SDL_SetHint("SDL_RENDER_VSYNC", "1");
	    window = SDL_CreateWindow("Low Level Game", width, height, SDL_WINDOW_RESIZABLE);
		if(!window) {
			SDL_Log("Failed to initialise window: %s", SDL_GetError());
			return false;
		}

		//SDL_SetRenderLogicalPresentation(appData->renderer, GAMEWIDTH, GAMEHEIGHT, SDL_LOGICAL_PRESENTATION_LETTERBOX);

		SDL_ShowWindow(window);

		SDL_RaiseWindow(window);

			// Set flags for the shader formats which this program can use
		SDL_GPUShaderFormat gpuFlags {SDL_GPU_SHADERFORMAT_SPIRV|SDL_GPU_SHADERFORMAT_DXIL|SDL_GPU_SHADERFORMAT_MSL};

		// Create a truct which interfaces with a GPU device which meets the criteria establishefd in the flags
		device = SDL_CreateGPUDevice(gpuFlags, true, NULL);
		if (device == nullptr){
			SDL_Log("Coudn't create GPU Device: %s", SDL_GetError());
			return false;
		}

		// Links the GPU device to a specific window
		if(!SDL_ClaimWindowForGPUDevice(device, window)){
			SDL_Log("Couldn't claim window for GPU device: %s\n", SDL_GetError());
			return false;
		}	
		return true;
	}

	void ResizeAndReCreateDepthTextures(int w, int h)
	{
		width = w;
		height = h;
		assets.CreateDepthTextures(device, w, h);
		camera.UpdateAspectRatio(w,h);
	}
};

#endif
