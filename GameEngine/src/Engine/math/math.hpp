#pragma once

#include <utility>
#include <concepts>
#include <stdexcept>

namespace Math
{
	template <typename Type>
		requires std::is_arithmetic_v<Type>
	struct eVec2
	{
		Type v1, v2;

		// ------------------------------- Addition -------------------------------
		eVec2& operator+=(const eVec2& other)
		{
			this->v1 += other.v1;
			this->v2 += other.v2;

			return *this;
		}

		template <typename Type>
			requires std::is_arithmetic_v<Type>
		eVec2& operator+=(const Type& other)
		{
			this->v1 += other;
			this->v2 += other;

			return *this;
		}

		eVec2 operator+(const eVec2& other)
		{
			return { v1 + other.v1, v2 + other.v2 };
		}

		// ------------------------------ Subtraction ------------------------------
		eVec2& operator-=(const eVec2& other)
		{
			this->v1 -= other.v1;
			this->v2 -= other.v2;

			return *this;
		}

		template <typename Type>
			requires std::is_arithmetic_v<Type>
		eVec2& operator-=(const Type& other)
		{
			this->v1 -= other;
			this->v2 -= other;

			return *this;
		}

		eVec2 operator-(const eVec2& other)
		{
			return { v1 - other.v1, v2 - other.v2 };
		}

		// ----------------------------- Multiplication -----------------------------
		eVec2& operator*=(const eVec2& other)
		{
			this->v1 *= other.v1;
			this->v2 *= other.v2;

			return *this;
		}

		template <typename Type>
			requires std::is_arithmetic_v<Type>
		eVec2& operator*=(const Type& other)
		{
			this->v1 *= other;
			this->v2 *= other;

			return *this;
		}

		eVec2 operator*(const eVec2& other)
		{
			return { v1 * other.v1, v2 * other.v2 };
		}

		// ------------------------------ Division ------------------------------
		eVec2& operator/=(const eVec2& other)
		{
			if (other.v1 == 0 ||
				other.v2 == 0)
			{
				throw std::runtime_error("Division by zero on a vector!");
			}

			this->v1 /= other.v1;
			this->v2 /= other.v2;

			return *this;
		}

		template <typename Type>
			requires std::is_arithmetic_v<Type>
		eVec2& operator/=(const Type& other)
		{
			if (other == 0)
			{
				throw std::runtime_error("Division by zero on a vector!");
			}

			this->v1 /= other;
			this->v2 /= other;

			return *this;
		}

		eVec2 operator/(const eVec2& other)
		{
			if (other.v1 == 0 ||
				other.v2 == 0)
			{
				throw std::runtime_error("Division by zero on a vector!");
			}

			return { v1 / other.v1, v2 / other.v2 };
		}
	};

	template <typename Type>
		requires std::is_arithmetic_v<Type>
	struct eVec3
	{
		Type v1, v2, v3;
		
		// ------------------------------- Addition -------------------------------
		eVec3& operator+=(const eVec3& other)
		{
			this->v1 += other.v1;
			this->v2 += other.v2;
			this->v3 += other.v3;

			return *this;
		}

		template <typename Type>
			requires std::is_arithmetic_v<Type>
		eVec3& operator+=(const Type& other)
		{
			this->v1 += other;
			this->v2 += other;
			this->v3 += other;

			return *this;
		}

		eVec3 operator+(const eVec3& other)
		{
			return { v1 + other.v1, v2 + other.v2, v3 + other.v3 };
		}

		// ------------------------------ Subtraction ------------------------------
		eVec3& operator-=(const eVec3& other)
		{
			this->v1 -= other.v1;
			this->v2 -= other.v2;
			this->v3 -= other.v3;

			return *this;
		}

		template <typename Type>
			requires std::is_arithmetic_v<Type>
		eVec3& operator-=(const Type& other)
		{
			this->v1 -= other;
			this->v2 -= other;
			this->v3 -= other;

			return *this;
		}

		eVec3 operator-(const eVec3& other)
		{
			return { v1 - other.v1, v2 - other.v2, v3 - other.v3 };
		}

		// ----------------------------- Multiplication -----------------------------
		eVec3& operator*=(const eVec3& other)
		{
			this->v1 *= other.v1;
			this->v2 *= other.v2;
			this->v3 *= other.v3;

			return *this;
		}

		template <typename Type>
			requires std::is_arithmetic_v<Type>
		eVec3& operator*=(const Type& other)
		{
			this->v1 *= other;
			this->v2 *= other;
			this->v3 *= other;

			return *this;
		}

		eVec3 operator*(const eVec3& other)
		{
			return { v1 * other.v1, v2 * other.v2, v3 * other.v3 };
		}

		// ------------------------------ Division ------------------------------
		eVec3& operator/=(const eVec3& other)
		{
			if (other.v1 == 0 ||
				other.v2 == 0 ||
				other.v3 == 0)
			{
				throw std::runtime_error("Division by zero on a vector!");
			}

			this->v1 /= other.v1;
			this->v2 /= other.v2;
			this->v3 /= other.v3;

			return *this;
		}

		template <typename Type>
			requires std::is_arithmetic_v<Type>
		eVec3& operator/=(const Type& other)
		{
			if (other == 0)
			{
				throw std::runtime_error("Division by zero on a vector!");
			}

			this->v1 /= other;
			this->v2 /= other;
			this->v3 /= other;

			return *this;
		}

		eVec3 operator/(const eVec3& other)
		{
			if (other.v1 == 0 ||
				other.v2 == 0 ||
				other.v3 == 0)
			{
				throw std::runtime_error("Division by zero on a vector!");
			}

			return { v1 / other.v1, v2 / other.v2, v3 / other.v3 };
		}
	};

	template <typename Type>
		requires std::is_arithmetic_v<Type>
	struct eMtx2x2
	{
		Type v1, v2, 
			 v3, v4;


	};

	template <typename Type>
		requires std::is_arithmetic_v<Type>
	struct eMtx3x3
	{
		Type v1, v2, v3, 
			 v4, v5, v6, 
			 v7, v8, v9;


	};

	template <typename Type>
		requires std::is_arithmetic_v<Type>
	struct eMtx4x4
	{
		Type v1, v2, v3, v4, 
			 v5, v6, v7, v8, 
			 v9, v10, v11, v12;


	};
}