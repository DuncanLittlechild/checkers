#ifndef DL_SDLGPU_CAMERA3D_H
#define DL_SDLGPU_CAMERA3D_H
#include "Matrix4x4.h"
struct Camera3d{
	static constexpr Vector3 worldUp {0.0f, 1.0f, 0.0f};
	Vector3 pos{0.0f, 1.0f, 0.0f};
	Vector3 target{1.0f, 0.0f, 1.0f};
	float zRot {};

	double fov {60.0 * SDL_PI_D/180.0};
	double aspectRatio{};

	double near {0.1};
	double far {10.0};

	Matrix4x4 viewMat {};
	Matrix4x4 projectionMat{};
	Matrix4x4 vpMat {};

	bool moveMade{false};

	void Init(double w, double h)
	{
		UpdateAspectRatio(w,h);
		UpdateProjMat();
		UpdateViewMat();
		UpdateVpMat();
	}

	void UpdateCam(float xMov, float zMov, float xCamMov, float yCamMov)
	{
		Vector3 xAx {viewMat.m11, viewMat.m21, viewMat.m31};
		Vector3 yAx {viewMat.m12, viewMat.m22, viewMat.m32};
		Vector3 zAx {viewMat.m13, viewMat.m23, viewMat.m33};
		Vector3 posChangeVec {xMov * xAx + zMov * zAx};
		Vector3 camChangeVec {posChangeVec + xCamMov * xAx + yCamMov * yAx};
		pos += posChangeVec;
		target += camChangeVec;
	}

	void UpdateViewMat()
	{
		viewMat = Matrix4x4_CreateViewMatrix(pos, target, worldUp);
	}

	void UpdateProjMat()
	{
		projectionMat = Matrix4x4_CreateProjectionMatrix(fov, aspectRatio, near, far);
	}
	void UpdateAspectRatio(double w, double h)
	{
		aspectRatio = (double)w/h;
		UpdateProjMat();
	}
	void UpdateVpMat()
	{
		vpMat = Matrix4x4_Multiply(viewMat, projectionMat);
	}
};

#endif