/*============================================================================================
* @file         math.tpp
* @author       Caven Koh (caven.koh@digipen.edu)
* @date         15-09-2025
* @brief        This file contains the definitions for the math library math.hpp
*
* * All content © 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
*//*========================================================================================*/
#pragma once


namespace Math
{

	// ----------------------------- 2D Vectors -------------------------------
	// ------------------------------- Addition -------------------------------

	template <Arithmetic Type>
	eVec2<Type>& eVec2<Type>::operator+=(const eVec2<Type>& other)
	{
		this->x += other.x;
		this->y += other.y;

		return *this;
	}

	template <Arithmetic Type>
	eVec2<Type>& eVec2<Type>::operator+=(const Type& other)
	{
		this->x += other;
		this->y += other;

		return *this;
	}

	template <Arithmetic Type>
	eVec2<Type> eVec2<Type>::operator+(const eVec2<Type>& other)
	{
		return { x + other.x, y + other.y };
	}
	template <Arithmetic Type>
	eVec2<Type> eVec2<Type>::operator+(const Type& other)
	{
		return {x + other, y + other};
	}

	// ------------------------------ Subtraction ------------------------------
	template <Arithmetic Type>
		
	eVec2<Type>& eVec2<Type>::operator-=(const eVec2<Type>& other)
	{
		this->x -= other.x;
		this->y -= other.y;

		return *this;
	}

	template <Arithmetic Type>
		
	eVec2<Type>& eVec2<Type>::operator-=(const Type& other)
	{
		this->x -= other;
		this->y -= other;

		return *this;
	}

	template <Arithmetic Type>
	eVec2<Type> eVec2<Type>::operator-(const eVec2<Type>& other)
	{
		return { x - other.x, y - other.y };
	}

	// ----------------------------- Multiplication -----------------------------
	template <Arithmetic Type>
	eVec2<Type>& eVec2<Type>::operator*=(const eVec2<Type>& other)
	{
		this->x *= other.x;
		this->y *= other.y;

		return *this;
	}

	template <Arithmetic Type>
	eVec2<Type>& eVec2<Type>::operator*=(const Type& other)
	{
		this->x *= other;
		this->y *= other;

		return *this;
	}

	template <Arithmetic Type>
	eVec2<Type> eVec2<Type>::operator*(const eVec2<Type>& other)
	{
		return { x * other.x, y * other.y };
	}

	template <Arithmetic Type>
	eVec2<Type> eVec2<Type>::operator*(Type other) const
	{
		return {x * other, y*other};
	
	}

	

	// ------------------------------ Division ------------------------------
	template <Arithmetic Type>
	eVec2<Type>& eVec2<Type>::operator/=(const eVec2<Type>& other)
	{
		if (other.x == 0 ||
			other.y == 0)
		{
			throw std::runtime_error("Division by zero on a vector!");
		}

		this->x /= other.x;
		this->y /= other.y;

		return *this;
	}

	template <Arithmetic Type>
	eVec2<Type>& eVec2<Type>::operator/=(const Type& other)
	{
		if (other == 0)
		{
			throw std::runtime_error("Division by zero on a vector!");
		}

		this->x /= other;
		this->y /= other;

		return *this;
	}

	template <Arithmetic Type>
	eVec2<Type> eVec2<Type>::operator/(const eVec2<Type>& other)
	{
		if (other.x == 0 ||
			other.y == 0)
		{
			throw std::runtime_error("Division by zero on a vector!");
		}

		return { x / other.x, y / other.y };
	}

	template <Arithmetic Type>
	eVec2<Type> eVec2<Type>::operator/(Type val) const
	{
		if (val == 0)
		{
			throw std::runtime_error("Division by zero on a vector!");
		}

		return { x / val, y / val };

	}

	// ------------------------------ Unary ------------------------------
	// this negates the vector
	template <Arithmetic Type>
	eVec2<Type> eVec2<Type>::operator-() const
	{
		return { -x, -y };
	}



	// --------------------- Friend functions for eVec2 ---------------------------
	template <Arithmetic Type>
	eVec2<Type> operator*(const Type& lhs, const eVec2<Type>& rhs)
	{
		return {lhs * rhs.x, lhs * rhs.y};
	}

	template <Arithmetic Type>
	eVec2<Type> operator+(const eVec2<Type>& lhs, const eVec2<Type>&rhs)
	{
		eVec2<Type> tmp {lhs};
		tmp += rhs;
		return tmp;
	}

	template <Arithmetic Type>
	eVec2<Type> operator-(const eVec2<Type>& lhs, const eVec2<Type>&rhs)
	{
		eVec2<Type> tmp {lhs};
		tmp -= rhs;
		return tmp;
	}

	//eVec2Normalize takes a const &eVec2 vec to normalize and puts the unit vector into result
	template <Arithmetic Type>
	void eVec2Normalize(eVec2<Type>& result, const eVec2<Type>& vec)
	{
		//normalized outward normal of a vector = n/||n||
		Type length = eVec2Length(vec);
		if(length < 0.000001f)
		{
			result = {0.0f, 0.0f};
		}
		else
		{
			result = vec / eVec2Length(vec);
		}
		

	}

