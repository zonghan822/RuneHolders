/*============================================================================================
* @file         math.hpp
* @author       Caven Koh (caven.koh@digipen.edu)
* @date         15-09-2025
* @brief        This file contains struct templates for Vector2D, Vector3D, Mtx3x3, and Mtx4x4
*				as well as operator overloads and helper functions which assists in arithmetic
*				calculations
* 
* * All content � 2025 DigiPen (SINGAPORE) Corporation, all rights reserved.
*//*========================================================================================*/
#pragma once

#include <utility>
#include <concepts>
#include <cmath>
#include <stdexcept>
#include <type_traits>
#include <iostream>


constexpr float PI = 3.14159265358979323846f;

namespace Math
{
	template <typename Type>
	concept Arithmetic = std::is_arithmetic_v<Type>;

	template <Arithmetic T>
	constexpr T maxValue(const T& a, const T& b);

	template <Arithmetic T>
	constexpr T minValue(const T& a, const T& b);

	template <Arithmetic Type>
	struct eVec2
	{
		Type x, // X coords
			 y; // Y coords

		/*============================================================================================
		 * \brief       Adds another vector to this vector (component-wise).
		 * \param[in]   other  Vector to add.
		 * \return      Reference to this vector after addition.
		*//*========================================================================================*/
		eVec2& operator+=(const eVec2& other);

		/*============================================================================================
		 * \brief       Adds a scalar value to both components of this vector.
		 * \param[in]   other  Scalar value to add.
		 * \return      Reference to this vector after addition.
		*//*========================================================================================*/
		eVec2& operator+=(const Type& other);

		/*============================================================================================
		 * \brief       Returns a new vector equal to this vector plus another (component-wise).
		 * \param[in]   other  Vector to add.
		 * \return      New eVec2 containing the sum.
		*//*========================================================================================*/
		eVec2 operator+(const eVec2& other);

		/*============================================================================================
		 * \brief       Returns a new vector equal to this vector plus a scalar.
		 * \param[in]   other  Scalar value to add.
		 * \return      New eVec2 containing the sum.
		*//*========================================================================================*/
		eVec2 operator+(const Type& other);

		/*============================================================================================
		 * \brief       Subtracts another vector from this vector (component-wise).
		 * \param[in]   other  Vector to subtract.
		 * \return      Reference to this vector after subtraction.
		*//*========================================================================================*/
		eVec2& operator-=(const eVec2& other);

		/*============================================================================================
		 * \brief       Subtracts a scalar from both components of this vector.
		 * \param[in]   other  Scalar value to subtract.
		 * \return      Reference to this vector after subtraction.
		*//*========================================================================================*/
		eVec2& operator-=(const Type& other);

		/*============================================================================================
		 * \brief       Returns a new vector equal to this vector minus another (component-wise).
		 * \param[in]   other  Vector to subtract.
		 * \return      New eVec2 containing the difference.
		*//*========================================================================================*/
		eVec2 operator-(const eVec2& other);

		/*============================================================================================
		 * \brief       Multiplies this vector by another vector (component-wise).
		 * \param[in]   other  Vector to multiply by.
		 * \return      Reference to this vector after multiplication.
		*//*========================================================================================*/
		eVec2& operator*=(const eVec2& other);

		/*============================================================================================
		 * \brief       Multiplies both components of this vector by a scalar.
		 * \param[in]   other  Scalar multiplier.
		 * \return      Reference to this vector after multiplication.
		*//*========================================================================================*/
		eVec2& operator*=(const Type& other);

		/*============================================================================================
		 * \brief       Returns a new vector equal to this vector multiplied by another (component-wise).
		 * \param[in]   other  Vector to multiply by.
		 * \return      New eVec2 containing the product.
		*//*========================================================================================*/
		eVec2 operator*(const eVec2& other);

		/*============================================================================================
		 * \brief       Returns a new vector equal to this vector scaled by a scalar value.
		 * \param[in]   val  Scalar multiplier.
		 * \return      New eVec2 containing the scaled result.
		*//*========================================================================================*/
		eVec2 operator*(Type val) const;

		/*============================================================================================
		 * \brief       Divides this vector by another vector (component-wise).
		 * \param[in]   other  Vector to divide by.
		 * \return      Reference to this vector after division.
		*//*========================================================================================*/
		eVec2& operator/=(const eVec2& other);

