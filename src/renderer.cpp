#include "helpers/shaderUtils.h"
#include "structs/AppData.h"
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
#include "dl_imgui_utils.h"
namespace DL_Renderer{
    void RenderThing(Thing& thing, SDL_GPUCommandBuffer* commandBuffer, SDL_GPURenderPass* renderPass)
    {
        Matrix4x4 moveMat {Matrix4x4_Multiply(Matrix4x4_CreateScale(thing.scale), Matrix4x4_CreateTranslation(thing.offset.x, thing.offset.y, thing.offset.z))};
        SDL_PushGPUVertexUniformData(commandBuffer, 0, &moveMat, sizeof(Matrix4x4));        
        SDL_DrawGPUIndexedPrimitives(renderPass, assets.pieceIndices.indexCount, 1, 0, 0, 0);
    }

    void RenderBoard(SDL_GPURenderPass* renderPass, SDL_GPUCommandBuffer* commandBuffer, Matrix4x4& boardTransform, SDL_GPUBufferBinding& indexBuffer, std::span<SDL_GPUBufferBinding> whiteVertexBuffers, std::span<SDL_GPUBufferBinding> blackVertexBuffers, DL_LightingFragPerObject& fragPerObject, AppData* appData)
    {
        // Render the board
        SDL_PushGPUVertexUniformData(commandBuffer, 0, &boardTransform, sizeof(Matrix4x4));
        SDL_BindGPUIndexBuffer(renderPass, &indexBuffer, SDL_GPU_INDEXELEMENTSIZE_16BIT);
        fragPerObject.trueColour = appData->board.whiteCol;
        SDL_PushGPUFragmentUniformData(commandBuffer, 1, &fragPerObject, sizeof(DL_LightingFragPerObject));

        SDL_BindGPUVertexBuffers(renderPass, 0, whiteVertexBuffers.data(), whiteVertexBuffers.size());
        SDL_DrawGPUIndexedPrimitives(renderPass, appData->assets.chessboardIndices.indexCount, 1, 0, 0, 0);
        fragPerObject.trueColour = appData->board.blackCol;
        
        SDL_PushGPUFragmentUniformData(commandBuffer, 1, &fragPerObject, sizeof(DL_LightingFragPerObject));

        SDL_BindGPUVertexBuffers(renderPass, 0, blackVertexBuffers.data(), blackVertexBuffers.size());
        SDL_DrawGPUIndexedPrimitives(renderPass, appData->assets.chessboardIndices.indexCount, 1, 0, 0, 0);
    }

    void RenderPieces(std::span<Piece> pieces, SDL_GPURenderPass* renderPass, SDL_GPUCommandBuffer* commandBuffer, AppData* appData)
    {
        for(auto& piece : pieces)
        {
            if (&appData->board.GetSelectedPiece() == &piece || (piece.flags & Piece::TAKEN))
            {
                continue;
            }
            RenderPiece(piece, commandBuffer, renderPass, appData);
        }
    }