	//eVec2Length returns the length of the vector
	template <Arithmetic Type>
	Type eVec2Length(const eVec2<Type>& vec)
	{
		//length = sqrt(x^2 + y^2)
		 Type val = std::sqrt(eVec2SquaredLength(vec));
		 return val;
	}

	//eVec2SquaredLength returns the squared length of the vector
	template <Arithmetic Type>
	Type eVec2SquaredLength(const eVec2<Type>& vec)
	{
		//sq length = (x^2 + y^2)
		return ((vec.x * vec.x) + (vec.y * vec.y));

	}


	//eVec2Distance returns the distance between two vectors
	//They are considered as the distance between any 2 points along the vector
	template <Arithmetic Type>
	Type eVec2Distance(const eVec2<Type>& vec1, const eVec2<Type>& vec2)
	{
		//distance = sqrt((x1 - x2)^2, (y1-y2)^2)
		return std::sqrt(eVec2<Type>::eVec2SquaredDistance(vec1, vec2));

	}

	//eVec2SquaredDistance returns the squared distance between two vectors
	template <Arithmetic Type>
	Type eVec2SquaredDistance(const eVec2<Type>& vec1, const eVec2<Type>& vec2)
	{
		//sq distance = ((x1 - x2)^2, (y1-y2)^2)
		return (((vec1.x - vec2.x) * (vec1.x - vec2.x)) + ((vec1.y - vec2.y) * (vec1.y - vec2.y)));

	}

	//eVec2DotProduct returns the dot product between 2 vectors
	template<typename Type>
	Type eVec2DotProduct(const eVec2<Type>& vec1, const eVec2<Type>& vec2)
	{
		//Dot product = x1 * x2 + y1*y2
		return (vec1.x * vec2.x) + (vec1.y * vec2.y);
	}

	template<typename Type>
	std::ostream& operator<<(std::ostream& os, const eVec2<Type>& vec1)
	{
		return os << "x : " << vec1.x << " | y : " << vec1.y;
	}

	template <Arithmetic Type>
    bool operator==(const eVec2<Type>& lhs, const eVec2<Type>& rhs)
    {
        if constexpr (std::is_floating_point_v<Type>)
        {
            constexpr Type EPSILON = static_cast<Type>(1e-6);
            return std::fabs(lhs.x - rhs.x) < EPSILON &&
                   std::fabs(lhs.y - rhs.y) < EPSILON;
        }
        else
        {
            return lhs.x == rhs.x && lhs.y == rhs.y;
        }
    }

    template <Arithmetic Type>
    bool operator!=(const eVec2<Type>& lhs, const eVec2<Type>& rhs)
    {
        return !(lhs == rhs);
    }

	// -------------------------------- 3x3 MATRICES ------------------------------

	// ------------------------------- Addition -------------------------------
	template <Arithmetic Type>
	eVec3<Type>& eVec3<Type>::operator+=(const eVec3<Type>& other)
	{
		this->x += other.x;
		this->y += other.y;
		this->z += other.z;

		return *this;
	}

	template <Arithmetic Type>
	eVec3<Type>& eVec3<Type>::operator+=(const Type& other)
	{
		this->x += other;
		this->y += other;
		this->z += other;

		return *this;
	}

	template <Arithmetic Type>
	eVec3<Type> eVec3<Type>::operator+(const eVec3<Type>& other)
	{
		return { x + other.x, y + other.y, z + other.z };
	}

	// ------------------------------ Subtraction ------------------------------
	template <Arithmetic Type>
	eVec3<Type>& eVec3<Type>::operator-=(const eVec3<Type>& other)
	{
		this->x -= other.x;
		this->y -= other.y;
		this->z -= other.z;

		return *this;
	}

	template <Arithmetic Type>
	eVec3<Type>& eVec3<Type>::operator-=(const Type& other)
	{
		this->x -= other;
		this->y -= other;
		this->z -= other;

		return *this;
	}

	template <Arithmetic Type>
	eVec3<Type> eVec3<Type>::operator-(const eVec3<Type>& other)
	{
		return { x - other.x, y - other.y, z - other.z };
	}

	// ----------------------------- Multiplication -----------------------------
	template <Arithmetic Type>
	eVec3<Type>& eVec3<Type>::operator*=(const eVec3<Type>& other)
	{
		this->x *= other.x;
		this->y *= other.y;
		this->z *= other.z;

		return *this;
	}

	template <Arithmetic Type>
	eVec3<Type>& eVec3<Type>::operator*=(const Type& other)
	{
		this->x *= other;
		this->y *= other;
		this->z *= other;

		return *this;
	}

	template <Arithmetic Type>
	eVec3<Type> eVec3<Type>::operator*(const eVec3<Type>& other)
	{
		return { x * other.x, y * other.y, z * other.z };
	}

	// ------------------------------ Division ------------------------------
	template <Arithmetic Type>
	eVec3<Type>& eVec3<Type>::operator/=(const eVec3<Type>& other)
	{
		if (other.x == 0 ||
			other.y == 0 ||
			other.z == 0)
		{
			throw std::runtime_error("Division by zero on a vector!");
		}

		this->x /= other.x;
		this->y /= other.y;
		this->z /= other.z;

		return *this;
	}