		/*============================================================================================
		 * \brief       Divides both components of this vector by a scalar.
		 * \param[in]   other  Scalar divisor.
		 * \return      Reference to this vector after division.
		*//*========================================================================================*/
		eVec2& operator/=(const Type& other);

		/*============================================================================================
		 * \brief       Returns a new vector equal to this vector divided by another (component-wise).
		 * \param[in]   other  Vector to divide by.
		 * \return      New eVec2 containing the quotient.
		*//*========================================================================================*/
		eVec2 operator/(const eVec2& other);

		/*============================================================================================
		 * \brief       Returns a new vector equal to this vector divided by a scalar.
		 * \param[in]   val  Scalar divisor.
		 * \return      New eVec2 containing the quotient.
		*//*========================================================================================*/
		eVec2 operator/(Type val) const;

		/*============================================================================================
		 * \brief       Returns the negation of this vector (flips the sign of each component).
		 * \return      New eVec2 containing the negated vector.
		*//*========================================================================================*/
		eVec2 operator-() const;
	};

	// --------------------- Friend functions for eVec2 ---------------------------
	/*============================================================================================
	 * \brief       Multiplies a scalar by a vector (scalar * vector).
	 * \param[in]   lhs  Scalar multiplier.
	 * \param[in]   rhs  Vector to multiply.
	 * \return      New eVec2 containing the scaled vector.
	*//*========================================================================================*/
	template <Arithmetic Type>
	eVec2<Type> operator*(const Type& lhs, const eVec2<Type>& rhs);

	/*============================================================================================
	 * \brief       Adds two vectors (component-wise).
	 * \param[in]   lhs  Left-hand vector.
	 * \param[in]   rhs  Right-hand vector.
	 * \return      New eVec2 containing the sum.
	*//*========================================================================================*/
	template <Arithmetic Type>
	eVec2<Type> operator+(const eVec2<Type>& lhs, const eVec2<Type>& rhs);

	/*============================================================================================
	 * \brief       Subtracts one vector from another (component-wise).
	 * \param[in]   lhs  Left-hand vector.
	 * \param[in]   rhs  Right-hand vector.
	 * \return      New eVec2 containing the difference.
	*//*========================================================================================*/
	template <Arithmetic Type>
	eVec2<Type> operator-(const eVec2<Type>& lhs, const eVec2<Type>& rhs);

	/*============================================================================================
	 * \brief       Normalizes a vector (scales it to unit length).
	 * \param[out]  result  Normalized unit vector result.
	 * \param[in]   vec     Input vector to normalize.
	*//*========================================================================================*/
	template <Arithmetic Type>
	void eVec2Normalize(eVec2<Type>& result, const eVec2<Type>& vec);

	/*============================================================================================
	 * \brief       Computes the length (magnitude) of a vector.
	 * \param[in]   vec  Input vector.
	 * \return      Length of the vector.
	*//*========================================================================================*/
	template <typename Type>
	Type eVec2Length(const eVec2<Type>& vec);

	/*============================================================================================
	 * \brief       Computes the squared length of a vector.
	 * \param[in]   vec  Input vector.
	 * \return      Squared length of the vector.
	*//*========================================================================================*/
	template <Arithmetic Type>
	Type eVec2SquaredLength(const eVec2<Type>& vec);

	/*============================================================================================
	 * \brief       Computes the distance between two vectors (treated as points).
	 * \param[in]   vec1  First vector.
	 * \param[in]   vec2  Second vector.
	 * \return      Distance between vec1 and vec2.
	*//*========================================================================================*/
	template <Arithmetic Type>
	Type eVec2Distance(const eVec2<Type>& vec1, const eVec2<Type>& vec2);

	/*============================================================================================
	 * \brief       Computes the squared distance between two vectors (treated as points).
	 * \param[in]   vec1  First vector.
	 * \param[in]   vec2  Second vector.
	 * \return      Squared distance between vec1 and vec2.
	*//*========================================================================================*/
	template <Arithmetic Type>
	Type eVec2SquaredDistance(const eVec2<Type>& vec1, const eVec2<Type>& vec2);

	/*============================================================================================
	 * \brief       Computes the dot product between two vectors.
	 * \param[in]   vec1  First vector.
	 * \param[in]   vec2  Second vector.
	 * \return      Dot product value.
	*//*========================================================================================*/
	template<typename Type>
	Type eVec2DotProduct(const eVec2<Type>& vec1, const eVec2<Type>& vec2);

