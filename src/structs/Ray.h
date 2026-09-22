#ifndef DL_CHESS_RAY_H
#define DL_CHESS_RAY_H
#include <optional>
#include "SDL3/SDL_mouse.h"
#include "structs/Camera3d.h"
#include "structs/Matrix4x4.h"
#include "structs/Vector3.h"
struct Ray {
    Vector3 origin{};
    Vector3 dir{};

    Ray(Vector3 g_start, Vector3 g_end)
        : origin{g_start}
        , dir {g_end - g_start}
    {}
};

inline Ray GetRayFromScreenCoordinate(int width, int height, float clickX, float clickY, Camera3d* camera)
{
    float xP {(clickX/width) * 2 - 1};
    float yP {-(clickY/height) * 2 + 1};
    
    Vector3 pN {xP, yP, .0f};
    Vector3 pF {xP, yP, 1.0f};
    Matrix4x4 invVp {Matrix4x4_Invert(camera->vpMat)};

    Vector3 rayBegin{Vector3_Transform(pN, invVp)};
    Vector3 rayEnd {Vector3_Transform(pF, invVp)};
    return {rayBegin, rayEnd};
}

inline Vector3 GetRayIntersectFlatPlane(const Ray& ray, const Vector3& planeNormal)
{
    float parallelCheck{Vector3_Dot(ray.dir, planeNormal)};
    if (parallelCheck < 1e-8 && parallelCheck > -1e-8)
    {
        ;
    }
    // Determine how far along the line the ray intersects the plane
    float t {(-ray.origin.y) / ray.dir.y};
    return {ray.origin.x + ray.dir.x * t, ray.origin.y + ray.dir.y * t, ray.origin.z + ray.dir.z * t};
}

#endif