	template <Arithmetic Type>
	eVec3<Type>& eVec3<Type>::operator/=(const Type& other)
	{
		if (other == 0)
		{
			throw std::runtime_error("Division by zero on a vector!");
		}

		this->x /= other;
		this->y /= other;
		this->z /= other;

		return *this;
	}

	template <Arithmetic Type>
	eVec3<Type> eVec3<Type>::operator/(const eVec3<Type>& other)
	{
		if (other.x == 0 ||
			other.y == 0 ||
			other.z == 0)
		{
			throw std::runtime_error("Division by zero on a vector!");
		}

		return { x / other.x, y / other.y, z / other.z };
	}
	
	// -------------------- 3 x 3 Matrix -----------------------

	// -------------------- Constructors ---------------------
	template <Arithmetic Type>
	eMtx3x3<Type>::eMtx3x3 
		(Type _00, Type _01, Type _02,
		 Type _10, Type _11, Type _12,
		 Type _20, Type _21, Type _22) :

		 v1(_00), v2(_01), v3(_02),
		 v4(_10), v5(_11), v6(_12),
		 v7(_20), v8(_21), v9(_22) {}

	//copy constructor
	template <Arithmetic Type>
	eMtx3x3<Type>::eMtx3x3(const eMtx3x3<Type>& other) = default;

	//Assignment operator
	template <Arithmetic Type>
	eMtx3x3<Type>& eMtx3x3<Type>::operator=(const eMtx3x3<Type>& other) = default;

	// ------------------------ Multiplication ----------------------------
	template <Arithmetic Type>
	eMtx3x3<Type>& eMtx3x3<Type>::operator*=(const eMtx3x3<Type>& rhs)
	{
		//3x3 C, Cij = Ai1*B1j + Ai2*B2j + Ai3*B3j OR {row x column elems} 
		eMtx3x3<Type> other(
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
	template <Arithmetic Type> 
	void eMtx3x3<Type>::eMtx3x3Translate(eMtx3x3<Type>& result, Type x, Type y)
	{
		//translate vector is stored inside the last column
		result.v1 = 1.0f;	result.v2 = 0.f;	result.v3 = x;
		result.v4 = 0.f;	result.v5 = 1.f;	result.v6 = y;
		result.v7 = 0.f;	result.v8 = 0.f;	result.v9 = 1.f;

	}

	template <Arithmetic Type>
	void eMtx3x3<Type>::eMtx3x3Scale(eMtx3x3<Type> result, Type x, Type y)
	{
		//scale is stored in the x and y identity
		result.v1 = x;		result.v2 = 0.f;	result.v3 = 0.f;
		result.v4 = 0.f;	result.v5 = y;		result.v6 = 0.f;
		result.v7 = 0.f;	result.v8 = 0.f;	result.v9 = 1.f;
	}

	template <Arithmetic Type>
	void eMtx3x3<Type>::eMtx3x3RotRad(eMtx3x3<Type>& result, Type angle)
	{

		/*rotation matrix = cosa -sina  0
							sina  cosa  0
							0	 0	1	*/

		result.v1 = std::cos(angle);	result.v2 = -(std::sin(angle));   result.v3 = 0.f;
		result.v4 = std::sin(angle);	result.v5 = std::cos(angle);	  result.v6 = 0.f;
		result.v7 = 0.f;				result.v8 = 0.f;				  result.v9 = 1.f;

	}

	template <Arithmetic Type>
	void eMtx3x3<Type>::eMtx3x3RotDeg(eMtx3x3<Type>& result, Type angle)
	{

		//convert the degree into rad
		float radAngle = angle * (PI / 180.f);

		/*rotation matrix = cosa -sina  0
							sina  cosa  0
							0	 0	1	*/

		result.v1 = std::cos(radAngle);	result.v2 = -(std::sin(radAngle)); result.v3 = 0.f;
		result.v4 = std::sin(radAngle);	result.v5 = std::cos(radAngle); result.v6 = 0.f;
		result.v7 = 0.f;	result.v8 = 0.f; result.v9 = 1.f;

	}

	template <Arithmetic Type>
	void eMtx3x3<Type>::eMtx3x3Transpose(eMtx3x3<Type>& result, const eMtx3x3<Type>& mtx)
	{

		//transpose matrix swaps the rows and cols
		result.v1 = mtx.v1;  result.v2 = mtx.v4;  result.v3 = mtx.v7;
		result.v4 = mtx.v2;  result.v5 = mtx.v5;  result.v6 = mtx.v8;
		result.v7 = mtx.v3;  result.v8 = mtx.v6;  result.v9 = mtx.v9;

	}


	// ------------------------------- MISCELLANEOUS ----------------------------------

	template <typename T>
	T max(const T& a, const T& b)
	{
		return a > b ? a : b;
	}

	template <typename T>
	T min(const T& a, const T& b)
	{
		return a < b ? a : b;
	}


}
