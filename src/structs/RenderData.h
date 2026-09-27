#ifndef DL_CHESS_RENDERDATA_H
#define DL_CHESS_RENDERDATA_H
#include "structs/AssetManager.h"
struct RenderData {
    GPUMesh* mesh{nullptr};
    Material* material{nullptr};
    Vector3 offset{0.0f, 0.0f, 0.0f};
    float scale{};
};

#endif