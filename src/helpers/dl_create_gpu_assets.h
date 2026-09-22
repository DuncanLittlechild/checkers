#ifndef DL_CHESS_CREATE_GPU_ASSETS_H
#define DL_CHESS_CREATE_GPU_ASSETS_H
#include "SDL3/SDL.h"
#include <span>
#include <vector>
#include "helpers/shaderUtils.h"
#include "structs/Vertex.h"


template <typename T>
bool CreateGPUBuffer(SDL_GPUDevice* device, std::span<const T> vertices, SDL_GPUBufferUsageFlags usage, SDL_GPUBuffer** buffer, Uint32* indexCount = nullptr)
{
    Uint32 verticesSize = sizeof(T) * vertices.size();

    SDL_GPUBufferCreateInfo vertexCreateInfo {
        .usage = usage,
        .size = verticesSize,
    };

    *buffer = SDL_CreateGPUBuffer(device,&vertexCreateInfo);
    if(*buffer == nullptr)
    {
        SDL_Log("Couldn't create buffer");
        return false;   
    }

    SDL_GPUTransferBufferCreateInfo transferBufferCreateInfo {
        .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
        .size = verticesSize
    };

    SDL_GPUTransferBuffer* transferBuffer{
        SDL_CreateGPUTransferBuffer(device, &transferBufferCreateInfo)
    };

    if(transferBuffer == nullptr)
    {
        SDL_Log("Couldn't create transfer buffer");
        return false;
    }

    // transfer info into the transfer buffer
    void* vertexTransferPtr1{SDL_MapGPUTransferBuffer(device, transferBuffer, false)};
    SDL_memcpy(vertexTransferPtr1, vertices.data(), verticesSize);
    SDL_UnmapGPUTransferBuffer(device, transferBuffer);

    SDL_GPUCommandBuffer* uploadCommandBuff {
        SDL_AcquireGPUCommandBuffer(device)
    };
    if (uploadCommandBuff == nullptr)
    {
        SDL_Log("Couldn't acquire GPU command buffer: %s", SDL_GetError());
        return false;
    }

    SDL_GPUCopyPass* copyPass {SDL_BeginGPUCopyPass(uploadCommandBuff)};

    SDL_GPUTransferBufferLocation bufferLocation {
        .transfer_buffer = transferBuffer,
        .offset = 0
    };

    SDL_GPUBufferRegion bufferRegion {
        .buffer = *buffer,
        .offset = 0,
        .size = verticesSize
    };

    SDL_UploadToGPUBuffer(copyPass, &bufferLocation, &bufferRegion, true);

    SDL_EndGPUCopyPass(copyPass);
    if(!SDL_SubmitGPUCommandBuffer(uploadCommandBuff))
    {
        SDL_Log("Couldn't submit GPU command buffer: %s", SDL_GetError());
        return false;
    }

    SDL_ReleaseGPUTransferBuffer(device, transferBuffer);

    if(usage == SDL_GPU_BUFFERUSAGE_INDEX)
    {
        if(indexCount != nullptr)
        {
            *indexCount = vertices.size();
        }
        else 
        {
            SDL_Log("Index count must not be null when creating index buffer");
            return false;
        }
    }

    return true;        
}

inline bool CreateDepthTexture(SDL_GPUDevice* device, SDL_GPUTexture** depthTexture, int w, int h)
{
    if (*depthTexture != nullptr) SDL_ReleaseGPUTexture(device, *depthTexture);
    SDL_GPUTextureCreateInfo createInfo {
        .type = SDL_GPU_TEXTURETYPE_2D,
        .format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT,
        .usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET,
        .width = (Uint32)w,
        .height = (Uint32)h,
        .layer_count_or_depth = 1,
        .num_levels = 1
    };
    *depthTexture = SDL_CreateGPUTexture(device, &createInfo);

    return true;
}

