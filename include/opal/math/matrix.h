#pragma once

#include "opal/assert.h"
#include "opal/math-base.h"
#include "opal/math/normal3.h"
#include "opal/math/point3.h"
#include "opal/math/point4.h"
#include "opal/math/vector3.h"
#include "opal/math/vector4.h"
#include "opal/type-traits.h"

namespace Opal
{

template <FloatingPoint T, int k_row_count = 4, int k_col_count = 4>
struct Matrix
{
    static_assert(k_row_count > 0);
    static_assert(k_col_count > 0);

    using value_type = T;

    constexpr static int k_row_count_value = k_row_count;
    constexpr static int k_col_count_value = k_col_count;

    T elements[k_row_count][k_col_count];

    /**
     * @brief Constructs a Matrix with no initialization.
     * @example
     *     Matrix<int> defaultMatrix;
     */
    constexpr Matrix();

    /**
     * @brief Constructs a Matrix with values on a diagonal set to a specified values while
     * rest are set to 0.
     * @param value The value to set for all elements of the matrix.
     * @example
     *     Matrix<float> matrixWithValue(2.0f);
     */
    constexpr explicit Matrix(T value);

    /**
     * @brief Constructs a Matrix using a fixed-size array of elements of type T.
     * @param mat_elements An array of elements in row-major order.
     * @example
     *     int elements[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
     *     Matrix<int> matrixFromArray(elements);
     */
    constexpr explicit Matrix(const T (&mat_elements)[k_row_count * k_col_count]);

    /**
     * @brief Constructs a Matrix using two dimensional fixed-size array of elements of type T.
     * @param mat_elements Two dimensional array of elements.
     * @example
     *     int elements[3][3] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
     *     Matrix<int, 3, 3> matrixFromArray(elements);
     */
    constexpr explicit Matrix(const T (&mat_elements)[k_row_count][k_col_count]);

    /**
     * @brief Returns a Matrix with all elements set to zero.
     * @return A Matrix instance initialized with zeros.
     * @example
     *     Matrix<int> zeroMatrix = Matrix<int>::Zero();
     */
    static Matrix Zero();

    /** Operators **/
    T& operator()(i32 row, i32 column);
    const T& operator()(i32 row, i32 column) const;

    bool operator==(const Matrix& other) const;
    bool operator!=(const Matrix& other) const;

    Matrix operator*(const Matrix& other) const;
    Matrix& operator*=(const Matrix& other);
    Matrix operator+(const Matrix& other) const;
    Matrix& operator+=(const Matrix& other);
    Matrix operator-(const Matrix& other) const;
    Matrix& operator-=(const Matrix& other);

    template <typename U>
    Matrix operator*(U scalar) const;
    template <typename U>
    Matrix& operator*=(U scalar);
    template <typename U>
    Matrix operator/(U scalar) const;
    template <typename U>
    Matrix& operator/=(U scalar);

    Point3<T> operator*(const Point3<T>& p) const;
    Point4<T> operator*(const Point4<T>& p) const;
    Vector3<T> operator*(const Vector3<T>& v) const;
    Vector4<T> operator*(const Vector4<T>& v) const;
    Normal3<T> operator*(const Normal3<T>& n) const;
};

template <typename MatrixType, IntegralOrFloatingPoint U>
MatrixType operator*(U scalar, const MatrixType& m);

/**
 * Check if two matrices are equal.
 * @param m1 The first matrix.
 * @param m2 The second matrix.
 * @param epsilon The epsilon value to use for comparison.
 * @return True if the matrices are equal, false otherwise.
 */
template <typename MatrixType>
bool IsEqual(const MatrixType& m1, const MatrixType& m2, typename MatrixType::value_type epsilon);

/**
 * Transpose the matrix.
 * @param m The matrix to transpose.
 * @return The transposed matrix.
 */
template <typename MatrixType>
[[nodiscard]] MatrixType Transpose(const MatrixType& m);

/**
 * Invert the matrix.
 * @param m The matrix to invert.
 * @return The inverted matrix.
 */
template <typename MatrixType>
[[nodiscard]] MatrixType Inverse(const MatrixType& m);

template <typename T>
using Matrix4x4 = Matrix<T, 4, 4>;

template <typename T>
using Matrix3x3 = Matrix<T, 3, 3>;

template <typename T>
using Matrix4x3 = Matrix<T, 4, 3>;

}  // namespace Opal

