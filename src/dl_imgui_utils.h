#ifndef DL_SDL3_IMGUI_UTILS_H
#define DL_SDL3_IMGUI_UTILS_H
#include "SDL3/SDL_gpu.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlgpu3.h"

static ImDrawData* drawData {nullptr};

static void HelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::BeginItemTooltip())
    {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

inline void InitImgui(SDL_GPUDevice* device, SDL_Window* window) {
    // Basic setup
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // Set appearance and scaling of ImGui
    //ImGuiStyle& style = ImGui::GetStyle();
    //style.ScaleAllSizes(GameSettings::INITIALSCALE);
    // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
    //style.FontScaleDpi = GameSettings::INITIALSCALE;
    ImGui::StyleColorsDark();

    // Enable keyboard navigation in imgui
    ImGuiIO& io {ImGui::GetIO()};
    io.FontGlobalScale = 2;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Initialise ImGui globals for SDL
    ImGui_ImplSDL3_InitForSDLGPU(window);
    ImGui_ImplSDLGPU3_InitInfo init_info = {};
    init_info.Device = device;
    init_info.ColorTargetFormat = SDL_GetGPUSwapchainTextureFormat(device, window);
    init_info.MSAASamples = SDL_GPU_SAMPLECOUNT_1;                      // Only used in multi-viewports mode.
    init_info.SwapchainComposition = SDL_GPU_SWAPCHAINCOMPOSITION_SDR;  // Only used in multi-viewports mode.
    init_info.PresentMode = SDL_GPU_PRESENTMODE_VSYNC;
    ImGui_ImplSDLGPU3_Init(&init_info);
}

inline void PrepareImgui(SDL_GPUCommandBuffer* commandBuffer) {
    ImGui_ImplSDLGPU3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
    ImGuiIO& io {ImGui::GetIO()};
    ImGui::Begin("Tab");
    ImGui::End();

    ImGui::Render();
    drawData = ImGui::GetDrawData();
    ImGui_ImplSDLGPU3_PrepareDrawData(drawData, commandBuffer);
}

inline void RenderImgui(SDL_GPUCommandBuffer* commandBuffer, SDL_GPURenderPass* renderPass)
{
    ImGui_ImplSDLGPU3_RenderDrawData(drawData, commandBuffer, renderPass);
}

inline void QuitImgui(){
    // Shutdown ImGui
    ImGui_ImplSDLGPU3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
}


#endif
