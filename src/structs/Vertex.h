#ifndef DL_CHESS_VERTEX_H
#define DL_CHESS_VERTEX_H
#include "Vector3.h"
struct Vertex{
    Vector3 pos{};
    Vector3 norms{};
    Vector3 colour{};
};

static_assert(sizeof(Vertex) == 36, "Vertex has unmarked padding");

#endif