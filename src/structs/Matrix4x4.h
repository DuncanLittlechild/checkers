#ifndef DL_SDLGPU_MATRICES_H
#define DL_SDLGPU_MATRICES_H

#include "Vector3.h"
// Matrix Math
typedef struct Matrix4x4
{
	float m11, m12, m13, m14;
	float m21, m22, m23, m24;
	float m31, m32, m33, m34;
	float m41, m42, m43, m44;
} Matrix4x4;

constexpr Matrix4x4 IDENTITYMATRIX{
	1,0,0,0,
	0,1,0,0,
	0,0,1,0,
	0,0,0,1
};

struct MVPMatrix
{
	Matrix4x4 m{IDENTITYMATRIX};
	Matrix4x4 vp{IDENTITYMATRIX};
};

inline Matrix4x4 Matrix4x4_Multiply(const Matrix4x4& matrix1, const Matrix4x4& matrix2)
{
	Matrix4x4 result;

	result.m11 = (
		(matrix1.m11 * matrix2.m11) +
		(matrix1.m12 * matrix2.m21) +
		(matrix1.m13 * matrix2.m31) +
		(matrix1.m14 * matrix2.m41)
	);
	result.m12 = (
		(matrix1.m11 * matrix2.m12) +
		(matrix1.m12 * matrix2.m22) +
		(matrix1.m13 * matrix2.m32) +
		(matrix1.m14 * matrix2.m42)
	);
	result.m13 = (
		(matrix1.m11 * matrix2.m13) +
		(matrix1.m12 * matrix2.m23) +
		(matrix1.m13 * matrix2.m33) +
		(matrix1.m14 * matrix2.m43)
	);
	result.m14 = (
		(matrix1.m11 * matrix2.m14) +
		(matrix1.m12 * matrix2.m24) +
		(matrix1.m13 * matrix2.m34) +
		(matrix1.m14 * matrix2.m44)
	);
	result.m21 = (
		(matrix1.m21 * matrix2.m11) +
		(matrix1.m22 * matrix2.m21) +
		(matrix1.m23 * matrix2.m31) +
		(matrix1.m24 * matrix2.m41)
	);
	result.m22 = (
		(matrix1.m21 * matrix2.m12) +
		(matrix1.m22 * matrix2.m22) +
		(matrix1.m23 * matrix2.m32) +
		(matrix1.m24 * matrix2.m42)
	);
	result.m23 = (
		(matrix1.m21 * matrix2.m13) +
		(matrix1.m22 * matrix2.m23) +
		(matrix1.m23 * matrix2.m33) +
		(matrix1.m24 * matrix2.m43)
	);
	result.m24 = (
		(matrix1.m21 * matrix2.m14) +
		(matrix1.m22 * matrix2.m24) +
		(matrix1.m23 * matrix2.m34) +
		(matrix1.m24 * matrix2.m44)
	);
	result.m31 = (
		(matrix1.m31 * matrix2.m11) +
		(matrix1.m32 * matrix2.m21) +
		(matrix1.m33 * matrix2.m31) +
		(matrix1.m34 * matrix2.m41)
	);
	result.m32 = (
		(matrix1.m31 * matrix2.m12) +
		(matrix1.m32 * matrix2.m22) +
		(matrix1.m33 * matrix2.m32) +
		(matrix1.m34 * matrix2.m42)
	);
	result.m33 = (
		(matrix1.m31 * matrix2.m13) +
		(matrix1.m32 * matrix2.m23) +
		(matrix1.m33 * matrix2.m33) +
		(matrix1.m34 * matrix2.m43)
	);
	result.m34 = (
		(matrix1.m31 * matrix2.m14) +
		(matrix1.m32 * matrix2.m24) +
		(matrix1.m33 * matrix2.m34) +
		(matrix1.m34 * matrix2.m44)
	);
	result.m41 = (
		(matrix1.m41 * matrix2.m11) +
		(matrix1.m42 * matrix2.m21) +
		(matrix1.m43 * matrix2.m31) +
		(matrix1.m44 * matrix2.m41)
	);
	result.m42 = (
		(matrix1.m41 * matrix2.m12) +
		(matrix1.m42 * matrix2.m22) +
		(matrix1.m43 * matrix2.m32) +
		(matrix1.m44 * matrix2.m42)
	);
	result.m43 = (
		(matrix1.m41 * matrix2.m13) +
		(matrix1.m42 * matrix2.m23) +
		(matrix1.m43 * matrix2.m33) +
		(matrix1.m44 * matrix2.m43)
	);
	result.m44 = (
		(matrix1.m41 * matrix2.m14) +
		(matrix1.m42 * matrix2.m24) +
		(matrix1.m43 * matrix2.m34) +
		(matrix1.m44 * matrix2.m44)
	);

	return result;
}

