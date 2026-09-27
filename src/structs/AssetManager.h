#ifndef DL_CHESS_ASSETMANAGER_H
#define DL_CHESS_ASSETMANAGER_H
#include "SDL3/SDL_gpu.h"
#include <cstring>
#include <sstream>
#include <string>
#include <vector>
#include "SDL3/SDL_iostream.h"
#include "SDL3/SDL_stdinc.h"
#include "SDL3/SDL_video.h"
#include "Vertex.h"
#include "helpers/dl_primitives.h"
#include "helpers/shaderUtils.h"
#include "helpers/dl_create_gpu_assets.h"

struct GPUMesh{
    SDL_GPUBuffer* vertices{nullptr};
    SDL_GPUBuffer* indices{nullptr};
    Uint32 indexCount{};

    bool Create(SDL_GPUDevice* device, const Mesh& mesh)
    {
        indexCount = mesh.indices.size();
        Uint32 verticesSize = sizeof(Vertex) * mesh.vertices.size();
        Uint32 indicesSize = sizeof(uint16_t) * mesh.indices.size();

        SDL_GPUBufferCreateInfo vertexCreateInfo {
            .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
            .size = verticesSize,
        };
        SDL_GPUBufferCreateInfo indexCreateInfo {
            .usage = SDL_GPU_BUFFERUSAGE_VERTEX,
            .size = verticesSize,
        };

        vertices = SDL_CreateGPUBuffer(device,&vertexCreateInfo);
        if(vertices == nullptr)
        {
            SDL_Log("Couldn't create vertex buffer");
            return false;   
        }
        indices = SDL_CreateGPUBuffer(device,&indexCreateInfo);
        if(indices == nullptr)
        {
            SDL_Log("Couldn't create index buffer");
            return false;   
        }

        SDL_GPUTransferBufferCreateInfo transferBufferCreateInfo {
            .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
            .size = verticesSize + indicesSize
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
        void* transferPtr{SDL_MapGPUTransferBuffer(device, transferBuffer, false)};
        SDL_memcpy(transferPtr, mesh.vertices.data(), verticesSize);
        void* indexTransferPtr {(char*)transferPtr + verticesSize}; 
        SDL_memcpy(indexTransferPtr, mesh.indices.data(), indicesSize);
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
            .buffer = vertices,
            .offset = 0,
            .size = verticesSize
        };
        SDL_UploadToGPUBuffer(copyPass, &bufferLocation, &bufferRegion, true);

        SDL_GPUTransferBufferLocation indexBufferLocation {
            .transfer_buffer = transferBuffer,
            .offset = verticesSize
        };
        SDL_GPUBufferRegion indexBufferRegion {
            .buffer = indices,
            .offset = 0,
            .size = indicesSize
        };
        SDL_UploadToGPUBuffer(copyPass, &indexBufferLocation, &indexBufferRegion, true);

        SDL_EndGPUCopyPass(copyPass);
        if(!SDL_SubmitGPUCommandBuffer(uploadCommandBuff))
        {
            SDL_Log("Couldn't submit GPU command buffer: %s", SDL_GetError());
            return false;
        }

        SDL_ReleaseGPUTransferBuffer(device, transferBuffer);
        return true;    
    }
};

struct Material{
    Vector3 albedo{};
    float gpuPadding{};
    Vector3 specularColour{};
    float shininess{};
};


struct AssetManager{
    GPUMesh tile{};
    GPUMesh piece{};
    GPUMesh king {};
    GPUMesh cube{};

    SDL_GPUGraphicsPipeline* pipeline {nullptr};
	SDL_GPUGraphicsPipeline* shadowPipeline{nullptr};

	SDL_GPUTexture* depth{nullptr};
	SDL_GPUTexture* shadows{nullptr};
	SDL_GPUSampler* shadowSampler{nullptr};

    Material tileMatW{};
    Material tileMatB{};

    Material pieceMatW{};
    Material pieceMatB{};
    Material pieceMatSelected{};

    Uint32 shadowRes{1024};

    bool Load(SDL_GPUDevice* device, SDL_Window* window, int w, int h)
    {
        tile.Create(device, CreateTileMesh());
        piece.Create(device, MakeCheckersPiece());
        king.Create(device, MakeCheckersPiece(16, 0.5f, CHECKERSHEIGHT * 2));
        cube.Create(device, CreateCubeMesh());
        /*
        tileMatW.albedo = {1.0f, 1.0f, 1.0f};
        tileMatW.specularColour = {1.0f, 1.0f, 1.0f};
        tileMatW.shininess = 120.f;

        tileMatB.albedo = {.0f, .0f, .0f};
        tileMatB.specularColour = {1.0f, 1.0f, 1.0f};
        tileMatB.shininess = 120.f;

        pieceMatW.albedo = {1.0f, 1.0f, 1.0f};
        pieceMatW.specularColour = {.05f, .05f, .05f};
        pieceMatW.shininess = 4.f;

        pieceMatB.albedo = {.0f, .0f, .0f};
        pieceMatB.specularColour = {.05f, .05f, .05f};
        pieceMatB.shininess = 4.f;

        pieceMatSelected.albedo = {1.0f, .0f, .0f};
        pieceMatSelected.specularColour = {1.0f, .0f, .0f};
        pieceMatSelected.shininess = 50.f;
        */
        LoadMatData();
        CreatePipelines(device, window);
        CreateDepthTextures(device, w, h);
        CreateGPUSampler(&shadowSampler, device);

        return true;
    };