	/*============================================================================================
	 * \brief       Outputs the vector components to an output stream.
	 * \param[in]   os    Output stream.
	 * \param[in]   vec1  Vector to output.
	 * \return      Reference to the output stream.
	*//*========================================================================================*/
	template<typename Type>
	std::ostream& operator<<(std::ostream& os, const eVec2<Type>& vec1);

	template <Arithmetic Type>
	bool operator==(const eVec2<Type>& lhs, const eVec2<Type>& rhs);

	template <Arithmetic Type>
	bool operator!=(const eVec2<Type>& lhs, const eVec2<Type>& rhs);

	// -------------------------------- 3D VECTORS --------------------------------
	template <Arithmetic Type>
	struct eVec3
	{
		Type x, // X coords
			 y, // Y coords
			 z; // Z coords

		/*============================================================================================
		 * \brief       Adds another vector to this vector (component-wise).
		 * \param[in]   other  Vector to add.
		 * \return      Reference to this vector after addition.
		*//*========================================================================================*/
		eVec3& operator+=(const eVec3& other);

		/*============================================================================================
		 * \brief       Adds a scalar value to all components of this vector.
		 * \param[in]   other  Scalar value to add.
		 * \return      Reference to this vector after addition.
		*//*========================================================================================*/
		eVec3& operator+=(const Type& other);

		/*============================================================================================
		 * \brief       Returns a new vector equal to this vector plus another (component-wise).
		 * \param[in]   other  Vector to add.
		 * \return      New eVec3 containing the sum.
		*//*========================================================================================*/
		eVec3 operator+(const eVec3& other);

		/*============================================================================================
		 * \brief       Subtracts another vector from this vector (component-wise).
		 * \param[in]   other  Vector to subtract.
		 * \return      Reference to this vector after subtraction.
		*//*========================================================================================*/
		eVec3& operator-=(const eVec3& other);

		/*============================================================================================
		 * \brief       Subtracts a scalar from all components of this vector.
		 * \param[in]   other  Scalar value to subtract.
		 * \return      Reference to this vector after subtraction.
		*//*========================================================================================*/
		eVec3& operator-=(const Type& other);

		/*============================================================================================
		 * \brief       Returns a new vector equal to this vector minus another (component-wise).
		 * \param[in]   other  Vector to subtract.
		 * \return      New eVec3 containing the difference.
		*//*========================================================================================*/
		eVec3 operator-(const eVec3& other);

		/*============================================================================================
		 * \brief       Multiplies this vector by another vector (component-wise).
		 * \param[in]   other  Vector to multiply by.
		 * \return      Reference to this vector after multiplication.
		*//*========================================================================================*/
		eVec3& operator*=(const eVec3& other);

		/*============================================================================================
		 * \brief       Multiplies all components of this vector by a scalar.
		 * \param[in]   other  Scalar multiplier.
		 * \return      Reference to this vector after multiplication.
		*//*========================================================================================*/
		eVec3& operator*=(const Type& other);

		/*============================================================================================
		 * \brief       Returns a new vector equal to this vector multiplied by another (component-wise).
		 * \param[in]   other  Vector to multiply by.
		 * \return      New eVec3 containing the product.
		*//*========================================================================================*/
		eVec3 operator*(const eVec3& other);

		/*============================================================================================
		 * \brief       Divides this vector by another vector (component-wise).
		 * \param[in]   other  Vector to divide by.
		 * \return      Reference to this vector after division.
		*//*========================================================================================*/
		eVec3& operator/=(const eVec3& other);

		/*============================================================================================
		 * \brief       Divides all components of this vector by a scalar.
		 * \param[in]   other  Scalar divisor.
		 * \return      Reference to this vector after division.
		*//*========================================================================================*/
		eVec3& operator/=(const Type& other);

		/*============================================================================================
		 * \brief       Returns a new vector equal to this vector divided by another (component-wise).
		 * \param[in]   other  Vector to divide by.
		 * \return      New eVec3 containing the quotient.
		*//*========================================================================================*/
		eVec3 operator/(const eVec3& other);
	};

#if 0
	template <Arithmetic Type>
	struct eMtx2x2
	{
		Type v1, v2,
			v3, v4;


	};
#endif

	// -------------------------------- 3x3 MATRICES --------------------------------
	template <Arithmetic Type>
	struct eMtx3x3
	{
		Type v1, v2, v3, // 00, 01, 02
			v4, v5, v6,	 // 10, 11, 12
			v7, v8, v9;	 // 20, 21, 22

		/*============================================================================================
		 * \brief       Default constructor. Initializes matrix without setting values.
		*//*========================================================================================*/
		eMtx3x3() = default;