inline Matrix4x4 Matrix4x4_Invert(Matrix4x4 m)
{
	// Cache 2x2 determinants from the bottom two rows, reused across
	// multiple cofactors below (same technique the original XNA/MonoGame
	// Matrix.Invert implementation uses, to avoid recomputing shared terms).
	float b00 = m.m31 * m.m42 - m.m32 * m.m41;
	float b01 = m.m31 * m.m43 - m.m33 * m.m41;
	float b02 = m.m31 * m.m44 - m.m34 * m.m41;
	float b03 = m.m32 * m.m43 - m.m33 * m.m42;
	float b04 = m.m32 * m.m44 - m.m34 * m.m42;
	float b05 = m.m33 * m.m44 - m.m34 * m.m43;

	float d11 =  (m.m22 * b05 - m.m23 * b04 + m.m24 * b03);
	float d12 = -(m.m21 * b05 - m.m23 * b02 + m.m24 * b01);
	float d13 =  (m.m21 * b04 - m.m22 * b02 + m.m24 * b00);
	float d14 = -(m.m21 * b03 - m.m22 * b01 + m.m23 * b00);

	float det = m.m11 * d11 + m.m12 * d12 + m.m13 * d13 + m.m14 * d14;

	// A near-zero determinant means the matrix is singular (non-invertible) --
	// e.g. a degenerate/scaled-to-zero transform. Guard against divide-by-zero
	// rather than returning garbage silently.
	if (SDL_fabsf(det) < 1e-8f)
	{
		return (Matrix4x4) {
			1,0,0,0,
			0,1,0,0,
			0,0,1,0,
			0,0,0,1
		};
	}

	float invDet = 1.0f / det;

	float b06 = m.m21 * m.m42 - m.m22 * m.m41;
	float b07 = m.m21 * m.m43 - m.m23 * m.m41;
	float b08 = m.m21 * m.m44 - m.m24 * m.m41;
	float b09 = m.m22 * m.m43 - m.m23 * m.m42;
	float b10 = m.m22 * m.m44 - m.m24 * m.m42;
	float b11 = m.m23 * m.m44 - m.m24 * m.m43;

	float b12 = m.m21 * m.m32 - m.m22 * m.m31;
	float b13 = m.m21 * m.m33 - m.m23 * m.m31;
	float b14 = m.m21 * m.m34 - m.m24 * m.m31;
	float b15 = m.m22 * m.m33 - m.m23 * m.m32;
	float b16 = m.m22 * m.m34 - m.m24 * m.m32;
	float b17 = m.m23 * m.m34 - m.m24 * m.m33;

	Matrix4x4 result;

	result.m11 = d11 * invDet;
	result.m21 = d12 * invDet;
	result.m31 = d13 * invDet;
	result.m41 = d14 * invDet;

	result.m12 = -(m.m12 * b05 - m.m13 * b04 + m.m14 * b03) * invDet;
	result.m22 =  (m.m11 * b05 - m.m13 * b02 + m.m14 * b01) * invDet;
	result.m32 = -(m.m11 * b04 - m.m12 * b02 + m.m14 * b00) * invDet;
	result.m42 =  (m.m11 * b03 - m.m12 * b01 + m.m13 * b00) * invDet;

	result.m13 =  (m.m14 * b09 - m.m13 * b10 + m.m12 * b11) * invDet;
	result.m23 = -(m.m14 * b07 - m.m13 * b08 + m.m11 * b11) * invDet;
	result.m33 =  (m.m14 * b06 - m.m12 * b08 + m.m11 * b10) * invDet;
	result.m43 = -(m.m13 * b06 - m.m12 * b07 + m.m11 * b09) * invDet;

	result.m14 = -(m.m12 * b17 - m.m13 * b16 + m.m14 * b15) * invDet;
	result.m24 =  (m.m11 * b17 - m.m13 * b14 + m.m14 * b13) * invDet;
	result.m34 = -(m.m11 * b16 - m.m12 * b14 + m.m14 * b12) * invDet;
	result.m44 =  (m.m11 * b15 - m.m12 * b13 + m.m13 * b12) * invDet;

	return result;
}