// Implementation //////////////////////////////////////////////////////////////////////////////////

template <Opal::FloatingPoint T, int k_row_count, int k_col_count>
constexpr Opal::Matrix<T, k_row_count, k_col_count>::Matrix()
{
    // Do nothing
}

template <Opal::FloatingPoint T, int k_row_count, int k_col_count>
constexpr Opal::Matrix<T, k_row_count, k_col_count>::Matrix(T value)
{
    for (i32 i = 0; i < k_row_count; ++i)
    {
        for (i32 j = 0; j < k_col_count; ++j)
        {
            elements[i][j] = (i == j) ? value : 0;
        }
    }
}

template <Opal::FloatingPoint T, int k_row_count, int k_col_count>
constexpr Opal::Matrix<T, k_row_count, k_col_count>::Matrix(const T (&mat_elements)[k_row_count * k_col_count])
{
    memcpy(elements, mat_elements, k_row_count * k_col_count * sizeof(T));
}

template <Opal::FloatingPoint T, int k_row_count, int k_col_count>
constexpr Opal::Matrix<T, k_row_count, k_col_count>::Matrix(const T (&mat_elements)[k_row_count][k_col_count])
{
    memcpy(elements, mat_elements, k_row_count * k_col_count * sizeof(T));
}

template <Opal::FloatingPoint T, int k_row_count, int k_col_count>
Opal::Matrix<T, k_row_count, k_col_count> Opal::Matrix<T, k_row_count, k_col_count>::Zero()
{
    return Matrix<T>(static_cast<T>(0));
}

template <Opal::FloatingPoint T, int k_row_count, int k_col_count>
bool Opal::Matrix<T, k_row_count, k_col_count>::operator==(const Matrix& other) const
{
    return memcmp(elements, other.elements, k_row_count * k_col_count * sizeof(T)) == 0;
}

template <Opal::FloatingPoint T, int k_row_count, int k_col_count>
bool Opal::Matrix<T, k_row_count, k_col_count>::operator!=(const Matrix& other) const
{
    return !(*this == other);
}

template <Opal::FloatingPoint T, int k_row_count, int k_col_count>
T& Opal::Matrix<T, k_row_count, k_col_count>::operator()(i32 row, i32 column)
{
    OPAL_ASSERT(row >= 0 && row < k_row_count, "Row index out of range.");
    OPAL_ASSERT(column >= 0 && column < k_col_count, "Column index out of range.");
    return elements[row][column];
}

template <Opal::FloatingPoint T, int k_row_count, int k_col_count>
const T& Opal::Matrix<T, k_row_count, k_col_count>::operator()(i32 row, i32 column) const
{
    OPAL_ASSERT(row >= 0 && row < k_row_count, "Row index out of range.");
    OPAL_ASSERT(column >= 0 && column < k_col_count, "Column index out of range.");
    return elements[row][column];
}

template <Opal::FloatingPoint T, int k_row_count, int k_col_count>
Opal::Matrix<T, k_row_count, k_col_count> Opal::Matrix<T, k_row_count, k_col_count>::operator*(const Matrix& other) const
{
    Matrix<T> result;
    for (i32 i = 0; i < k_row_count; ++i)
    {
        for (i32 j = 0; j < k_col_count; ++j)
        {
            result.elements[i][j] = 0;
            for (i32 k = 0; k < k_col_count; ++k)
            {
                result.elements[i][j] += elements[i][k] * other.elements[k][j];
            }
        }
    }
    return result;
}

template <Opal::FloatingPoint T, int k_row_count, int k_col_count>
Opal::Matrix<T, k_row_count, k_col_count>& Opal::Matrix<T, k_row_count, k_col_count>::operator*=(const Matrix& other)
{
    *this = *this * other;
    return *this;
}

template <Opal::FloatingPoint T, int k_row_count, int k_col_count>
Opal::Matrix<T, k_row_count, k_col_count> Opal::Matrix<T, k_row_count, k_col_count>::operator+(const Matrix& other) const
{
    Matrix<T> result;
    for (i32 i = 0; i < k_row_count; ++i)
    {
        for (i32 j = 0; j < k_col_count; ++j)
        {
            result.elements[i][j] = elements[i][j] + other.elements[i][j];
        }
    }
    return result;
}

