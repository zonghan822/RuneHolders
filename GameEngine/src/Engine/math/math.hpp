#pragma once

#include <utility>
#include <concepts>
#include <cmath>
#include <stdexcept>
#include <type_traits>


constexpr float PI = 3.14159265358979323846f;

namespace Math
{
	template <typename Type>
	concept Arithmetic = std::is_arithmetic_v<Type>;

	template <Arithmetic Type>
	struct eVec2
	{
		Type v1, v2;

		// ------------------------------- Addition -------------------------------
		eVec2& operator+=(const eVec2& other);
		eVec2& operator+=(const Type& other);
		eVec2 operator+(const eVec2& other) const;
		eVec2 operator+(const Type& other) const;

		// ------------------------------ Subtraction ------------------------------
		eVec2& operator-=(const eVec2& other);
		eVec2& operator-=(const Type& other);
		eVec2 operator-(const eVec2& other) const;
		eVec2 operator-(const Type& other) const;

		// ----------------------------- Multiplication -----------------------------
		eVec2& operator*=(const eVec2& other);
		eVec2& operator*=(const Type& other);
		eVec2 operator*(const eVec2& other) const;
		eVec2 operator*(const Type& other) const;

		// ------------------------------ Division ------------------------------
		eVec2& operator/=(const eVec2& other);
		eVec2& operator/=(const Type& other);
		eVec2 operator/(const eVec2& other) const;
		eVec2 operator/(Type val) const;

		// ------------------------------ Unary ------------------------------
		// this negates the vector
		eVec2 operator-() const;

	};

	// --------------------- Friend functions for eVec2 ---------------------------

	//eVec2Normalize takes a const &eVec2 vec to normalize and puts the unit vector into result
	template <Arithmetic Type>
	void eVec2Normalize(eVec2<Type>& result, const eVec2<Type>& vec);

	//eVec2Length returns the length of the vector
	template <Arithmetic Type>
	Type eVec2Length(const eVec2<Type>& vec);

	//eVec2SquaredLength returns the squared length of the vector
	template <Arithmetic Type>
	Type eVec2SquaredLength(const eVec2<Type>& vec);

	//eVec2Distance returns the distance between two vectors
	//They are considered as the distance between any 2 points along the vector
	template <Arithmetic Type>
	Type eVec2Distance(const eVec2<Type>& vec1, const eVec2<Type>& vec2);

	//eVec2SquaredDistance returns the squared distance between two vectors
	template <Arithmetic Type>
	Type eVec2SquaredDistance(const eVec2<Type>& vec1, const eVec2<Type>& vec2);

	//eVec2DotProduct returns the dot product between 2 vectors
	template<Arithmetic Type>
	Type eVec2DotProduct(const eVec2<Type>& vec1, const eVec2<Type>& vec2);


	// -------------------------------- 3x3 MATRICES ------------------------------


	template <Arithmetic Type>
	struct eVec3
	{
		Type v1, v2, v3;
		
		// ------------------------------- Addition -------------------------------
		eVec3& operator+=(const eVec3& other);
		eVec3& operator+=(const Type& other);
		eVec3 operator+(const eVec3& other) const;
		eVec3 operator+(const Type& other) const;

		// ------------------------------ Subtraction ------------------------------
		eVec3& operator-=(const eVec3& other);
		eVec3& operator-=(const Type& other);
		eVec3 operator-(const eVec3& other) const;
		eVec3 operator-(const Type& other) const;

		// ----------------------------- Multiplication -----------------------------
		eVec3& operator*=(const eVec3& other);
		eVec3& operator*=(const Type& other);
		eVec3 operator*(const eVec3& other) const;
		eVec3 operator*(const Type& other) const;

		// ------------------------------ Division ------------------------------
		eVec3& operator/=(const eVec3& other);
		eVec3& operator/=(const Type& other);
		eVec3 operator/(const eVec3& other) const;
		eVec3 operator/(const Type& other) const;

		// ------------------------------ Unary ------------------------------
		eVec3 operator-() const;
	};

#if 0
	template <Arithmetic Type>
	struct eMtx2x2
	{
		Type v1, v2, 
			 v3, v4;


	};
#endif
	// -------------------------------- 3x3 MATRICES ------------------------------
	template <Arithmetic Type>
	struct eMtx3x3
	{
		Type v1, v2, v3, 
			 v4, v5, v6, 
			 v7, v8, v9;

		// -------------------- Constructors ---------------------

		eMtx3x3
		(Type _00, Type _01, Type _02,
		 Type _10, Type _11, Type _12,
		 Type _20, Type _21, Type _22);

		//copy constructor
		eMtx3x3(const eMtx3x3& other);

		//Assignment operator
		eMtx3x3& operator=(const eMtx3x3& other);
		// ------------------------ Multiplication ----------------------------
		eMtx3x3& operator*=(const eMtx3x3& rhs);


		// ----------------------- Member functions ----------------------

		//eMtx3x3Translate stores the translate vector about a 2D plane
		void eMtx3x3Translate(eMtx3x3<Type>& result, Type x, Type y);
		void eMtx3x3Scale(eMtx3x3<Type> result, Type x, Type y);
		void eMtx3x3RotRad(eMtx3x3<Type>& result, Type angle);
		void eMtx3x3RotDeg(eMtx3x3<Type>& result, Type angle);
		void eMtx3x3Transpose(eMtx3x3<Type>& result, const eMtx3x3<Type>& mtx);

	};

	template <Arithmetic Type>
	struct eMtx4x4
	{
		Type v1, v2, v3, v4, 
			 v5, v6, v7, v8, 
			 v9, v10, v11, v12;


	};

}

#include "math.tpp"