    bool CreatePipelines(SDL_GPUDevice* device, SDL_Window* window)
	{
		DL_ShaderInfo vertexShaderInfo{
			.shaderFilename = "CameraPosition.vert",
			.numUniformBuffers = 2
		};
		DL_ShaderInfo fragmentShaderInfo{
			.shaderFilename = "Lighting.frag",
            .numSamplers = 1,
			.numUniformBuffers = 2,
		};
		CreatePipeline(
			&pipeline,
			device, window,
			&vertexShaderInfo, &fragmentShaderInfo
		);


		DL_ShaderInfo shadowVertexShaderInfo{
			.shaderFilename = "BasicPos.vert",
			.numUniformBuffers = 2
		};
		DL_ShaderInfo shadowFragmentShaderInfo{
			.shaderFilename = "BasicColour.frag",
			.numUniformBuffers = 1
		};
		CreatePipeline(
			&shadowPipeline,
			device, window,
			&shadowVertexShaderInfo, &shadowFragmentShaderInfo
		);

        return true;
	}

	void CreateDepthTextures(SDL_GPUDevice* device, int w, int h)
	{
		CreateDepthTexture(device, &depth, w, h);
		// Create shadow depth texture
        CreateShadowTexture(device);
	}

    void CreateShadowTexture(SDL_GPUDevice* device)
    {
        SDL_GPUTextureCreateInfo createInfo {
            .type = SDL_GPU_TEXTURETYPE_2D,
            .format = SDL_GPU_TEXTUREFORMAT_D32_FLOAT,
            .usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET|SDL_GPU_TEXTUREUSAGE_SAMPLER,
            .width = shadowRes,
            .height = shadowRes,
            .layer_count_or_depth = 1,
            .num_levels = 1
        };
        shadows = SDL_CreateGPUTexture(device, &createInfo);
    }

    bool LoadMatData()
    {
        auto SplitStringByChar {[](const char* string, char split, std::vector<std::string>& lines){
            std::string tmp{};
            for (int i {0}; string[i] != '\0'; ++i)
            {
                char c {string[i]};
                if(c != split)
                {
                    tmp += c;
                }
                else
                {
                    lines.push_back(tmp);
                    tmp.clear();
                }
            }
            lines.push_back(tmp);
        }};

        auto ExtractVector3FromStrArray {[](std::vector<std::string>& strArr) {
            return Vector3{std::stof(strArr[0]), std::stof(strArr[1]), std::stof(strArr[2])};
        }};
        std::size_t dataSize;
        char* file = static_cast<char*>(SDL_LoadFile(RESOURCES_PATH "materialData.txt",&dataSize));
        if (file == nullptr)
        {
            SDL_Log("Could not load file materialData.txt");
            return false;
        }
        std::vector<std::string> lines {};
        SplitStringByChar(file, '\n', lines);

        std::vector<std::string> sections{};
        std::vector<std::string> albedoStrArr{};
        std::vector<std::string> specularColourStrArr{};
        for (auto& line : lines)
        {
            SplitStringByChar(line.c_str(), ',', sections);

            SplitStringByChar(sections[1].c_str(), '|', albedoStrArr);
            SplitStringByChar(sections[2].c_str(), '|', specularColourStrArr);

            Vector3 albedo {ExtractVector3FromStrArray(albedoStrArr)};
            Vector3 specularColour {ExtractVector3FromStrArray(specularColourStrArr)};
            float shininess{std::stof(sections[3].c_str())};

            Material* mat;
            if(!strcmp(sections[0].c_str(), "tileW"))
                mat = &tileMatW;
            else if(!strcmp(sections[0].c_str(), "tileB"))
                mat = &tileMatB;
            else if(!strcmp(sections[0].c_str(), "pieceW"))
                mat = &pieceMatW;
            else if(!strcmp(sections[0].c_str(), "pieceB"))
                mat = &pieceMatB;
            else if(!strcmp(sections[0].c_str(), "pieceS"))
                mat = &pieceMatSelected;
            else
                assert(false && "materialData.txt configured incorrectly - line starts with unrecognised material\n");

            mat->albedo = albedo;
            mat->specularColour = specularColour;
            mat->shininess = shininess;

            albedoStrArr.clear();
            specularColourStrArr.clear();
            sections.clear();
        }
        SDL_free(file);
        return true;
    }

    bool SaveMatData()
    {
        auto MatToStr {[](const Material& material, std::string name) ->std::string {
            std::stringstream ss;
            ss << name << ',' 
                << material.albedo.x << '|' << material.albedo.y << '|' << material.albedo.z << ','
                << material.specularColour.x << '|' << material.specularColour.y << '|' << material.specularColour.z << ','
                << material.shininess;
            return ss.str();
        }};
        std::string out = MatToStr(tileMatW, "tileW");
        out += '\n';
        out += MatToStr(tileMatB, "tileB");
        out += '\n';
        out += MatToStr(pieceMatW, "pieceW");
        out += '\n';
        out += MatToStr(pieceMatB, "pieceB");
        out += '\n';
        out += MatToStr(pieceMatSelected, "pieceS");

        std::size_t dataSize;
        return SDL_SaveFile(RESOURCES_PATH "materialData.txt", out.c_str(), out.size());        
    }

};

#endif