inline Matrix4x4 Matrix4x4_CreateRotationZ(float radians)
{
	return (Matrix4x4) {
		 SDL_cosf(radians), SDL_sinf(radians), 0, 0,
		-SDL_sinf(radians), SDL_cosf(radians), 0, 0,
						 0, 				0, 1, 0,
						 0,					0, 0, 1
	};
}

inline Matrix4x4 Matrix4x4_CreateTranslation(float x, float y, float z)
{
	return (Matrix4x4) {
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		x, y, z, 1
	};
}

inline Matrix4x4 Matrix4x4_CreateScale(float scale)
{
	return (Matrix4x4){
		scale, 0, 0, 0,
		0, scale, 0, 0,
		0, 0, scale, 0,
		0, 0,0, 1
	};
}

inline Matrix4x4 Matrix4x4_CreateOrthographicProjectionMatrix(
	float left,
	float right,
	float bottom,
	float top,
	float zNearPlane,
	float zFarPlane
) {
	return (Matrix4x4) {
		2.0f / (right - left), 0, 0, 0,
		0, 2.0f / (top - bottom), 0, 0,
		0, 0, 1.0f / (zNearPlane - zFarPlane), 0,
		(left + right) / (left - right), (top + bottom) / (bottom - top), zNearPlane / (zNearPlane - zFarPlane), 1
	};
}

inline Matrix4x4 Matrix4x4_CreateProjectionMatrix(
	float fieldOfView,
	float aspectRatio,
	float nearPlaneDistance,
	float farPlaneDistance
) {
	float num = 1.0f / ((float) SDL_tanf(fieldOfView * 0.5f));
	return (Matrix4x4) {
		num / aspectRatio, 0, 0, 0,
		0, num, 0, 0,
		0, 0, farPlaneDistance / (nearPlaneDistance - farPlaneDistance), -1,
		0, 0, (nearPlaneDistance * farPlaneDistance) / (nearPlaneDistance - farPlaneDistance), 0
	};
}

inline Matrix4x4 Matrix4x4_CreateViewMatrix(
	Vector3 cameraPosition,
	Vector3 cameraTarget,
	Vector3 worldUpVector
) {
	Vector3 targetToPosition = {
		cameraPosition.x - cameraTarget.x,
		cameraPosition.y - cameraTarget.y,
		cameraPosition.z - cameraTarget.z
	};
	Vector3 vectorA = Vector3_Normalize(targetToPosition);
	Vector3 vectorB = Vector3_Normalize(Vector3_Cross(worldUpVector, vectorA));
	Vector3 vectorC = Vector3_Cross(vectorA, vectorB);

	return (Matrix4x4) {
		vectorB.x, vectorC.x, vectorA.x, 0,
		vectorB.y, vectorC.y, vectorA.y, 0,
		vectorB.z, vectorC.z, vectorA.z, 0,
		-Vector3_Dot(vectorB, cameraPosition), -Vector3_Dot(vectorC, cameraPosition), -Vector3_Dot(vectorA, cameraPosition), 1
	};
}

inline Vector3 Vector3_Transform(Vector3 vec, Matrix4x4 m)
{
	// Treat vec as a position with an implicit w = 1 (row-vector convention,
	// matching this file: v * M). Compute the full 4-component result,
	// including w, rather than assuming w stays 1.
	float x = (vec.x * m.m11) + (vec.y * m.m21) + (vec.z * m.m31) + m.m41;
	float y = (vec.x * m.m12) + (vec.y * m.m22) + (vec.z * m.m32) + m.m42;
	float z = (vec.x * m.m13) + (vec.y * m.m23) + (vec.z * m.m33) + m.m43;
	float w = (vec.x * m.m14) + (vec.y * m.m24) + (vec.z * m.m34) + m.m44;

	// Perspective divide -- required whenever M might not be a pure
	// affine transform (e.g. inverting a view-projection matrix, as in
	// your raycasting case). For a pure model/view matrix, w is always
	// exactly 1 here and this divide is a harmless no-op.
	if (SDL_fabsf(w) > 1e-8f)
	{
		return (Vector3) { x / w, y / w, z / w };
	}
	
	return (Vector3) { x, y, z };
}

#endif