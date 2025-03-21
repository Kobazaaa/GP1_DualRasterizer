#include "Vector3.h"

#include <cassert>

#include "Vector4.h"
#include <cmath>

#include "MathHelpers.h"
#include "Vector2.h"

namespace dae {
	const Vector3 Vector3::UnitX = Vector3{ 1, 0, 0 };
	const Vector3 Vector3::UnitY = Vector3{ 0, 1, 0 };
	const Vector3 Vector3::UnitZ = Vector3{ 0, 0, 1 };
	const Vector3 Vector3::Zero = Vector3{ 0, 0, 0 };

	Vector3 Vector3::Max(const Vector3& v1, const Vector3& v2)
	{
		return {
			std::fmax(v1.x, v2.x),
			std::fmax(v1.y, v2.y),
			std::fmax(v1.z, v2.z)
		};
	}
	Vector3 Vector3::Min(const Vector3& v1, const Vector3& v2)
	{
		return {
			std::fmin(v1.x, v2.x),
			std::fmin(v1.y, v2.y),
			std::fmin(v1.z, v2.z)
		};
	}

	bool Vector3::NearZero()
	{
		float epsilon = 0.0001f;
		return (std::abs(x) < epsilon) and (std::abs(y) < epsilon) and (std::abs(z) < epsilon);
	}


	Vector3::Vector3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

	Vector3::Vector3(const Vector4& v) : x(v.x), y(v.y), z(v.z) {}

	Vector3::Vector3(const Vector3& from, const Vector3& to) : x(to.x - from.x), y(to.y - from.y), z(to.z - from.z) {}

	float Vector3::Magnitude() const
	{
		return sqrtf(x * x + y * y + z * z);
	}

	float Vector3::SqrMagnitude() const
	{
		return x * x + y * y + z * z;
	}

	float Vector3::Normalize()
	{
		const float m = Magnitude();
		const float mInv = 1.f / m;
		x *= mInv;
		y *= mInv;
		z *= mInv;

		return m;
	}

	Vector3 Vector3::Normalized() const
	{
		const float m = Magnitude();
		const float mInv = 1.f / m;
		return { x * mInv, y * mInv, z * mInv };
	}

	float Vector3::Dot(const Vector3& v1, const Vector3& v2)
	{
		return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
	}

	Vector3 Vector3::Cross(const Vector3& v1, const Vector3& v2)
	{
		return Vector3{
			v1.y * v2.z - v1.z * v2.y,
			v1.z * v2.x - v1.x * v2.z,
			v1.x * v2.y - v1.y * v2.x
		};
	}

	Vector3 Vector3::Project(const Vector3& v1, const Vector3& v2)
	{
		return (v2 * (Dot(v1, v2) / Dot(v2, v2)));
	}

	Vector3 Vector3::Reject(const Vector3& v1, const Vector3& v2)
	{
		return (v1 - v2 * (Dot(v1, v2) / Dot(v2, v2)));
	}

	Vector3 Vector3::Reflect(const Vector3& v1, const Vector3& v2)
	{
		return v1 - (2.f * Vector3::Dot(v1, v2) * v2);
	}

	Vector4 Vector3::ToPoint4() const
	{
		return { x, y, z, 1 };
	}

	Vector4 Vector3::ToVector4() const
	{
		return { x, y, z, 0 };
	}

	Vector2 Vector3::GetXY() const
	{
		return { x, y };
	}

#pragma region Operator Overloads
	Vector3 Vector3::operator*(float scale) const
	{
		return { x * scale, y * scale, z * scale };
	}

	Vector3 Vector3::operator/(float scale) const
	{
		const float invScale = 1.f / scale;
		return { x * invScale, y * invScale, z * invScale };
	}

	Vector3 Vector3::operator+(const Vector3& v) const
	{
		return { x + v.x, y + v.y, z + v.z };
	}

	Vector3 Vector3::operator-(const Vector3& v) const
	{
		return { x - v.x, y - v.y, z - v.z };
	}

	Vector3 Vector3::operator-() const
	{
		return { -x ,-y,-z };
	}

	Vector3& Vector3::operator*=(float scale)
	{
		x *= scale;
		y *= scale;
		z *= scale;
		return *this;
	}

	Vector3& Vector3::operator/=(float scale)
	{
		const float invScale = 1.f / scale;
		x *= invScale;
		y *= invScale;
		z *= invScale;
		return *this;
	}

	Vector3& Vector3::operator-=(const Vector3& v)
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;
		return *this;
	}

	Vector3& Vector3::operator+=(const Vector3& v)
	{
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	}

	float& Vector3::operator[](int index)
	{
		assert(index <= 2 && index >= 0);

		if (index == 0) return x;
		if (index == 1) return y;
		return z;
	}

	float Vector3::operator[](int index) const
	{
		assert(index <= 2 && index >= 0);

		if (index == 0) return x;
		if (index == 1) return y;
		return z;
	}

	bool Vector3::operator==(const Vector3& v) const
	{
		return AreEqual(x, v.x) && AreEqual(y, v.y) && AreEqual(z, v.z);
	}

#pragma endregion
}
