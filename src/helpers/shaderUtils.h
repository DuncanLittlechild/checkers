#ifndef DL_GPUTEST_SHADERUTILS_H
#define DL_GPUTEST_SHADERUTILS_H
#include "SDL3/SDL.h"
#include <string>
#include <filesystem>

struct DL_ShaderInfo{
    Uint32 numSamplers{};
    Uint32 numStorageTextures{};
    Uint32 numStorageBuffers{};
    Uint32 numUniformBuffers{};
};

inline SDL_GPUShader* LoadShader(SDL_GPUDevice* device, const std::string& shaderFilename, const DL_ShaderInfo& dlShaderInfo = {})
{
    SDL_GPUShaderStage stage;
    if(shaderFilename.contains(".vert"))
    {
        stage = SDL_GPU_SHADERSTAGE_VERTEX;
    }
    else if (shaderFilename.contains(".frag"))
    {
        stage = SDL_GPU_SHADERSTAGE_FRAGMENT;
    }
    else
    {
        SDL_Log("Couldn't deduce shader stage from file name: %s", shaderFilename.c_str());
        return nullptr;
    }
    std::filesystem::path fullPath;
    #if PRODUCTION_BUILD==1
        fullPath = std::filesystem::path{SDL_GetBasePath()} / "shaders";
    #else
        fullPath = SHADER_PATH;
    #endif

    SDL_GPUShaderFormat format {SDL_GPU_SHADERFORMAT_INVALID};
    const char* entryPoint;

    SDL_GPUShaderFormat backendFormats {SDL_GetGPUShaderFormats(device)};

    if(backendFormats & SDL_GPU_SHADERFORMAT_SPIRV)
    {
        fullPath /= shaderFilename + ".spv";
        format = SDL_GPU_SHADERFORMAT_SPIRV;
        entryPoint = "main";
    }
    else if(backendFormats & SDL_GPU_SHADERFORMAT_MSL)
    {
        fullPath /= shaderFilename + ".msl";
        format = SDL_GPU_SHADERFORMAT_MSL;
        entryPoint = "main0";
    }
    else if(backendFormats & SDL_GPU_SHADERFORMAT_DXIL)
    {
        fullPath /= shaderFilename + ".dxil";
        format = SDL_GPU_SHADERFORMAT_DXIL;
        entryPoint = "main";
    }
    else
    {
        SDL_Log("Couldn't find a supported shader for backend type %s!", SDL_GetGPUDeviceDriver(device));
        return nullptr;
    }
    size_t fileSize;
    void* code {SDL_LoadFile(fullPath.c_str(), &fileSize)};
    if (code == nullptr)
    {
        SDL_Log("Couldn't load shader file from disk!\n\t%s", SDL_GetError());
        return nullptr;
    }
    SDL_GPUShaderCreateInfo shaderInfo {SDL_GPUShaderCreateInfo{
        .code_size = fileSize,
        .code = static_cast<Uint8*>(code),
        .entrypoint = entryPoint,
        .format = format,
        .stage = stage,
        .num_samplers = dlShaderInfo.numSamplers,
        .num_storage_textures = dlShaderInfo.numStorageTextures,
        .num_storage_buffers = dlShaderInfo.numStorageBuffers,
        .num_uniform_buffers = dlShaderInfo.numUniformBuffers
    }};

    SDL_GPUShader* shader {SDL_CreateGPUShader(device, &shaderInfo)};
    if (shader == nullptr)
    {
        SDL_Log("Couldn't create shader from file %s: %s", fullPath.c_str(), SDL_GetError());
    }

    SDL_free(code);

    return shader;
}
#endif