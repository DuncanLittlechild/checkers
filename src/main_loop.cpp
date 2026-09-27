#include "SDL3/SDL_events.h"
#include "SDL3/SDL_gpu.h"
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_mouse.h"
#include "SDL3/SDL_video.h"
#include "helpers/dl_primitives.h"
#include "helpers/shaderUtils.h"
#include "structs/Board.h"
#include "structs/Matrix4x4.h"
#include <cstddef>
#include <SDL3/SDL.h>
#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL_main.h>
#include <iostream>
#include "dl_imgui_utils.h"
#include "structs/AppData.h"
#include "renderer.h"
#include "structs/Ray.h"
#include "game.h"

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
    std::cout << "App initialised\n";
    if(!SDL_InitSubSystem(SDL_INIT_VIDEO)){
        SDL_Log("Failed to init video: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

	AppData* appData {new AppData()};

    if(!appData->InitWindowAndDevice())
        return SDL_APP_FAILURE;

    appData->camera.Init(appData->width, appData->height);
    appData->lightSource.Init(appData->lightRadius, appData->lightRadius);
    appData->lightSource.fov = appData->lightRadius;
    appData->lightSource.target = Vector3{4.0f, 0.0f, 4.0f};

    appData->assets.Load(appData->device, appData->window, appData->width, appData->height);

    appData->board.Init(appData->assets);
    InitImgui(appData->device, appData->window);

	*appstate = appData;

    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event){
	AppData* appData {(AppData*)appstate};
    ImGui_ImplSDL3_ProcessEvent(event);
    switch(event->type) {
        case (SDL_EVENT_QUIT) : {
            return SDL_APP_SUCCESS;
        } break;
        case(SDL_EVENT_KEY_DOWN) :{
            switch(event->key.scancode){
                case (SDL_SCANCODE_W):{
                    appData->input.pressFlags |= PlayerInput::W_PRESSED;
                } break;
                case (SDL_SCANCODE_S):{
                    appData->input.pressFlags |= PlayerInput::S_PRESSED;
                } break;
                case (SDL_SCANCODE_A):{
                    appData->input.pressFlags |= PlayerInput::A_PRESSED;
                }break;
                case (SDL_SCANCODE_D):{
                    appData->input.pressFlags |= PlayerInput::D_PRESSED;
                }break;
                case (SDL_SCANCODE_UP):{
                    appData->input.pressFlags |= PlayerInput::UP_PRESSED;
                }break;
                case (SDL_SCANCODE_DOWN):{
                    appData->input.pressFlags |= PlayerInput::DOWN_PRESSED;
                }break;
                case (SDL_SCANCODE_LEFT):{
                    appData->input.pressFlags |= PlayerInput::LEFT_PRESSED;
                }break;
                case (SDL_SCANCODE_RIGHT):{
                    appData->input.pressFlags |= PlayerInput::RIGHT_PRESSED;
                }break;
            }
        } break;
        case(SDL_EVENT_KEY_UP) :{
            switch(event->key.scancode){             
                case (SDL_SCANCODE_W):{
                    appData->input.pressFlags &= ~PlayerInput::W_PRESSED;
                }break;
                case (SDL_SCANCODE_S):{
                    appData->input.pressFlags &= ~PlayerInput::S_PRESSED;
                }break;
                case (SDL_SCANCODE_A):{
                    appData->input.pressFlags &= ~PlayerInput::A_PRESSED;
                }break;
                case (SDL_SCANCODE_D):{
                    appData->input.pressFlags &= ~PlayerInput::D_PRESSED;
                }break;
                case (SDL_SCANCODE_UP):{
                    appData->input.pressFlags &= ~PlayerInput::UP_PRESSED;
                }break;
                case (SDL_SCANCODE_DOWN):{
                    appData->input.pressFlags &= ~PlayerInput::DOWN_PRESSED;
                }break;
                case (SDL_SCANCODE_LEFT):{
                    appData->input.pressFlags &= ~PlayerInput::LEFT_PRESSED;
                }break;
                case (SDL_SCANCODE_RIGHT):{
                    appData->input.pressFlags &= ~PlayerInput::RIGHT_PRESSED;
                }break;
            }
        } break;
        case (SDL_EVENT_MOUSE_BUTTON_DOWN):{
            switch(event->button.button){
                case(SDL_BUTTON_LEFT):{
                    float x, y;
                    SDL_GetMouseState(&x, &y);
                    appData->input.clickFlags |= PlayerInput::LCLICK;
                    appData->input.mouseX = x;
                    appData->input.mouseY = y;
                } break;
                case(SDL_BUTTON_RIGHT):{
                    appData->input.clickFlags |= PlayerInput::RCLICK;
                } break;
            }
        } break;
        case (SDL_EVENT_WINDOW_RESIZED): {
            int w, h;
            SDL_GetWindowSize(appData->window, &w, &h);
            appData->ResizeAndReCreateDepthTextures(w, h);
        } break;
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate){
	AppData* appData {(AppData*)appstate};
    Uint64 currentTime {SDL_GetPerformanceCounter()};
    appData->deltaTime = (currentTime - appData->lastTime) / (double)SDL_GetPerformanceFrequency();
    appData->lastTime = currentTime;

    UpdateGame(appData);
    if(!DL_Renderer::RenderGame(appData))
    {
        return SDL_APP_FAILURE;
    }

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result){
	AppData* appData {(AppData*)appstate};
    QuitImgui();
    SDL_ReleaseWindowFromGPUDevice(appData->device, appData->window);
    SDL_DestroyWindow(appData->window);
    SDL_DestroyGPUDevice(appData->device);
}