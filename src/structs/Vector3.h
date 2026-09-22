#ifndef DL_CHESS_VECTOR3_H
#define DL_CHESS_VECTOR3_H
#include "SDL3/SDL.h"

struct Vector3
{
	float x{};
	float y{};
	float z{};

	Vector3& operator+=(const Vector3& rhs)
	{
		x += rhs.x;
		y += rhs.y;
		z += rhs.z;
		return *this;
	}

	Vector3 operator+(const Vector3& rhs)
	{
		Vector3 tmp{*this};
		return tmp += rhs;
	}

	Vector3& operator-=(const Vector3& rhs)
	{
		x -= rhs.x;
		y -= rhs.y;
		z -= rhs.z;
		return *this;
	}

	Vector3 operator-(const Vector3& rhs)
	{
		Vector3 tmp{*this};
		return tmp -=rhs;
	}
};

inline Vector3 operator*(const Vector3& vec, float scal)
{
	Vector3 tmp{vec};
	tmp.x *= scal;
	tmp.y *= scal;
	tmp.z *= scal;
	return tmp;
}

inline Vector3 operator*(float scal, const Vector3& vec)
{
	Vector3 tmp{vec};
	tmp.x *= scal;
	tmp.y *= scal;
	tmp.z *= scal;
	return tmp;
}

inline Vector3 Vector3_Normalize(Vector3 vec)
{
	float magnitude = SDL_sqrtf((vec.x * vec.x) + (vec.y * vec.y) + (vec.z * vec.z));
	return (Vector3) {
		vec.x / magnitude,
		vec.y / magnitude,
		vec.z / magnitude
	};
}

inline float Vector3_Dot(Vector3 vecA, Vector3 vecB)
{
	return (vecA.x * vecB.x) + (vecA.y * vecB.y) + (vecA.z * vecB.z);
}

inline Vector3 Vector3_Cross(Vector3 vecA, Vector3 vecB)
{
	return (Vector3) {
		vecA.y * vecB.z - vecB.y * vecA.z,
		-(vecA.x * vecB.z - vecB.x * vecA.z),
		vecA.x * vecB.y - vecB.x * vecA.y
	};
}



#endif