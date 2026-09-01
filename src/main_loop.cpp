#include "SDL3/SDL_init.h"
#include <cstddef>
#include <SDL3/SDL.h>
#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL_main.h>
#include <iostream>
#include "dl_imgui_utils.h"
#include "structs/AppData.h"

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv) {
    std::cout << "App initialised\n";
    if(!SDL_InitSubSystem(SDL_INIT_VIDEO)){
        SDL_Log("Failed to init video: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

	AppData* appData {new AppData()};

    SDL_SetHint("SDL_RENDER_VSYNC", "1");
    appData->window = SDL_CreateWindow("Low Level Game", appData->width, appData->height, SDL_WINDOW_RESIZABLE);
    if(!appData->window) {
        SDL_Log("Failed to initialise window: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    //SDL_SetRenderLogicalPresentation(appData->renderer, GAMEWIDTH, GAMEHEIGHT, SDL_LOGICAL_PRESENTATION_LETTERBOX);

    SDL_ShowWindow(appData->window);

    SDL_RaiseWindow(appData->window);

        // Set flags for the shader formats which this program can use
    SDL_GPUShaderFormat gpuFlags {SDL_GPU_SHADERFORMAT_SPIRV|SDL_GPU_SHADERFORMAT_DXIL|SDL_GPU_SHADERFORMAT_MSL};

    // Create a truct which interfaces with a GPU device which meets the criteria establishefd in the flags
    appData->device = SDL_CreateGPUDevice(gpuFlags, true, NULL);
    if (appData->device == nullptr){
        SDL_Log("Coudn't create GPU Device: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    // Links the GPU device to a specific window
    if(!SDL_ClaimWindowForGPUDevice(appData->device, appData->window)){
        SDL_Log("Couldn't claim window for GPU device: %s\n", SDL_GetError());
        return SDL_APP_FAILURE;
    }
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
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* appstate){
	AppData* appData {(AppData*)appstate};
    Uint64 currentTime {SDL_GetPerformanceCounter()};
    double deltaTime {(currentTime - appData->lastTime) / (double)SDL_GetPerformanceFrequency()};
    appData->lastTime = currentTime;

    SDL_GPUCommandBuffer* commandBuffer {SDL_AcquireGPUCommandBuffer(appData->device)};
    if (commandBuffer == nullptr) {
        SDL_Log("Could not acquire command buffer from gpu: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    SDL_GPUTexture* swapChainTexture;
    if (!SDL_WaitAndAcquireGPUSwapchainTexture(commandBuffer, appData->window, &swapChainTexture, NULL, NULL))
    {
        SDL_Log("Couldn't acquire swapchain texture: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    if (swapChainTexture != nullptr)
    {
        /* PREPARE INFO STRUCTS*/
        SDL_GPUColorTargetInfo targetInfo {
            .texture = swapChainTexture,
            .clear_color = SDL_FColor{0.0f, 0.0f, 0.0f, 1.0f},
            .load_op = SDL_GPU_LOADOP_CLEAR,
            .store_op = SDL_GPU_STOREOP_STORE,
        };
        /* START RENDER PASS*/
        PrepareImgui(commandBuffer);
        SDL_GPURenderPass* renderPass {SDL_BeginGPURenderPass(commandBuffer, &targetInfo, 1, nullptr)};
        /* BIND PIPELINES AND DRAW */

        RenderImgui(commandBuffer, renderPass);
        SDL_EndGPURenderPass(renderPass);
        SDL_SubmitGPUCommandBuffer(commandBuffer);
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