		/*============================================================================================
		 * \brief       Constructs a matrix with 9 specified values.
		 * \param[in]   _00.._22  Elements of the 3x3 matrix in row-major order.
		*//*========================================================================================*/
		eMtx3x3(Type _00, Type _01, Type _02,
			Type _10, Type _11, Type _12,
			Type _20, Type _21, Type _22);

		/*============================================================================================
		 * \brief       Copy constructor.
		 * \param[in]   other  Matrix to copy from.
		*//*========================================================================================*/
		eMtx3x3(const eMtx3x3& other);

		/*============================================================================================
		 * \brief       Assignment operator.
		 * \param[in]   other  Matrix to assign from.
		 * \return      Reference to this matrix after assignment.
		*//*========================================================================================*/
		eMtx3x3& operator=(const eMtx3x3& other);

		/*============================================================================================
		 * \brief       Multiplies this matrix by another matrix (matrix * matrix).
		 * \param[in]   rhs  Right-hand side matrix.
		 * \return      Reference to this matrix after multiplication.
		*//*========================================================================================*/
		eMtx3x3& operator*=(const eMtx3x3& rhs);

		/*============================================================================================
		 * \brief       Builds a 2D translation matrix.
		 * \param[out]  result  Resulting matrix with translation applied.
		 * \param[in]   x       Translation along X axis.
		 * \param[in]   y       Translation along Y axis.
		*//*========================================================================================*/
		void eMtx3x3Translate(eMtx3x3<Type>& result, Type x, Type y);

		/*============================================================================================
		 * \brief       Builds a 2D scaling matrix.
		 * \param[out]  result  Resulting matrix with scaling applied.
		 * \param[in]   x       Scale along X axis.
		 * \param[in]   y       Scale along Y axis.
		*//*========================================================================================*/
		void eMtx3x3Scale(eMtx3x3<Type> result, Type x, Type y);

		/*============================================================================================
		 * \brief       Builds a 2D rotation matrix (angle in radians).
		 * \param[out]  result  Resulting matrix with rotation applied.
		 * \param[in]   angle   Rotation angle in radians.
		*//*========================================================================================*/
		void eMtx3x3RotRad(eMtx3x3<Type>& result, Type angle);

		/*============================================================================================
		 * \brief       Builds a 2D rotation matrix (angle in degrees).
		 * \param[out]  result  Resulting matrix with rotation applied.
		 * \param[in]   angle   Rotation angle in degrees.
		*//*========================================================================================*/
		void eMtx3x3RotDeg(eMtx3x3<Type>& result, Type angle);

		/*============================================================================================
		 * \brief       Computes the transpose of a 3x3 matrix.
		 * \param[out]  result  Transposed matrix.
		 * \param[in]   mtx     Input matrix.
		*//*========================================================================================*/
		void eMtx3x3Transpose(eMtx3x3<Type>& result, const eMtx3x3<Type>& mtx);
	};

	// -------------------------------- 4x4 MATRICES --------------------------------
	template <Arithmetic Type>
	struct eMtx4x4
	{
		Type v1, v2, v3, v4,	// 00, 01, 02, 03
			v5, v6, v7, v8,		// 10, 11, 12, 13
			v9, v10, v11, v12;  // 20, 21, 22, 23

		/*============================================================================================
		 * \brief       Default constructor. Initializes matrix without setting values.
		*//*========================================================================================*/
		eMtx4x4() = default;

	};

	// ------------------------------- MISCELLANEOUS ----------------------------------

	/*============================================================================================
	 * \brief       Compares two values and identify which is greater
	 * \param[in]   a  First value.
	 * \param[in]   b  Second value.
	 * \return      Returns the greater value
	*//*========================================================================================*/
	template <typename T>
	T max(const T& a, const T& b);


	/*============================================================================================
	 * \brief       Compares two values and identify which is lesser
	 * \param[in]   a  First value.
	 * \param[in]   b  Second value.
	 * \return      Returns the lesser value
	*//*========================================================================================*/
	template <typename T>
	T min(const T& a, const T& b);

	/*============================================================================================
	 * \brief       Raises a base value to a power (exponent)
	 * \param[in]   base      The base value
	 * \param[in]   exponent  The exponent value
	 * \return      Returns base raised to the power of exponent
	*//*========================================================================================*/
	template <typename T>
	T pow(const T& base, const T& exponent);

} // End of namespace Math

#include "math.tpp"