inline bool CreateGPUSampler(SDL_GPUSampler** sampler, SDL_GPUDevice* device)
{
    SDL_GPUSamplerCreateInfo pointClamp {
        .min_filter = SDL_GPU_FILTER_NEAREST,
        .mag_filter = SDL_GPU_FILTER_NEAREST,
        .mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST,
        .address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
        .address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
        .address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
    };
    // PointClamp
    *sampler = SDL_CreateGPUSampler(device, &pointClamp);
    return true;
}

inline bool CreateGPUVertexBuffer(SDL_GPUDevice* device, const std::vector<Vertex>& vertices, SDL_GPUBuffer** buffer)
{
    return CreateGPUBuffer(device, std::span<const Vertex>(vertices), SDL_GPU_BUFFERUSAGE_VERTEX, buffer);
}

inline bool CreateGPUIndexBuffer(SDL_GPUDevice* device, const std::vector<uint16_t>& vertices, SDL_GPUBuffer** buffer, Uint32& indexCount)
{
    return CreateGPUBuffer(device, std::span<const uint16_t>(vertices), SDL_GPU_BUFFERUSAGE_INDEX, buffer, &indexCount);
}

inline bool CreatePipeline(SDL_GPUGraphicsPipeline** pipeline, SDL_GPUDevice* device, SDL_Window* window, DL_ShaderInfo* vertexShaderInfo, DL_ShaderInfo* fragmentShaderInfo)
{
    SDL_GPUShader* vertexColourShader {LoadShader(device, vertexShaderInfo)};
    if(vertexColourShader == nullptr)
    {
        SDL_Log("Couldn'tcreate vertex shader\n");
        return false;
    }

    SDL_GPUShader* basicFragShader {LoadShader(device, fragmentShaderInfo)};
    if(basicFragShader == nullptr)
    {
        SDL_Log("Couldn'tcreate fragment shader\n");
        return false;
    }

    std::array colourTargetDescriptions {
        SDL_GPUColorTargetDescription{
            .format = SDL_GetGPUSwapchainTextureFormat(device, window)
        }
    };

    std::array vertexBufferDescriptions {
        SDL_GPUVertexBufferDescription {
            .slot = 0,
            .pitch = sizeof(Vertex),
            .input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
            .instance_step_rate = 0
        }
    };

    std::array vertexAttributes {
        SDL_GPUVertexAttribute{
            .location = 0,
            .buffer_slot = 0,
            .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
            .offset = 0
        },
        SDL_GPUVertexAttribute{
            .location = 1,
            .buffer_slot = 0,
            .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
            .offset = sizeof(float) * 3
        },
        SDL_GPUVertexAttribute{
            .location = 2,
            .buffer_slot = 0,
            .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
            .offset = sizeof(float) * 6
        }
    };

    SDL_GPUGraphicsPipelineCreateInfo pipelineCreateInfo {
        .vertex_shader = vertexColourShader,
        .fragment_shader = basicFragShader,
        .vertex_input_state = {
            .vertex_buffer_descriptions = vertexBufferDescriptions.data(),
            .num_vertex_buffers = vertexBufferDescriptions.size(),
            .vertex_attributes = vertexAttributes.data(),
            .num_vertex_attributes = vertexAttributes.size()
        },
        .primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
        .rasterizer_state = {
            .fill_mode = SDL_GPU_FILLMODE_FILL,
            .cull_mode = SDL_GPU_CULLMODE_BACK,
            .front_face = SDL_GPU_FRONTFACE_COUNTER_CLOCKWISE,
        },
        .depth_stencil_state = {
            .compare_op = SDL_GPU_COMPAREOP_LESS,
            .enable_depth_test = true,
            .enable_depth_write = true,
            .enable_stencil_test = false
        },
        .target_info = {
            .color_target_descriptions = colourTargetDescriptions.data(),
            .num_color_targets = colourTargetDescriptions.size(),
            .depth_stencil_format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT,
            .has_depth_stencil_target = true,
        }
    };

    *pipeline = SDL_CreateGPUGraphicsPipeline(device, &pipelineCreateInfo);

    SDL_ReleaseGPUShader(device, vertexColourShader);
    SDL_ReleaseGPUShader(device, basicFragShader);

    return true;
}

#endif