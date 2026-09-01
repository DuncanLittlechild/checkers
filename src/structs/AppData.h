#ifndef DL_SDL3_APPDATA_H
#define DL_SDL3_APPDATA_H
#include "SDL3/SDL_gpu.h"
#include <SDL3/SDL.h>
#include "Camera3d.h"
struct AppData {
	static constexpr int width {1280};
	static constexpr int height {720};

	SDL_Window* window {nullptr};
	SDL_GPUDevice* device{nullptr};
	Camera3d camera{};

	Uint64 lastTime {0};
	double deltaTime {0.0f};

};

#endif
