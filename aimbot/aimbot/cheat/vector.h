#pragma once
#include <numbers>
#include <cmath>

struct CVector3
{
	// Constructor
	CVector3(
		const float x = 0.0f,
		const float y = 0.0f,
		const float z = 0.0f) noexcept :
		x(x), y(y), z(z) { }

	// Bunch of operator overloads
	const CVector3 operator-(const CVector3& other) const noexcept
	{
		return CVector3{ x - other.x, y - other.y, z - other.z };
	}
	const CVector3 operator+(const CVector3& other) const noexcept
	{
		return CVector3{ x + other.x, y + other.y, z + other.z };
	}
	const CVector3 operator/(const float factor) const noexcept
	{
		return CVector3{ x / factor, y / factor, z / factor };
	}
	const CVector3 operator*(const float factor) const noexcept
	{
		return CVector3{ x * factor, y * factor, z * factor };
	}

	// Converting to angle
	const CVector3 ToAngle() const noexcept
	{
		return CVector3{
			std::atan2(-z, std::hypot(x, y)) * (180.0f / std::numbers::pi_v<float>),
			std::atan2(y, x) * (180.0f / std::numbers::pi_v<float>),
			0.0f };
	}

	// Checking if zero
	const bool IsZero() const noexcept
	{
		return x == 0.f && y == 0.f && z == 0.f;
	}

	// Variables
	float x, y, z;
};