template <Opal::FloatingPoint T, int k_row_count, int k_col_count>
Opal::Matrix<T, k_row_count, k_col_count>& Opal::Matrix<T, k_row_count, k_col_count>::operator+=(const Matrix& other)
{
    *this = *this + other;
    return *this;
}

template <Opal::FloatingPoint T, int k_row_count, int k_col_count>
Opal::Matrix<T, k_row_count, k_col_count> Opal::Matrix<T, k_row_count, k_col_count>::operator-(const Matrix& other) const
{
    Matrix<T> result;
    for (i32 i = 0; i < k_row_count; ++i)
    {
        for (i32 j = 0; j < k_col_count; ++j)
        {
            result.elements[i][j] = elements[i][j] - other.elements[i][j];
        }
    }
    return result;
}

template <Opal::FloatingPoint T, int k_row_count, int k_col_count>
Opal::Matrix<T, k_row_count, k_col_count>& Opal::Matrix<T, k_row_count, k_col_count>::operator-=(const Matrix& other)
{
    *this = *this - other;
    return *this;
}

template <Opal::FloatingPoint T, int k_row_count, int k_col_count>
template <typename U>
Opal::Matrix<T, k_row_count, k_col_count> Opal::Matrix<T, k_row_count, k_col_count>::operator*(U scalar) const
{
    Matrix<T> result;
    for (i32 i = 0; i < k_row_count; ++i)
    {
        for (i32 j = 0; j < k_col_count; ++j)
        {
            result.elements[i][j] = elements[i][j] * static_cast<T>(scalar);
        }
    }
    return result;
}

template <Opal::FloatingPoint T, int k_row_count, int k_col_count>
template <typename U>
Opal::Matrix<T, k_row_count, k_col_count>& Opal::Matrix<T, k_row_count, k_col_count>::operator*=(U scalar)
{
    *this = *this * scalar;
    return *this;
}

template <Opal::FloatingPoint T, int k_row_count, int k_col_count>
template <typename U>
Opal::Matrix<T, k_row_count, k_col_count> Opal::Matrix<T, k_row_count, k_col_count>::operator/(U scalar) const
{
    Matrix<T> result;
    for (i32 i = 0; i < k_row_count; ++i)
    {
        for (i32 j = 0; j < k_col_count; ++j)
        {
            result.elements[i][j] = elements[i][j] / static_cast<T>(scalar);
        }
    }
    return result;
}

template <Opal::FloatingPoint T, int k_row_count, int k_col_count>
template <typename U>
Opal::Matrix<T, k_row_count, k_col_count>& Opal::Matrix<T, k_row_count, k_col_count>::operator/=(U scalar)
{
    *this = *this / scalar;
    return *this;
}

template <Opal::FloatingPoint T, int k_row_count, int k_col_count>
Opal::Point3<T> Opal::Matrix<T, k_row_count, k_col_count>::operator*(const Point3<T>& p) const
{
    OPAL_ASSERT(k_col_count == 4, "Matrix must be 3x4 or 4x4.");
    OPAL_ASSERT(k_row_count == 3 || k_row_count == 4, "Matrix must be 3x4 or 4x4.");
    const T x = elements[0][0] * p.x + elements[0][1] * p.y + elements[0][2] * p.z + elements[0][3];
    const T y = elements[1][0] * p.x + elements[1][1] * p.y + elements[1][2] * p.z + elements[1][3];
    const T z = elements[2][0] * p.x + elements[2][1] * p.y + elements[2][2] * p.z + elements[2][3];
    // Here we allow simple optimization where the last row is [0, 0, 0, 1] for most of the transform matrices.
    if constexpr (k_row_count == 4)
    {
        const T w = elements[3][0] * p.x + elements[3][1] * p.y + elements[3][2] * p.z + elements[3][3];
        return Point3<T>(x / w, y / w, z / w);
    }
    else
    {
        return Point3<T>(x, y, z);
    }
}