    void RenderGame(AppData* appData, SDL_GPUCommandBuffer* commandBuffer, SDL_GPUTexture* swapchainTexture)
    {
        /* PREPARE INFO STRUCTS*/
        SDL_GPUColorTargetInfo targetInfo {
            .texture = swapchainTexture,
            .clear_color = SDL_FColor{0.0f, 0.0f, 0.0f, 1.0f},
            .load_op = SDL_GPU_LOADOP_CLEAR,
            .store_op = SDL_GPU_STOREOP_STORE,
        };

        SDL_GPUDepthStencilTargetInfo depthInfo{
            .texture = appData->depth,
            .clear_depth = 1.0f,
            .load_op = SDL_GPU_LOADOP_CLEAR,
            .store_op = SDL_GPU_STOREOP_DONT_CARE
        };

        SDL_GPUDepthStencilTargetInfo shadowDepthInfo{
            .texture = appData->shadows,
            .clear_depth = 1.0f,
            .load_op = SDL_GPU_LOADOP_CLEAR,
            .store_op = SDL_GPU_STOREOP_STORE
        };

        std::array whiteVertexBuffers {
            SDL_GPUBufferBinding{
                .buffer = appData->assets.whiteSquareVertices,
                .offset = 0
            }
        };

        std::array blackVertexBuffers {
            SDL_GPUBufferBinding{
                .buffer = appData->assets.blackSquareVertices,
                .offset = 0
            }
        };

        std::array cubeVertexBuffers {
            SDL_GPUBufferBinding{
                .buffer = appData->assets.cubeVertices,
                .offset = 0
            }
        };

        SDL_GPUBufferBinding cubeIndex {
            .buffer = appData->assets.cubeIndices.buffer,
            .offset = 0
        };

        SDL_GPUBufferBinding indexBuffer{
            .buffer = appData->assets.chessboardIndices.buffer,
            .offset = 0
        };

        std::array pieceVertexBuffers{
            SDL_GPUBufferBinding{
                .buffer = appData->assets.pieceVertices,
                .offset = 0
            }
        };

        SDL_GPUBufferBinding pieceIndexBuffers{
            .buffer = appData->assets.pieceIndices.buffer,
            .offset = 0
        };

        PrepareImgui(appData, commandBuffer);
        static Matrix4x4 boardTransform{Matrix4x4_Multiply(Matrix4x4_CreateScale(8.0f), Matrix4x4_CreateTranslation(4.0, 0.0, 4.0))};

        // Shadows render pass
        /*
        SDL_GPURenderPass* shadowRenderPass {SDL_BeginGPURenderPass(commandBuffer, &targetInfo, 1, &shadowDepthInfo)};


        SDL_BindGPUGraphicsPipeline(shadowRenderPass, appData->shadowPipeline);
        SDL_PushGPUVertexUniformData(commandBuffer, 1, &appData->lightSource.vpMat, sizeof(Matrix4x4));
        RenderBoard(shadowRenderPass, commandBuffer, boardTransform, indexBuffer, whiteVertexBuffers, blackVertexBuffers, appData);

        SDL_BindGPUVertexBuffers(shadowRenderPass, 0, pieceVertexBuffers.data(), pieceVertexBuffers.size());
        SDL_BindGPUIndexBuffer(shadowRenderPass, &pieceIndexBuffers, SDL_GPU_INDEXELEMENTSIZE_16BIT);
        RenderPieces(appData->board.blackPieces, shadowRenderPass, commandBuffer, appData);
        RenderPieces(appData->board.whitePieces, shadowRenderPass, commandBuffer, appData);
        SDL_EndGPURenderPass(shadowRenderPass);
        */

        SDL_GPURenderPass* renderPass {SDL_BeginGPURenderPass(commandBuffer, &targetInfo, 1, &depthInfo)};

        /* BIND PIPELINES AND DRAW */
        SDL_BindGPUGraphicsPipeline(renderPass, appData->pipeline);
        SDL_GPUTextureSamplerBinding binding {
            .texture = appData->shadows,
            .sampler = appData->shadowSampler
        };
        SDL_BindGPUFragmentSamplers(renderPass, 0, &binding, 1);

        SDL_PushGPUVertexUniformData(commandBuffer, 1, &appData->camera.vpMat, sizeof(Matrix4x4));

        DL_LightingFragPerLoop fragPerLoopUB{
            .cameraPos = appData->camera.pos,
            .lightPos = appData->lightSource.pos,
        };
        SDL_PushGPUFragmentUniformData(commandBuffer, 0, &fragPerLoopUB, sizeof(DL_LightingFragPerLoop));
        DL_LightingFragPerObject fragPerObjectUB{
            .trueColour = appData->board.blackCol,
            .specularColour = appData->boardSpecularColour,
            .shinyness = appData->boardShinyness 
        };


        SDL_PushGPUVertexUniformData(commandBuffer, 0, &boardTransform, sizeof(Matrix4x4));
        SDL_BindGPUIndexBuffer(renderPass, &indexBuffer, SDL_GPU_INDEXELEMENTSIZE_16BIT);
        fragPerObjectUB.trueColour = appData->board.whiteCol;
        SDL_PushGPUFragmentUniformData(commandBuffer, 1, &fragPerObjectUB, sizeof(DL_LightingFragPerObject));

        SDL_BindGPUVertexBuffers(renderPass, 0, whiteVertexBuffers.data(), whiteVertexBuffers.size());
        SDL_DrawGPUIndexedPrimitives(renderPass, appData->assets.chessboardIndices.indexCount, 1, 0, 0, 0);
        fragPerObjectUB.trueColour = appData->board.blackCol;
        
        SDL_PushGPUFragmentUniformData(commandBuffer, 1, &fragPerObjectUB, sizeof(DL_LightingFragPerObject));

        SDL_BindGPUVertexBuffers(renderPass, 0, blackVertexBuffers.data(), blackVertexBuffers.size());
        SDL_DrawGPUIndexedPrimitives(renderPass, appData->assets.chessboardIndices.indexCount, 1, 0, 0, 0);

        
        // Render pieces
        SDL_BindGPUVertexBuffers(renderPass, 0, pieceVertexBuffers.data(), pieceVertexBuffers.size());
        SDL_BindGPUIndexBuffer(renderPass, &pieceIndexBuffers, SDL_GPU_INDEXELEMENTSIZE_16BIT);
        fragPerObjectUB.trueColour = appData->board.blackCol;
        fragPerObjectUB.specularColour = appData->pieceSpecularColour;
        fragPerObjectUB.shinyness = appData->pieceShinyness;
        SDL_PushGPUFragmentUniformData(commandBuffer, 1, &fragPerObjectUB, sizeof(fragPerObjectUB));
        
        RenderPieces(appData->board.blackPieces, renderPass, commandBuffer, appData);
        fragPerObjectUB.trueColour = appData->board.whiteCol;
        SDL_PushGPUFragmentUniformData(commandBuffer, 1, &fragPerObjectUB, sizeof(fragPerObjectUB));
        RenderPieces(appData->board.whitePieces, renderPass, commandBuffer, appData);
        Piece& selectedPiece {appData->board.GetSelectedPiece()};
        if (!selectedPiece.IsNull() && !(selectedPiece.flags & Piece::TAKEN))
        {
            fragPerObjectUB.trueColour = appData->board.selectedCol;
            SDL_PushGPUFragmentUniformData(commandBuffer, 1, &fragPerObjectUB, sizeof(fragPerObjectUB));
            RenderPiece(selectedPiece, commandBuffer, renderPass, appData);
        }

        RenderImgui(commandBuffer, renderPass);
        SDL_EndGPURenderPass(renderPass);
        SDL_SubmitGPUCommandBuffer(commandBuffer);
    }
};