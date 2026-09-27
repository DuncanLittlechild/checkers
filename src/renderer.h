#ifndef DL_CHESS_RENDERER_H
#define DL_CHESS_RENDERER_H
#include "SDL3/SDL_init.h"
#include "helpers/shaderUtils.h"
#include "structs/AssetManager.h"
#include "structs/Board.h"
#include "structs/Matrix4x4.h"
#include <array>
#include "SDL3/SDL_gpu.h"
#include "SDL3/SDL.h"
#include "structs/AppData.h"
#include "dl_imgui_utils.h"

namespace DL_Renderer{
    //Assumes per loop uniform buffers already populated
    // TODO: refactor to remove repeated code blocks.
    inline void RenderPieces(std::span<Piece> pieces, AssetManager* assets, SDL_GPURenderPass* renderPass, SDL_GPUCommandBuffer* commandBuffer)
    {
        // Uses a pointer to the piece to ensure that the correct material is always used,
        // regardless of the order of black and white
        Material* material {pieces[0].renderData.material};
        Uint32 indexCount {assets->piece.indexCount};

        std::array vertexBuffers{
            SDL_GPUBufferBinding{
                .buffer = assets->piece.vertices,
                .offset = 0
            }
        };

        SDL_GPUBufferBinding indexBuffer{
            .buffer = assets->piece.indices,
            .offset = 0
        };

        SDL_BindGPUVertexBuffers(renderPass, 0, vertexBuffers.data(), vertexBuffers.size());
        SDL_BindGPUIndexBuffer(renderPass, &indexBuffer, SDL_GPU_INDEXELEMENTSIZE_16BIT);
        SDL_PushGPUFragmentUniformData(commandBuffer, 1, material, sizeof(Material));
        
        std::vector<int> kingIndices{};
        kingIndices.reserve(8);
        Piece* selectedPiece{nullptr};
        // Iterate over themS
        for (auto& piece : pieces)
        {
            if (piece.flags & Piece::TAKEN)
                continue;
            else if (piece.type == Piece::KING)
            {
                kingIndices.push_back(piece.index);
                continue;
            }
            else if (piece.IsSelected())
            {
                selectedPiece = &piece;
                continue;
            }
            if (piece.renderData.material != material)
            {
                material = piece.renderData.material;
                SDL_PushGPUFragmentUniformData(commandBuffer, 1, material, sizeof(Material));
            }
            Matrix4x4 moveMat {Matrix4x4_Multiply(Matrix4x4_CreateScale(piece.renderData.scale), Matrix4x4_CreateTranslation(piece.renderData.offset))};
            SDL_PushGPUVertexUniformData(commandBuffer, 0, &moveMat, sizeof(Matrix4x4));        
            SDL_DrawGPUIndexedPrimitives(renderPass, indexCount, 1, 0, 0, 0);
        }
        if(selectedPiece != nullptr)
        {
            material = selectedPiece->renderData.material;
            SDL_PushGPUFragmentUniformData(commandBuffer, 1, material, sizeof(Material));
            Matrix4x4 moveMat {Matrix4x4_Multiply(Matrix4x4_CreateScale(selectedPiece->renderData.scale), Matrix4x4_CreateTranslation(selectedPiece->renderData.offset))};
            SDL_PushGPUVertexUniformData(commandBuffer, 0, &moveMat, sizeof(Matrix4x4));        
            SDL_DrawGPUIndexedPrimitives(renderPass, indexCount, 1, 0, 0, 0);
        }
        if(!kingIndices.empty())
        {
            vertexBuffers[0].buffer = assets->king.vertices;
            indexBuffer.buffer = assets->king.indices;

            SDL_BindGPUVertexBuffers(renderPass, 0, vertexBuffers.data(), vertexBuffers.size());
            SDL_BindGPUIndexBuffer(renderPass, &indexBuffer, SDL_GPU_INDEXELEMENTSIZE_16BIT);

            for (auto i : kingIndices)
            {

                if (pieces[i].IsSelected())
                {
                    selectedPiece = &pieces[i];
                    continue;
                }
                if (pieces[i].renderData.material != material)
                {
                    material = pieces[i].renderData.material;
                    SDL_PushGPUFragmentUniformData(commandBuffer, 1, material, sizeof(Material));
                }
                Matrix4x4 moveMat {Matrix4x4_Multiply(Matrix4x4_CreateScale(pieces[i].renderData.scale), Matrix4x4_CreateTranslation(pieces[i].renderData.offset))};
                SDL_PushGPUVertexUniformData(commandBuffer, 0, &moveMat, sizeof(Matrix4x4));        
                SDL_DrawGPUIndexedPrimitives(renderPass, indexCount, 1, 0, 0, 0);
            }
        }
        if(selectedPiece != nullptr)
        {
            material = selectedPiece->renderData.material;
            SDL_PushGPUFragmentUniformData(commandBuffer, 1, material, sizeof(Material));
            Matrix4x4 moveMat {Matrix4x4_Multiply(Matrix4x4_CreateScale(selectedPiece->renderData.scale), Matrix4x4_CreateTranslation(selectedPiece->renderData.offset))};
            SDL_PushGPUVertexUniformData(commandBuffer, 0, &moveMat, sizeof(Matrix4x4));        
            SDL_DrawGPUIndexedPrimitives(renderPass, indexCount, 1, 0, 0, 0);
        }
    }

