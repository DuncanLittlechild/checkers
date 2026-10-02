#ifndef DL_SDL3_IMGUI_UTILS_H
#define DL_SDL3_IMGUI_UTILS_H
#include "SDL3/SDL_gpu.h"
#include "SDL3/SDL_mouse.h"
#include "SDL3/SDL_stdinc.h"
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_sdlgpu3.h"
#include "structs/AppData.h"
#include "structs/Board.h"
#include "structs/Vector3.h"
#include <string_view>

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

inline void DL_Imgui_EditMaterial(Material& material, std::string_view name)
{
    static int instance {0};
    float albedo[3];
    SDL_memcpy(albedo, &material.albedo, sizeof(Vector3));
    float specularColour[3];
    SDL_memcpy(specularColour, &material.specularColour, sizeof(Vector3));
    std::string albedoStr {"albedo##"};
    albedoStr += name;
    std::string sCStr {"specular colour##"};
    sCStr += name;
    std::string shininessStr{"shininess##"};
    shininessStr += name;
    if(ImGui::CollapsingHeader(name.data()))
    {
        if(ImGui::ColorEdit3(albedoStr.c_str(), albedo))
            SDL_memcpy(&material.albedo, albedo, sizeof(Vector3));
        if(ImGui::ColorEdit3(sCStr.c_str(), specularColour))
            SDL_memcpy(&material.specularColour, specularColour, sizeof(Vector3));
        ImGui::SliderFloat(shininessStr.c_str(), &material.shininess, 0.0f, 128.f);
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
    ImGui::Begin("Data");
    if(ImGui::BeginTabBar("Edits"))
    {
        if(ImGui::BeginTabItem("Camera"))
        {
            ImGui::Text("Camera Position:  x:%f,  y:%f,  z:%f", appData->camera.pos.x,appData->camera.pos.y,appData->camera.pos.z);
            ImGui::Text("Camera target:  x:%f,  y:%f,  z:%f", appData->camera.target.x,appData->camera.target.y,appData->camera.target.z);
            ImGui::Text("DeltaTime: %f", appData->deltaTime);
            ImGui::Text("SquareClickedOn: x: %f, z: %f", appData->board.squareClickedOnX, appData->board.squareClickedOnZ);
            ImGui::Text("Selected square: x: %d, y: %d", appData->board.GetSelectedPiece().bPos.row, appData->board.GetSelectedPiece().bPos.col);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Light"))
        {
            if(ImGui::SliderFloat3("LightPos", &appData->lightSource.pos.x, -20.f, 20.f))
            {
                appData->lightSource.UpdateViewMat();
                appData->lightSource.UpdateVpMat();
            }
            if(ImGui::SliderFloat3("LightTarget", &appData->lightSource.target.x, -20.f, 20.f))
            {
                appData->lightSource.UpdateViewMat();
                appData->lightSource.UpdateVpMat();
            }
            if(ImGui::SliderFloat("LightRadius", &appData->lightRadius, 0.01f, SDL_PI_F))
            {
                appData->lightSource.fov = appData->lightRadius;
                appData->lightSource.UpdateProjMat();
                appData->lightSource.UpdateVpMat();
            }
            ImGui::EndTabItem();
        }
        if(ImGui::BeginTabItem("Materials"))
        {
            DL_Imgui_EditMaterial(appData->assets.pieceMatW, "White Piece");
            DL_Imgui_EditMaterial(appData->assets.pieceMatB, "Black Piece");
            DL_Imgui_EditMaterial(appData->assets.pieceMatSelected, "Selected Piece");
            DL_Imgui_EditMaterial(appData->assets.tileMatB, "White Tile");
            DL_Imgui_EditMaterial(appData->assets.tileMatB, "Black Tile");
            if(ImGui::Button("Save Materials"))
                appData->assets.SaveMatData();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

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
