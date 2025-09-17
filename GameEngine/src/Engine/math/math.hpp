#pragma once

#include <utility>
#include <concepts>
#include <cmath>
#include <stdexcept>

constexpr float PI = 3.14159265358979323846f;

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

		eVec2 operator/(Type val) const
		{
			if(val == 0) 
			{
				throw std::runtime_error("Division by zero on a vector!");
			}

			return {v1 / val, v2 / val};

		}

		// ------------------------------ Unary ------------------------------
		// this negates the vector
		eVec2 operator-() const
		{
			return {-v1, -v2};
		}

	};

	// --------------------- Friend functions for eVec2 ---------------------------

	//eVec2Normalize takes a const &eVec2 vec to normalize and puts the unit vector into result
	template <typename Type>
	void eVec2Normalize(eVec2<Type> &result, const eVec2<Type>& vec)
	{
		//normalized outward normal of a vector = n/||n||
		result = vec / eVec2Length(vec);

	}

	//eVec2Length returns the length of the vector
	template <typename Type>
	Type eVec2Length(const eVec2<Type>& vec)
	{
		//length = sqrt(x^2 + y^2)
		return std::sqrt(eVec2SquaredLength(vec));

	}

	//eVec2SquaredLength returns the squared length of the vector
	template <typename Type>
	Type eVec2SquaredLength(const eVec2<Type>& vec)
	{
		//sq length = (x^2 + y^2)
		return ((vec.v1 * vec.v1) + (vec.v2 * vec.v2));

	}


	//eVec2Distance returns the distance between two vectors
	//They are considered as the distance between any 2 points along the vector
	template <typename Type>
	Type eVec2Distance(const eVec2<Type>& vec1, const eVec2<Type>& vec2)
	{
		//distance = sqrt((x1 - x2)^2, (y1-y2)^2)
		return std::sqrt(eVec2SquaredDistance(vec1, vec2));

	}

	//eVec2SquaredDistance returns the squared distance between two vectors
	template <typename Type>
	Type eVec2SquaredDistance(const eVec2<Type>& vec1, const eVec2<Type>& vec2)
	{
		//sq distance = ((x1 - x2)^2, (y1-y2)^2)
		return (((vec1.v1 - vec2.v1) * (vec1.v1 - vec2.v1)) + ((vec1.v2 - vec2.v2) * (vec1.v2 - vec2.v2)));

	}

	//eVec2DotProduct returns the dot product between 2 vectors
	template<typename Type>
	Type eVec2DotProduct(const eVec2<Type>& vec1, const eVec2<Type>& vec2)
	{
		//Dot product = x1 * x2 + y1*y2
		return (vec1.v1*vec2.v1) + (vec1.v2*vec2.v2);
	}


	// -------------------------------- 3x3 MATRICES ------------------------------


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

		//template <typename Type>
			//requires std::is_arithmetic_v<Type>
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

		//template <typename Type>
			//requires std::is_arithmetic_v<Type>
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

		//template <typename Type>
			//requires std::is_arithmetic_v<Type>
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

		//template <typename Type>
			//requires std::is_arithmetic_v<Type>
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

		// -------------------- Constructors ---------------------
		//template <typename Type>
			//requires std::is_arithmetic_v<Type>
		eMtx3x3	
		(Type _00, Type _01, Type _02,
		 Type _10, Type _11, Type _12,
		 Type _20, Type _21, Type _22) :

		 
		 v1(_00),	v2(_01),	v3(_02),
		 v4(_10),	v5(_11),	v6(_12),
		 v7(_20),	v8(_21),	v9(_22) {}

		//copy constructor
		eMtx3x3(const eMtx3x3& other) = default;

		//Assignment operator
		//template <typename T>
			//requires std::is_arithmetic_v<Type>
		eMtx3x3& operator=(const eMtx3x3& other) = default;
		// ------------------------ Multiplication ----------------------------
		eMtx3x3& operator*=(const eMtx3x3& rhs)
		{
			//3x3 C, Cij = Ai1*B1j + Ai2*B2j + Ai3*B3j OR {row x column elems} 
			eMtx3x3 other (
				(v1 * rhs.v1) + (v2 * rhs.v4) + (v3 * rhs.v7),
				(v1 * rhs.v2) + (v2 * rhs.v5) + (v3 * rhs.v8),
				(v1 * rhs.v3) + (v2 * rhs.v6) + (v3 * rhs.v9),

				(v4 * rhs.v1) + (v5 * rhs.v4) + (v6 * rhs.v7),
				(v4 * rhs.v2) + (v5 * rhs.v5) + (v6 * rhs.v8),
				(v4 * rhs.v3) + (v5 * rhs.v6) + (v6 * rhs.v9),

				(v7 * rhs.v1) + (v8 * rhs.v4) + (v9 * rhs.v7),
				(v7 * rhs.v2) + (v8 * rhs.v5) + (v9 * rhs.v8),
				(v7 * rhs.v3) + (v8 * rhs.v6) + (v9 * rhs.v9)
   	 		);

			// Assign the computed matrix back to this
			*this = other;

			return *this;

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
		void eMtx3x3Scale(eMtx3x3<Type> result, Type x, Type y)
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

			result.v1 = std::cos(angle);	result.v2 = -(std::sin(angle));   result.v3 = 0.f;
			result.v4 = std::sin(angle);	result.v5 = std::cos(angle);	  result.v6 = 0.f;
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

			result.v1 = std::cos(radAngle);	result.v2 = -(std::sin(radAngle)); result.v3 = 0.f;
			result.v4 = std::sin(radAngle);	result.v5 =	   std::cos(radAngle); result.v6 = 0.f;
			result.v7 =			       0.f;	result.v8 =				      0.f; result.v9 = 1.f;
		
		}

		void eMtx3x3Transpose(eMtx3x3& result, const eMtx3x3& mtx)
		{

			//transpose matrix swaps the rows and cols

			result.v1 = mtx.v1;  result.v2 = mtx.v4;  result.v3 = mtx.v7;
			result.v4 = mtx.v2;  result.v5 = mtx.v5;  result.v6 = mtx.v8;
			result.v7 = mtx.v3;  result.v8 = mtx.v6;  result.v9 = mtx.v9;

		}

	};

	template<typename Type>
	eMtx3x3 operator*(const eMtx3x3<Type>& lhs, const eMtx3x3<Type>& rhs) {
		return eMtx3x3<Type>(
			(lhs.v1 * rhs.v1) + (lhs.v2 * rhs.v4) + (lhs.v3 * rhs.v7),
			(lhs.v1 * rhs.v2) + (lhs.v2 * rhs.v5) + (lhs.v3 * rhs.v8),
			(lhs.v1 * rhs.v3) + (lhs.v2 * rhs.v6) + (lhs.v3 * rhs.v9),

			(lhs.v4 * rhs.v1) + (lhs.v5 * rhs.v4) + (lhs.v6 * rhs.v7),
			(lhs.v4 * rhs.v2) + (lhs.v5 * rhs.v5) + (lhs.v6 * rhs.v8),
			(lhs.v4 * rhs.v3) + (lhs.v5 * rhs.v6) + (lhs.v6 * rhs.v9),

			(lhs.v7 * rhs.v1) + (lhs.v8 * rhs.v4) + (lhs.v9 * rhs.v7),
			(lhs.v7 * rhs.v2) + (lhs.v8 * rhs.v5) + (lhs.v9 * rhs.v8),
			(lhs.v7 * rhs.v3) + (lhs.v8 * rhs.v6) + (lhs.v9 * rhs.v9)
		);
	}

	template <typename Type>
		requires std::is_arithmetic_v<Type>
	struct eMtx4x4
	{
		Type v1, v2, v3, v4, 
			 v5, v6, v7, v8, 
			 v9, v10, v11, v12;


	};
}