#ifndef DL_CHESS_LIGHTSOURCE_H
#define DL_CHESS_LIGHTSOURCE_H

#include "structs/Vector3.h"
#include "structs/Matrix4x4.h"
struct PointLight {
    Vector3 pos{0.0f, 1.0f, 0.0f};
	Vector3 target{1.0f, 0.0f, 1.0f};

    double lightRadius{};

	double near {0.1};
	double far {100.0};

    Matrix4x4 viewMat {};
	Matrix4x4 projectionMat{};
	Matrix4x4 vpMat {};

	void Init(double r, Vector3 newPos, Vector3 newTarget)
	{
		lightRadius = r;
		pos = newPos;
		target = newTarget;

		UpdateVpMat();
	}

	void UpdateViewMat()
	{
		static Vector3 worldup {0.0f, 1.0f, 0.0f};
		viewMat = Matrix4x4_CreateViewMatrix(pos, target, worldup);
	}

	void UpdateProjMat()
	{
		projectionMat = Matrix4x4_CreateProjectionMatrix(lightRadius, 1.0, near, far);
	}
	void UpdateVpMat()
	{
		UpdateViewMat();
		UpdateProjMat();
		vpMat = Matrix4x4_Multiply(viewMat, projectionMat);
	}
};

#endif