#ifndef DL_SDL3_IMGUI_UTILS_H
#define DL_SDL3_IMGUI_UTILS_H
#include "SDL3/SDL_gpu.h"
#include "SDL3/SDL_mouse.h"
#include "SDL3/SDL_stdinc.h"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlgpu3.h"
#include "structs/AppData.h"

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
    //io.FontGlobalScale = 2;
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

inline void PrepareImgui(AppData* appData, SDL_GPUCommandBuffer* commandBuffer) {
    ImGui_ImplSDLGPU3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
    ImGuiIO& io {ImGui::GetIO()};
    ImGui::Begin("Camera");
    ImGui::Text("Camera Position:  x:%f,  y:%f,  z:%f", appData->camera.pos.x,appData->camera.pos.y,appData->camera.pos.z);
    ImGui::Text("Camera target:  x:%f,  y:%f,  z:%f", appData->camera.target.x,appData->camera.target.y,appData->camera.target.z);
    ImGui::Text("DeltaTime: %f", appData->deltaTime);
    ImGui::Text("Selected square: x: %d, y: %d", appData->board.GetSelectedPiece().bPos.row, appData->board.GetSelectedPiece().bPos.col);
    if(ImGui::ColorEdit3("WhiteCol", appData->assets.pieceMat.albedo))
    {
        SDL_memcpy(appData->assets.tileMat.albedo, appData->assets.pieceMat.albedo, sizeof(float) * 3);
    }
    ImGui::ColorEdit3("BlackCol", &appData->board.blackCol.x);

    if(ImGui::SliderFloat3("LightPos", &appData->lightSource.pos.x, -20.f, 20.f))
    {
        appData->lightSource.UpdateViewMat();
        appData->lightSource.UpdateVpMat();
    }
    ImGui::SliderFloat("Piece Shinyness", &appData->pieceShinyness, .0f, 128.f);
    ImGui::SliderFloat("Board Shinyness", &appData->boardShinyness, .0f, 128.f);

    ImGui::ColorEdit3("Board Specular Colour",&appData->boardSpecularColour.x);
    ImGui::ColorEdit3("Piece Specular Colour", &appData->pieceSpecularColour.x);
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