    inline void RenderTiles(std::span<Tile> tiles, AssetManager* assets, SDL_GPURenderPass* renderPass, SDL_GPUCommandBuffer* commandBuffer)
    {
        // Uses a pointer to the piece to ensure that the correct material is always used,
        // regardless of the order of black and white
        Material* material {tiles[0].renderData.material};
        Uint32 indexCount {assets->tile.indexCount};

        std::array vertexBuffers{
            SDL_GPUBufferBinding{
                .buffer = assets->tile.vertices,
                .offset = 0
            }
        };

        SDL_GPUBufferBinding indexBuffer{
            .buffer = assets->tile.indices,
            .offset = 0
        };

        SDL_BindGPUVertexBuffers(renderPass, 0, vertexBuffers.data(), vertexBuffers.size());
        SDL_BindGPUIndexBuffer(renderPass, &indexBuffer, SDL_GPU_INDEXELEMENTSIZE_16BIT);
        SDL_PushGPUFragmentUniformData(commandBuffer, 1, material, sizeof(Material));
        
        // Iterate over themS
        for (auto& tile : tiles)
        {
            if (tile.renderData.material != material)
            {
                material = tile.renderData.material;
                SDL_PushGPUFragmentUniformData(commandBuffer, 1, material, sizeof(Material));
            }
            Matrix4x4 moveMat {Matrix4x4_Multiply(Matrix4x4_CreateScale(tile.renderData.scale), Matrix4x4_CreateTranslation(tile.renderData.offset))};
            SDL_PushGPUVertexUniformData(commandBuffer, 0, &moveMat, sizeof(Matrix4x4));        
            SDL_DrawGPUIndexedPrimitives(renderPass, indexCount, 1, 0, 0, 0);
        }
    }


    inline bool RenderGame(AppData* appData)
    {
        SDL_GPUCommandBuffer* commandBuffer {SDL_AcquireGPUCommandBuffer(appData->device)};
        if (commandBuffer == nullptr) {
            SDL_Log("Could not acquire command buffer from gpu: %s", SDL_GetError());
            return false;
        }

        SDL_GPUTexture* swapchainTexture;
        if (!SDL_WaitAndAcquireGPUSwapchainTexture(commandBuffer, appData->window, &swapchainTexture, NULL, NULL))
        {
            SDL_Log("Couldn't acquire swapchain texture: %s", SDL_GetError());
            return false;
        }
        if(swapchainTexture == nullptr)
            return true;

        /* PREPARE INFO STRUCTS*/
        SDL_GPUColorTargetInfo targetInfo {
            .texture = swapchainTexture,
            .clear_color = SDL_FColor{0.0f, 0.0f, 0.0f, 1.0f},
            .load_op = SDL_GPU_LOADOP_CLEAR,
            .store_op = SDL_GPU_STOREOP_STORE,
        };

        SDL_GPUDepthStencilTargetInfo depthInfo{
            .texture = appData->assets.depth,
            .clear_depth = 1.0f,
            .load_op = SDL_GPU_LOADOP_CLEAR,
            .store_op = SDL_GPU_STOREOP_DONT_CARE
        };

        PrepareImgui(appData, commandBuffer);

        SDL_GPURenderPass* renderPass {SDL_BeginGPURenderPass(commandBuffer, &targetInfo, 1, &depthInfo)};

        /* BIND PIPELINES AND DRAW */
        SDL_BindGPUGraphicsPipeline(renderPass, appData->assets.pipeline);

        SDL_PushGPUVertexUniformData(commandBuffer, 1, &appData->camera.vpMat, sizeof(Matrix4x4));

        DL_LightingFragPerLoop fragPerLoopUB{
            .cameraPos = appData->camera.pos,
            .lightPos = appData->lightSource.pos,
        };
        SDL_PushGPUFragmentUniformData(commandBuffer, 0, &fragPerLoopUB, sizeof(DL_LightingFragPerLoop));

        RenderTiles(appData->board.board.data, &appData->assets, renderPass, commandBuffer);
        RenderPieces(appData->board.pieces, &appData->assets, renderPass, commandBuffer);

        RenderImgui(commandBuffer, renderPass);
        SDL_EndGPURenderPass(renderPass);
        SDL_SubmitGPUCommandBuffer(commandBuffer);

        return true;
    }
};
#endif