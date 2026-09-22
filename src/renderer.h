#ifndef DL_CHESS_RENDERER_H
#define DL_CHESS_RENDERER_H
#include "helpers/shaderUtils.h"
#include "structs/AssetManager.h"
#include "structs/Board.h"
#include "structs/Matrix4x4.h"
#include "structs/Vertex.h"
#include <array>
#include <cstdint>
#include "SDL3/SDL_gpu.h"
#include "SDL3/SDL.h"
#include "helpers/dl_primitives.h"
#include "structs/Camera3d.h"
//#include "dl_imgui_utils.h"
namespace DL_Renderer{
    void RenderPiece(Piece& piece, SDL_GPUCommandBuffer* commandBuffer, SDL_GPURenderPass* renderPass, AppData* appData);

    void RenderBoard(SDL_GPURenderPass* renderPass, SDL_GPUCommandBuffer* commandBuffer, Matrix4x4& boardTransform, SDL_GPUBufferBinding& indexBuffer, std::span<SDL_GPUBufferBinding> whiteVertexBuffers, std::span<SDL_GPUBufferBinding> blackVertexBuffers, DL_LightingFragPerObject& fragPerObject, AppData* appData);

    void RenderPieces(std::span<Piece> pieces, SDL_GPURenderPass* renderPass, SDL_GPUCommandBuffer* commandBuffer, AppData* appData);

    void RenderGame(AppData* appData, SDL_GPUCommandBuffer* commandBuffer, SDL_GPUTexture* swapchainTexture);
};
#endif