template <Opal::FloatingPoint T, int k_row_count, int k_col_count>
Opal::Point4<T> Opal::Matrix<T, k_row_count, k_col_count>::operator*(const Point4<T>& p) const
{
    OPAL_ASSERT(k_col_count == 4, "Matrix must be 3x4 or 4x4.");
    OPAL_ASSERT(k_row_count == 3 || k_row_count == 4, "Matrix must be 3x4 or 4x4.");
    const T x = elements[0][0] * p.x + elements[0][1] * p.y + elements[0][2] * p.z + elements[0][3] * p.w;
    const T y = elements[1][0] * p.x + elements[1][1] * p.y + elements[1][2] * p.z + elements[1][3] * p.w;
    const T z = elements[2][0] * p.x + elements[2][1] * p.y + elements[2][2] * p.z + elements[2][3] * p.w;
    if constexpr (k_row_count == 4)
    {
        const T w = elements[3][0] * p.x + elements[3][1] * p.y + elements[3][2] * p.z + elements[3][3] * p.w;
        return Point4<T>(x, y, z, w);
    }
    else
    {
        return Point4<T>(x, y, z, static_cast<T>(1));
    }
}

template <Opal::FloatingPoint T, int k_row_count, int k_col_count>
Opal::Vector3<T> Opal::Matrix<T, k_row_count, k_col_count>::operator*(const Vector3<T>& v) const
{
    OPAL_ASSERT(k_col_count == 4, "Matrix must be 3x4 or 4x4.");
    OPAL_ASSERT(k_row_count == 3 || k_row_count == 4, "Matrix must be 3x4 or 4x4.");
    if constexpr (k_row_count == 4)
    {
        OPAL_ASSERT(elements[3][0] == 0 && elements[3][1] == 0 && elements[3][2] == 0 && elements[3][3] == 1,
                    "Fourth row of matrix must be [0, 0, 0, 1].");
    }

    const T x = elements[0][0] * v.x + elements[0][1] * v.y + elements[0][2] * v.z;
    const T y = elements[1][0] * v.x + elements[1][1] * v.y + elements[1][2] * v.z;
    const T z = elements[2][0] * v.x + elements[2][1] * v.y + elements[2][2] * v.z;
    return Vector3<T>(x, y, z);
}

template <Opal::FloatingPoint T, int k_row_count, int k_col_count>
Opal::Vector4<T> Opal::Matrix<T, k_row_count, k_col_count>::operator*(const Vector4<T>& v) const
{
    OPAL_ASSERT(k_col_count == 4, "Matrix must be 3x4 or 4x4.");
    OPAL_ASSERT(k_row_count == 3 || k_row_count == 4, "Matrix must be 3x4 or 4x4.");
    const T x = elements[0][0] * v.x + elements[0][1] * v.y + elements[0][2] * v.z + elements[0][3] * v.w;
    const T y = elements[1][0] * v.x + elements[1][1] * v.y + elements[1][2] * v.z + elements[1][3] * v.w;
    const T z = elements[2][0] * v.x + elements[2][1] * v.y + elements[2][2] * v.z + elements[2][3] * v.w;
    if constexpr (k_row_count == 4)
    {
        const T w = elements[3][0] * v.x + elements[3][1] * v.y + elements[3][2] * v.z + elements[3][3] * v.w;
        return Vector4<T>(x, y, z, w);
    }
    else
    {
        return Vector4<T>(x, y, z, static_cast<T>(1));
    }
}

template <Opal::FloatingPoint T, int k_row_count, int k_col_count>
Opal::Normal3<T> Opal::Matrix<T, k_row_count, k_col_count>::operator*(const Normal3<T>& n) const
{
    OPAL_ASSERT(k_col_count == 4, "Matrix must be 3x4 or 4x4.");
    OPAL_ASSERT(k_row_count == 3 || k_row_count == 4, "Matrix must be 3x4 or 4x4.");
    const T x = elements[0][0] * n.x + elements[1][0] * n.y + elements[2][0] * n.z;
    const T y = elements[0][1] * n.x + elements[1][1] * n.y + elements[2][1] * n.z;
    const T z = elements[0][2] * n.x + elements[1][2] * n.y + elements[2][2] * n.z;
    return Normal3<T>(x, y, z);
}

