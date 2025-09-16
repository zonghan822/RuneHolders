#pragma once

#include <utility>
#include <concepts>


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


		// ------------------------ Multiplication ----------------------------
		eMtx3x3& operator*=(const eMtx3x3& other)
		{
			//3x3 C, Cij = Ai1*B1j + Ai2*B2j + Ai3*B3j OR {row x column elems} 
			eMtx3x3 temp; // Temporary matrix to store results

			temp.v1 = (v1 * rhs.v1) + (v2 * rhs.v4) + (v3 * rhs.v7);
			temp.v2 = (v1 * rhs.v2) + (v2 * rhs.v5) + (v3 * rhs.v8);
			temp.v3 = (v1 * rhs.v3) + (v2 * rhs.v6) + (v3 * rhs.v9);

			temp.v4 = (v4 * rhs.v1) + (v5 * rhs.v4) + (v6 * rhs.v7);
			temp.v5 = (v4 * rhs.v2) + (v5 * rhs.v5) + (v6 * rhs.v8);
			temp.v6 = (v4 * rhs.v3) + (v5 * rhs.v6) + (v6 * rhs.v9);

			temp.v7 = (v7 * rhs.v1) + (v8 * rhs.v4) + (v9 * rhs.v7);
			temp.v8 = (v7 * rhs.v2) + (v8 * rhs.v5) + (v9 * rhs.v8);
			temp.v9 = (v7 * rhs.v3) + (v8 * rhs.v6) + (v9 * rhs.v9);

			// Assign the computed matrix back to this
			*this = temp; 

			return *this;

		}

		eMtx3x3 operator*(const eMtx3x3& lhs, const eMtx3x3& rhs)
		{
			//Create a new matrix for the result
			Matrix3x3 result; 

			result.v1 = (lhs.v1 * rhs.v1) + (lhs.v2 * rhs.v4) + (lhs.v3 * rhs.v7);
			result.v2 = (lhs.v1 * rhs.v2) + (lhs.v2 * rhs.v5) + (lhs.v3 * rhs.v8);
			result.v3 = (lhs.v1 * rhs.v3) + (lhs.v2 * rhs.v6) + (lhs.v3 * rhs.v9);

			result.v4 = (lhs.v4 * rhs.v1) + (lhs.v5 * rhs.v4) + (lhs.v6 * rhs.v7);
			result.v5 = (lhs.v4 * rhs.v2) + (lhs.v5 * rhs.v5) + (lhs.v6 * rhs.v8);
			result.v6 = (lhs.v4 * rhs.v3) + (lhs.v5 * rhs.v6) + (lhs.v6 * rhs.v9);

			result.v7 = (lhs.v7 * rhs.v1) + (lhs.v8 * rhs.v4) + (lhs.v9 * rhs.v7);
			result.v8 = (lhs.v7 * rhs.v2) + (lhs.v8 * rhs.v5) + (lhs.v9 * rhs.v8);
			result.v9 = (lhs.v7 * rhs.v3) + (lhs.v8 * rhs.v6) + (lhs.v9 * rhs.v9);

			return result;

		}

		// ----------------------- Member functions ----------------------

		//eMtx3x3Translate stores the translate vector about a 2D plane
		template <typename Type>
			requires std::is_arithmetic_v(Type)
		void eMtx3x3Translate(eMtx3x3& result, Type x, Type y)
		{
			//translate vector is stored inside the last column
			result.v1 = 1.0f;	result.v2 = 0.f;	result.v3 = x;
			result.v4 = 0.f;	result.v5 = 1.f;	result.v6 = y;
			result.v7 = 0.f;	result.v8 = 0.f;	result.v9 = 1.f; 

		}

		template <typename Type>
		void eMtx3x3Scale(Matrix3x3 result, Type x, Type y)
		{
			//scale is stored in the x and y identity
			result.v1 =	x;		result.v2 = 0.f;	result.v3 = 0.f;
			result.v4 = 0.f;	result.v5 =	y;		result.v6 = 0.f;
			result.v7 = 0.f;	result.v8 = 0.f;	result.v9 = 1.f;
		}

		template <typename Type>
		void eMtx3x3RotRad(eMtx3x3& result, Type angle)
		{

			/*rotation matrix = cosa -sina  0
								sina  cosa  0
								0	 0	1	*/

			result.v1 = cosf(angle);	result.v2 = -(sinf(angle));   result.v3 = 0.f;
			result.v4 = sinf(angle);	result.v5 = cosf(angle);	  result.v6 = 0.f;
			result.v7 = 0.f;			result.v8 = 0.f;			  result.v9 = 1.f;	

		}

		template <typename Type>
		void eMtx3x3RotDeg(eMtx3x3 &result, Type angle)
		{

			//convert the degree into rad
			float radAngle = angle * (PI / 180.f);

			/*rotation matrix = cosa -sina  0
								sina  cosa  0
								0	 0	1	*/

			result.v1 = cosf(radAngle);	result.v2 = -(sinf(radAngle)); result.v3 = 0.f;
			result.v4 = sinf(radAngle);	result.v5 =	 cosf(radAngle);   result.v6 = 0.f;
			result.v7 =			 0.f;	result.v8 =				0.f;   result.v9 = 1.f;
		
		}

		void eMtx3x3Transpose(eMtx3x3& result, const eMtx3x3& mtx)
		{

			//transpose matrix swaps the rows and cols

			result.v1 = pMtx.v1;  result.v2 = pMtx.v4;  result.v3 = pMtx.v7;
			result.v4 = pMtx.v2;  result.v5 = pMtx.v5;  result.v6 = pMtx.v8;
			result.v7 = pMtx.v3;  result.v8 = pMtx.v6;  result.v9 = pMtx.v9;

		}

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