#ifndef DL_CHESS_PRIMITIVES_H
#define DL_CHESS_PRIMITIVES_H
#include "SDL3/SDL_stdinc.h"
#include "structs/Vertex.h"
#include <array>
#include <cassert>
#include <cstdint>
#include <vector>
#include <span>
struct Mesh{    
    std::vector<Vertex> vertices{};
    std::vector<uint16_t> indices{};
};

constexpr std::array<float, 9> CBC {-0.5, -0.375, -0.25, -0.125, 0.0, 0.125, 0.25, 0.375, 0.5};
inline Mesh CreateTileMesh()
{
    static constexpr std::array<Vertex, 4> TILEVERTICES {{
        {{-.5f, 0.0f, -.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
        {{-.5f, 0.0f, .5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        {{.5f, 0.0f, .5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 0.0f}},
        {{.5f, 0.0f, -.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}}
    }};
    static constexpr std::array<uint16_t, 6> TILEINDEX {{
        0, 1, 2,
        0, 2, 3
    }};

    return {{TILEVERTICES.begin(), TILEVERTICES.end()}, {TILEINDEX.begin(), TILEINDEX.end()}
    };
}
inline void GetFlatSquareVertices(std::vector<Vertex>& whiteVertices, std::vector<Vertex>& blackVertices, std::vector<uint16_t>& indices)
{
    constexpr static double increment {0.125};
    constexpr static Vector3 whiteCol {1.0,1.0, 1.0};
    constexpr static Vector3 blackCol {0.0,0.0, 0.0};
    constexpr static Vector3 norm {0.0,1.0,0.0};

    whiteVertices.resize(0);
    whiteVertices.reserve(128);
    blackVertices.resize(0);
    blackVertices.reserve(128);

    bool whiteSquare {false};

    for (int z {0}; z < 8; ++z)
    {
        for (int x{0}; x < 8; ++x)
        {
            if(whiteSquare)
            {
                whiteVertices.push_back({{CBC[x],0.0, CBC[z]},norm,whiteCol});
                whiteVertices.push_back({{CBC[x],0.0, CBC[z+1]},norm,whiteCol});
                whiteVertices.push_back({{CBC[x+1],0.0, CBC[z+1]},norm,whiteCol});
                whiteVertices.push_back({{CBC[x+1],0.0, CBC[z]},norm,whiteCol});
            }
            else
            {
                blackVertices.push_back({{CBC[x],0.0, CBC[z]},norm,blackCol});
                blackVertices.push_back({{CBC[x],0.0, CBC[z+1]},norm,blackCol});
                blackVertices.push_back({{CBC[x+1],0.0, CBC[z+1]},norm,blackCol});
                blackVertices.push_back({{CBC[x+1],0.0, CBC[z]},norm,blackCol});
            }
            whiteSquare = !whiteSquare;
        }
        whiteSquare = !whiteSquare;
    }
    indices.resize(0);
    indices.reserve(whiteVertices.size() * 3 / 2 );
    assert(whiteVertices.size() == 128 && "Vertex count incorrect!");
    for(int i {0}; i < whiteVertices.size(); i += 4)
    {
        indices.push_back(i);
        indices.push_back(i+1);
        indices.push_back(i+2);
        indices.push_back(i);
        indices.push_back(i+2);
        indices.push_back(i+3);
    }
}

// CUBE
std::array<Vertex, 24> cubeColourVertices = {{
    
    // Front (+Z) - red
    {{-0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
    {{ 0.5f, -0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
    {{ 0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},
    {{-0.5f,  0.5f,  0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}},

    // Back (-Z) - green
    {{ 0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}},
    {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}},
    {{-0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}},
    {{ 0.5f,  0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}},

    // Left (-X) - blue
    {{-0.5f, -0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f,  0.5f,  0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f,  0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}},

    // Right (+X) - yellow
    {{ 0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.0f}},
    {{ 0.5f,  0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.0f}},
    {{ 0.5f,  0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.0f}},
    {{ 0.5f, -0.5f,  0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.0f}},

    // Bottom (-Y) - magenta
    {{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 1.0f}},
    {{ 0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 1.0f}},
    {{ 0.5f, -0.5f,  0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 1.0f}},
    {{-0.5f, -0.5f,  0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f, 1.0f}},

    // Top (+Y) - cyan
    {{-0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 1.0f}},
    {{ 0.5f,  0.5f,  0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 1.0f}},
    {{ 0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 1.0f}},
    {{-0.5f,  0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 1.0f}},
}};

// 6 faces * 2 triangles * 3 indices = 36
std::array<uint16_t, 36> cubeIndices = {{
     0,  1,  2,   0,  2,  3,   // Front
     4,  5,  6,   4,  6,  7,   // Back
     8,  9, 10,   8, 10, 11,   // Left
    12, 13, 14,  12, 14, 15,   // Right
    16, 17, 18,  16, 18, 19,   // Bottom
    20, 21, 22,  20, 22, 23,   // Top
}};


inline Mesh CreateCubeMesh()
{
    Mesh mesh{
        .vertices {cubeColourVertices.begin(), cubeColourVertices.end()},
        .indices {cubeIndices.begin(), cubeIndices.end()}
    };
    
    return mesh;
}

constexpr float CHECKERSHEIGHT{0.125f};

inline Mesh MakeCheckersPiece(int sides = 16, float radius = 0.5f, float height = CHECKERSHEIGHT)
{
    Mesh mesh;
    const float halfHeight = height * 0.5f;
    const float PI = SDL_PI_F;

    // --- Side (smooth-shaded around the rim) ---
    // sides+1 columns (last duplicates the first, for a clean UV seam),
    // 2 rows (top ring, bottom ring). Each column shares one normal
    // between its top and bottom vertex, since the side's normal only
    // depends on angle, not height.
    int sideStart = (int)mesh.vertices.size();
    for (int i = 0; i <= sides; ++i)
    {
        float angle = (2.0f * PI * i) / sides;
        float x = SDL_cosf(angle);
        float z = SDL_sinf(angle);
        float u = (float)i / sides;

        Vertex top    { {x*radius, halfHeight, z*radius}, {x, 0.0f, z}, {u, 0.0f} };
        Vertex bottom { {x*radius, -halfHeight, z*radius}, {x, 0.0f, z}, {u, 1.0f} };
        mesh.vertices.push_back(top);
        mesh.vertices.push_back(bottom);
    }
    for (int i = 0; i < sides; ++i)
    {
        uint16_t topA = sideStart + i * 2;
        uint16_t botA = topA + 1;
        uint16_t topB = topA + 2;
        uint16_t botB = topA + 3;
        // two triangles per side quad, CCW from outside
        mesh.indices.insert(mesh.indices.end(), {topB, botB, botA});
        mesh.indices.insert(mesh.indices.end(), {topB, botA, topA});
    }

    // --- Caps (flat-shaded: own vertices, uniform normal, fan-triangulated) ---
    auto addCap = [&](float y, float normalY, bool flipWinding)
    {
        int centerIndex = (int)mesh.vertices.size();
        mesh.vertices.push_back({ {0.0f, y, 0.0f}, {0.0f, normalY, 0.0f}, {0.5f, 0.5f} });

        int rimStart = (int)mesh.vertices.size();
        for (int i = 0; i <= sides; ++i)
        {
            float angle = (2.0f * PI * i) / sides;
            float x = SDL_cosf(angle);
            float z = SDL_sinf(angle);
            float u = 0.5f + 0.5f * x;
            float v = 0.5f + 0.5f * z;
            mesh.vertices.push_back({ {x*radius, y, z*radius}, {0.0f, normalY, 0.0f}, {u, v} });
        }

        for (int i = 0; i < sides; ++i)
        {
            uint16_t a = rimStart + i;
            uint16_t b = rimStart + i + 1;
            if (!flipWinding)
                mesh.indices.insert(mesh.indices.end(), {(uint16_t)centerIndex, b, a});   // swapped
            else
                mesh.indices.insert(mesh.indices.end(), {(uint16_t)centerIndex, a, b});   // swapped
        }
    };

    addCap( halfHeight,  1.0f, false); // top cap, normal up
    addCap(-halfHeight, -1.0f, true);  // bottom cap, normal down, opposite winding

    return mesh;
}

#endif