template <typename MatrixType, Opal::IntegralOrFloatingPoint U>
MatrixType Opal::operator*(U scalar, const MatrixType& m)
{
    return m * static_cast<typename MatrixType::value_type>(scalar);
}

template <typename MatrixType>
bool Opal::IsEqual(const MatrixType& m1, const MatrixType& m2, typename MatrixType::value_type epsilon)
{
    for (i32 i = 0; i < MatrixType::k_row_count_value; ++i)
    {
        for (i32 j = 0; j < MatrixType::k_col_count_value; ++j)
        {
            if (!Opal::IsEqual(m1.elements[i][j], m2.elements[i][j], epsilon))
            {
                return false;
            }
        }
    }
    return true;
}

template <typename MatrixType>
MatrixType Opal::Transpose(const MatrixType& m)
{
    MatrixType result;
    for (i32 i = 0; i < MatrixType::k_row_count_value; ++i)
    {
        for (i32 j = 0; j < MatrixType::k_col_count_value; ++j)
        {
            result.elements[i][j] = m.elements[j][i];
        }
    }
    return result;
}

template <typename MatrixType>
MatrixType Opal::Inverse(const MatrixType& m)
{
    using value_type = typename MatrixType::value_type;
    OPAL_ASSERT(MatrixType::k_row_count_value == MatrixType::k_col_count_value, "The matrix must be symmetric");
    int indxc[MatrixType::k_row_count_value] = {};
    int indxr[MatrixType::k_row_count_value] = {};
    int ipiv[MatrixType::k_row_count_value] = {};
    value_type mat_inv[MatrixType::k_row_count_value][MatrixType::k_row_count_value] = {};
    memcpy(mat_inv, m.elements, sizeof(MatrixType));
    for (int it = 0; it < MatrixType::k_row_count_value; it++)
    {
        int index_row = 0;
        int index_column = 0;
        value_type big = 0;
        // Choose pivot
        for (int row = 0; row < MatrixType::k_row_count_value; row++)
        {
            if (ipiv[row] == 1)
            {
                continue;
            }
            for (int column = 0; column < MatrixType::k_col_count_value; column++)
            {
                if (ipiv[column] == 0)
                {
                    if (Opal::Abs(mat_inv[row][column]) >= big)
                    {
                        big = Opal::Abs(mat_inv[row][column]);
                        index_row = row;
                        index_column = column;
                    }
                }
                else if (ipiv[column] > 1)
                {
                    OPAL_ASSERT(false, "Singular matrix");
                }
            }
        }
        ++ipiv[index_column];
        // Swap rows _irow_ and _icol_ for pivot
        if (index_row != index_column)
        {
            for (int column = 0; column < MatrixType::k_col_count_value; column++)
            {
                std::swap(mat_inv[index_row][column], mat_inv[index_column][column]);
            }
        }
        indxr[it] = index_row;
        indxc[it] = index_column;
        if (mat_inv[index_column][index_column] == 0)
        {
            OPAL_ASSERT(false, "Singular matrix");
        }

        // Set m[icol][icol] to one by scaling row _icol_ appropriately
        const value_type pivinv = 1 / mat_inv[index_column][index_column];
        mat_inv[index_column][index_column] = 1;
        for (int column = 0; column < MatrixType::k_col_count_value; column++)
        {
            mat_inv[index_column][column] *= pivinv;
        }

        // Subtract this row from Others to zero out their columns
        for (int row = 0; row < MatrixType::k_row_count_value; row++)
        {
            if (row != index_column)
            {
                const value_type save = mat_inv[row][index_column];
                mat_inv[row][index_column] = 0;
                for (int column = 0; column < MatrixType::k_col_count_value; column++)
                {
                    mat_inv[row][column] -= mat_inv[index_column][column] * save;
                }
            }
        }
    }

    // Swap columns to reflect permutation
    for (int column = MatrixType::k_col_count_value - 1; column >= 0; --column)
    {
        if (indxr[column] != indxc[column])
        {
            for (int row = 0; row < 4; row++)
            {
                std::swap(mat_inv[row][indxr[column]], mat_inv[row][indxc[column]]);
            }
        }
    }

    return Matrix(mat_inv);
}
