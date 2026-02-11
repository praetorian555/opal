#include <limits>

#include "test-helpers.h"

#include "opal/math/matrix.h"

using namespace Opal;

using Matrix4x4f = Matrix<f32, 4, 4>;
using Matrix4x4d = Matrix<f64, 4, 4>;
using Matrix3x3f = Matrix<f32, 3, 3>;
using Matrix3x3d = Matrix<f64, 3, 3>;

TEST_CASE("Matrix 4x4 constructor", "[math][matrix]")
{
    SECTION("float")
    {
        const Matrix4x4f m1(1);
        CHECK(m1(0, 0) == 1.0f);
        CHECK(m1(1, 1) == 1.0f);
        CHECK(m1(2, 2) == 1.0f);
        CHECK(m1(3, 3) == 1.0f);
        CHECK(m1(0, 1) == 0.0f);
        CHECK(m1(0, 2) == 0.0f);
        CHECK(m1(0, 3) == 0.0f);
        CHECK(m1(1, 0) == 0.0f);
        CHECK(m1(1, 2) == 0.0f);
        CHECK(m1(1, 3) == 0.0f);
        CHECK(m1(2, 0) == 0.0f);
        CHECK(m1(2, 1) == 0.0f);
        CHECK(m1(2, 3) == 0.0f);
        CHECK(m1(3, 0) == 0.0f);
        CHECK(m1(3, 1) == 0.0f);
        CHECK(m1(3, 2) == 0.0f);

        constexpr f32 data1[] = {1.0f, 4.0f, -4.0f, 5.0f, 3.0f, -2.0f, 1.0f, 10.0f, 7.0f, 10.0f, -5.0f, -3.0f, -6.0f, -2.0f, -1.0f, 9.0f};
        const Matrix4x4f m2(data1);

        CHECK(m2(0, 0) == 1.0f);
        CHECK(m2(0, 1) == 4.0f);
        CHECK(m2(0, 2) == -4.0f);
        CHECK(m2(0, 3) == 5.0f);
        CHECK(m2(1, 0) == 3.0f);
        CHECK(m2(1, 1) == -2.0f);
        CHECK(m2(1, 2) == 1.0f);
        CHECK(m2(1, 3) == 10.0f);
        CHECK(m2(2, 0) == 7.0f);
        CHECK(m2(2, 1) == 10.0f);
        CHECK(m2(2, 2) == -5.0f);
        CHECK(m2(2, 3) == -3.0f);
        CHECK(m2(3, 0) == -6.0f);
        CHECK(m2(3, 1) == -2.0f);
        CHECK(m2(3, 2) == -1.0f);
        CHECK(m2(3, 3) == 9.0f);

        f32 data2[4][4] = {{1.0f, 4.0f, -4.0f, 5.0f}, {3.0f, -2.0f, 1.0f, 10.0f}, {7.0f, 10.0f, -5.0f, -3.0f}, {-6.0f, -2.0f, -1.0f, 9.0f}};

        Matrix4x4f m3(data2);
        CHECK(m3(0, 0) == 1.0f);
        CHECK(m3(0, 1) == 4.0f);
        CHECK(m3(0, 2) == -4.0f);
        CHECK(m3(0, 3) == 5.0f);
        CHECK(m3(1, 0) == 3.0f);
        CHECK(m3(1, 1) == -2.0f);
        CHECK(m3(1, 2) == 1.0f);
        CHECK(m3(1, 3) == 10.0f);
        CHECK(m3(2, 0) == 7.0f);
        CHECK(m3(2, 1) == 10.0f);
        CHECK(m3(2, 2) == -5.0f);
        CHECK(m3(2, 3) == -3.0f);
        CHECK(m3(3, 0) == -6.0f);
        CHECK(m3(3, 1) == -2.0f);
        CHECK(m3(3, 2) == -1.0f);
        CHECK(m3(3, 3) == 9.0f);
    }
    SECTION("double")
    {
        const Matrix4x4d m1(1);
        CHECK(m1(0, 0) == 1.0);
        CHECK(m1(1, 1) == 1.0);
        CHECK(m1(2, 2) == 1.0);
        CHECK(m1(3, 3) == 1.0);
        CHECK(m1(0, 1) == 0.0);
        CHECK(m1(0, 2) == 0.0);
        CHECK(m1(0, 3) == 0.0);
        CHECK(m1(1, 0) == 0.0);
        CHECK(m1(1, 2) == 0.0);
        CHECK(m1(1, 3) == 0.0);
        CHECK(m1(2, 0) == 0.0);
        CHECK(m1(2, 1) == 0.0);
        CHECK(m1(2, 3) == 0.0);
        CHECK(m1(3, 0) == 0.0);
        CHECK(m1(3, 1) == 0.0);
        CHECK(m1(3, 2) == 0.0);

        f64 data1[16] = {1.0, 4.0, -4.0, 5.0, 3.0, -2.0, 1.0, 10.0, 7.0, 10.0, -5.0, -3.0, -6.0, -2.0, -1.0, 9.0};
        const Matrix4x4d m2(data1);

        CHECK(m2(0, 0) == 1.0);
        CHECK(m2(0, 1) == 4.0);
        CHECK(m2(0, 2) == -4.0);
        CHECK(m2(0, 3) == 5.0);
        CHECK(m2(1, 0) == 3.0);
        CHECK(m2(1, 1) == -2.0);
        CHECK(m2(1, 2) == 1.0);
        CHECK(m2(1, 3) == 10.0);
        CHECK(m2(2, 0) == 7.0);
        CHECK(m2(2, 1) == 10.0);
        CHECK(m2(2, 2) == -5.0);
        CHECK(m2(2, 3) == -3.0);
        CHECK(m2(3, 0) == -6.0);
        CHECK(m2(3, 1) == -2.0);
        CHECK(m2(3, 2) == -1.0);
        CHECK(m2(3, 3) == 9.0);

        f64 data2[4][4] = {{1.0, 4.0, -4.0, 5.0}, {3.0, -2.0, 1.0, 10.0}, {7.0, 10.0, -5.0, -3.0}, {-6.0, -2.0, -1.0, 9.0}};

        const Matrix4x4d m3(data2);
        CHECK(m3(0, 0) == 1.0);
        CHECK(m3(0, 1) == 4.0);
        CHECK(m3(0, 2) == -4.0);
        CHECK(m3(0, 3) == 5.0);
        CHECK(m3(1, 0) == 3.0);
        CHECK(m3(1, 1) == -2.0);
        CHECK(m3(1, 2) == 1.0);
        CHECK(m3(1, 3) == 10.0);
        CHECK(m3(2, 0) == 7.0);
        CHECK(m3(2, 1) == 10.0);
        CHECK(m3(2, 2) == -5.0);
        CHECK(m3(2, 3) == -3.0);
        CHECK(m3(3, 0) == -6.0);
        CHECK(m3(3, 1) == -2.0);
        CHECK(m3(3, 2) == -1.0);
        CHECK(m3(3, 3) == 9.0);
    }
}

TEST_CASE("Matrix 4x4 comparison", "[math][matrix]")
{
    SECTION("float")
    {
        f32 data[16] = {1.0f, 4.0f, -4.0f, 5.0f, 3.0f, -2.0f, 1.0f, 10.0f, 7.0f, 10.0f, -5.0f, -3.0f, -6.0f, -2.0f, -1.0f, 9.0f};

        const Matrix4x4f m1(1);
        const Matrix4x4f m2(data);
        const Matrix4x4f m3(data);

        CHECK(m1 != m2);
        CHECK(m2 == m3);
    }
    SECTION("double")
    {
        f64 data[16] = {1.0, 4.0, -4.0, 5.0, 3.0, -2.0, 1.0, 10.0, 7.0, 10.0, -5.0, -3.0, -6.0, -2.0, -1.0, 9.0};

        const Matrix4x4d m1(1);
        const Matrix4x4d m2(data);
        const Matrix4x4d m3(data);

        CHECK(m1 != m2);
        CHECK(m2 == m3);
    }
}

TEST_CASE("Matrix 4x4 multiplication with another matrix", "[math][matrix]")
{
    SECTION("float")
    {
        f32 data1[4][4] = {{2, 2, 2, 2}, {3, 3, 3, 3}, {4, 4, 4, 4}, {5, 5, 5, 5}};
        f32 data2[4][4] = {{2, 3, 4, 5}, {2, 3, 4, 5}, {2, 3, 4, 5}, {2, 3, 4, 5}};
        const Matrix4x4f m1(data1);
        const Matrix4x4f m2(data2);

        Matrix4x4f m3 = m1 * m2;
        CHECK(m3(0, 0) == 16.0f);
        CHECK(m3(0, 1) == 24.0f);
        CHECK(m3(0, 2) == 32.0f);
        CHECK(m3(0, 3) == 40.0f);
        CHECK(m3(1, 0) == 24.0f);
        CHECK(m3(1, 1) == 36.0f);
        CHECK(m3(1, 2) == 48.0f);
        CHECK(m3(1, 3) == 60.0f);
        CHECK(m3(2, 0) == 32.0f);
        CHECK(m3(2, 1) == 48.0f);
        CHECK(m3(2, 2) == 64.0f);
        CHECK(m3(2, 3) == 80.0f);
        CHECK(m3(3, 0) == 40.0f);
        CHECK(m3(3, 1) == 60.0f);
        CHECK(m3(3, 2) == 80.0f);
        CHECK(m3(3, 3) == 100.0f);

        m3 = m1;
        CHECK(m3(0, 0) == 2.0f);
        CHECK(m3(0, 1) == 2.0f);
        CHECK(m3(0, 2) == 2.0f);
        CHECK(m3(0, 3) == 2.0f);
        CHECK(m3(1, 0) == 3.0f);
        CHECK(m3(1, 1) == 3.0f);
        CHECK(m3(1, 2) == 3.0f);
        CHECK(m3(1, 3) == 3.0f);
        CHECK(m3(2, 0) == 4.0f);
        CHECK(m3(2, 1) == 4.0f);
        CHECK(m3(2, 2) == 4.0f);
        CHECK(m3(2, 3) == 4.0f);
        CHECK(m3(3, 0) == 5.0f);
        CHECK(m3(3, 1) == 5.0f);
        CHECK(m3(3, 2) == 5.0f);
        CHECK(m3(3, 3) == 5.0f);

        m3 *= m2;
        CHECK(m3(0, 0) == 16.0f);
        CHECK(m3(0, 1) == 24.0f);
        CHECK(m3(0, 2) == 32.0f);
        CHECK(m3(0, 3) == 40.0f);
        CHECK(m3(1, 0) == 24.0f);
        CHECK(m3(1, 1) == 36.0f);
        CHECK(m3(1, 2) == 48.0f);
        CHECK(m3(1, 3) == 60.0f);
        CHECK(m3(2, 0) == 32.0f);
        CHECK(m3(2, 1) == 48.0f);
        CHECK(m3(2, 2) == 64.0f);
        CHECK(m3(2, 3) == 80.0f);
        CHECK(m3(3, 0) == 40.0f);
        CHECK(m3(3, 1) == 60.0f);
        CHECK(m3(3, 2) == 80.0f);
        CHECK(m3(3, 3) == 100.0f);
    }
    SECTION("double")
    {
        f64 data1[4][4] = {{2, 2, 2, 2}, {3, 3, 3, 3}, {4, 4, 4, 4}, {5, 5, 5, 5}};
        f64 data2[4][4] = {{2, 3, 4, 5}, {2, 3, 4, 5}, {2, 3, 4, 5}, {2, 3, 4, 5}};

        const Matrix4x4d m1(data1);
        const Matrix4x4d m2(data2);

        Matrix4x4d m3 = m1 * m2;
        CHECK(m3(0, 0) == 16.0);
        CHECK(m3(0, 1) == 24.0);
        CHECK(m3(0, 2) == 32.0);
        CHECK(m3(0, 3) == 40.0);
        CHECK(m3(1, 0) == 24.0);
        CHECK(m3(1, 1) == 36.0);
        CHECK(m3(1, 2) == 48.0);
        CHECK(m3(1, 3) == 60.0);
        CHECK(m3(2, 0) == 32.0);
        CHECK(m3(2, 1) == 48.0);
        CHECK(m3(2, 2) == 64.0);
        CHECK(m3(2, 3) == 80.0);
        CHECK(m3(3, 0) == 40.0);
        CHECK(m3(3, 1) == 60.0);
        CHECK(m3(3, 2) == 80.0);
        CHECK(m3(3, 3) == 100.0);

        m3 = m1;
        CHECK(m3(0, 0) == 2.0);
        CHECK(m3(0, 1) == 2.0);
        CHECK(m3(0, 2) == 2.0);
        CHECK(m3(0, 3) == 2.0);
        CHECK(m3(1, 0) == 3.0);
        CHECK(m3(1, 1) == 3.0);
        CHECK(m3(1, 2) == 3.0);
        CHECK(m3(1, 3) == 3.0);
        CHECK(m3(2, 0) == 4.0);
        CHECK(m3(2, 1) == 4.0);
        CHECK(m3(2, 2) == 4.0);
        CHECK(m3(2, 3) == 4.0);
        CHECK(m3(3, 0) == 5.0);
        CHECK(m3(3, 1) == 5.0);
        CHECK(m3(3, 2) == 5.0);
        CHECK(m3(3, 3) == 5.0);

        m3 *= m2;
        CHECK(m3(0, 0) == 16.0);
        CHECK(m3(0, 1) == 24.0);
        CHECK(m3(0, 2) == 32.0);
        CHECK(m3(0, 3) == 40.0);
        CHECK(m3(1, 0) == 24.0);
        CHECK(m3(1, 1) == 36.0);
        CHECK(m3(1, 2) == 48.0);
        CHECK(m3(1, 3) == 60.0);
        CHECK(m3(2, 0) == 32.0);
        CHECK(m3(2, 1) == 48.0);
        CHECK(m3(2, 2) == 64.0);
        CHECK(m3(2, 3) == 80.0);
        CHECK(m3(3, 0) == 40.0);
        CHECK(m3(3, 1) == 60.0);
        CHECK(m3(3, 2) == 80.0);
        CHECK(m3(3, 3) == 100.0);
    }
}

TEST_CASE("Matrix 4x4 addition", "[math][matrix]")
{
    SECTION("float")
    {
        const f32 data1[4][4] = {{2, 2, 2, 2}, {3, 3, 3, 3}, {4, 4, 4, 4}, {5, 5, 5, 5}};
        const f32 data2[4][4] = {{2, 3, 4, 5}, {2, 3, 4, 5}, {2, 3, 4, 5}, {2, 3, 4, 5}};
        const Matrix4x4f m1(data1);
        const Matrix4x4f m2(data2);

        Matrix4x4f m3 = m1 + m2;
        CHECK(m3(0, 0) == 4.0f);
        CHECK(m3(0, 1) == 5.0f);
        CHECK(m3(0, 2) == 6.0f);
        CHECK(m3(0, 3) == 7.0f);
        CHECK(m3(1, 0) == 5.0f);
        CHECK(m3(1, 1) == 6.0f);
        CHECK(m3(1, 2) == 7.0f);
        CHECK(m3(1, 3) == 8.0f);
        CHECK(m3(2, 0) == 6.0f);
        CHECK(m3(2, 1) == 7.0f);
        CHECK(m3(2, 2) == 8.0f);
        CHECK(m3(2, 3) == 9.0f);
        CHECK(m3(3, 0) == 7.0f);
        CHECK(m3(3, 1) == 8.0f);
        CHECK(m3(3, 2) == 9.0f);
        CHECK(m3(3, 3) == 10.0f);

        m3 = m1;
        CHECK(m3(0, 0) == 2.0f);
        CHECK(m3(0, 1) == 2.0f);
        CHECK(m3(0, 2) == 2.0f);
        CHECK(m3(0, 3) == 2.0f);
        CHECK(m3(1, 0) == 3.0f);
        CHECK(m3(1, 1) == 3.0f);
        CHECK(m3(1, 2) == 3.0f);
        CHECK(m3(1, 3) == 3.0f);
        CHECK(m3(2, 0) == 4.0f);
        CHECK(m3(2, 1) == 4.0f);
        CHECK(m3(2, 2) == 4.0f);
        CHECK(m3(2, 3) == 4.0f);
        CHECK(m3(3, 0) == 5.0f);
        CHECK(m3(3, 1) == 5.0f);
        CHECK(m3(3, 2) == 5.0f);
        CHECK(m3(3, 3) == 5.0f);

        m3 += m2;
        CHECK(m3(0, 0) == 4.0f);
        CHECK(m3(0, 1) == 5.0f);
        CHECK(m3(0, 2) == 6.0f);
        CHECK(m3(0, 3) == 7.0f);
        CHECK(m3(1, 0) == 5.0f);
        CHECK(m3(1, 1) == 6.0f);
        CHECK(m3(1, 2) == 7.0f);
        CHECK(m3(1, 3) == 8.0f);
        CHECK(m3(2, 0) == 6.0f);
        CHECK(m3(2, 1) == 7.0f);
        CHECK(m3(2, 2) == 8.0f);
        CHECK(m3(2, 3) == 9.0f);
        CHECK(m3(3, 0) == 7.0f);
        CHECK(m3(3, 1) == 8.0f);
        CHECK(m3(3, 2) == 9.0f);
        CHECK(m3(3, 3) == 10.0f);
    }
    SECTION("double")
    {
        const f64 data1[4][4] = {{2, 2, 2, 2}, {3, 3, 3, 3}, {4, 4, 4, 4}, {5, 5, 5, 5}};
        const f64 data2[4][4] = {{2, 3, 4, 5}, {2, 3, 4, 5}, {2, 3, 4, 5}, {2, 3, 4, 5}};

        const Matrix4x4d m1(data1);
        const Matrix4x4d m2(data2);

        Matrix4x4d m3 = m1 + m2;
        CHECK(m3(0, 0) == 4.0);
        CHECK(m3(0, 1) == 5.0);
        CHECK(m3(0, 2) == 6.0);
        CHECK(m3(0, 3) == 7.0);
        CHECK(m3(1, 0) == 5.0);
        CHECK(m3(1, 1) == 6.0);
        CHECK(m3(1, 2) == 7.0);
        CHECK(m3(1, 3) == 8.0);
        CHECK(m3(2, 0) == 6.0);
        CHECK(m3(2, 1) == 7.0);
        CHECK(m3(2, 2) == 8.0);
        CHECK(m3(2, 3) == 9.0);
        CHECK(m3(3, 0) == 7.0);
        CHECK(m3(3, 1) == 8.0);
        CHECK(m3(3, 2) == 9.0);
        CHECK(m3(3, 3) == 10.0);

        m3 = m1;
        CHECK(m3(0, 0) == 2.0);
        CHECK(m3(0, 1) == 2.0);
        CHECK(m3(0, 2) == 2.0);
        CHECK(m3(0, 3) == 2.0);
        CHECK(m3(1, 0) == 3.0);
        CHECK(m3(1, 1) == 3.0);
        CHECK(m3(1, 2) == 3.0);
        CHECK(m3(1, 3) == 3.0);
        CHECK(m3(2, 0) == 4.0);
        CHECK(m3(2, 1) == 4.0);
        CHECK(m3(2, 2) == 4.0);
        CHECK(m3(2, 3) == 4.0);
        CHECK(m3(3, 0) == 5.0);
        CHECK(m3(3, 1) == 5.0);
        CHECK(m3(3, 2) == 5.0);
        CHECK(m3(3, 3) == 5.0);

        m3 += m2;
        CHECK(m3(0, 0) == 4.0);
        CHECK(m3(0, 1) == 5.0);
        CHECK(m3(0, 2) == 6.0);
        CHECK(m3(0, 3) == 7.0);
        CHECK(m3(1, 0) == 5.0);
        CHECK(m3(1, 1) == 6.0);
        CHECK(m3(1, 2) == 7.0);
        CHECK(m3(1, 3) == 8.0);
        CHECK(m3(2, 0) == 6.0);
        CHECK(m3(2, 1) == 7.0);
        CHECK(m3(2, 2) == 8.0);
        CHECK(m3(2, 3) == 9.0);
        CHECK(m3(3, 0) == 7.0);
        CHECK(m3(3, 1) == 8.0);
        CHECK(m3(3, 2) == 9.0);
        CHECK(m3(3, 3) == 10.0);
    }
}

TEST_CASE("Matrix 4x4 subtraction", "[math][matrix]")
{
    SECTION("float")
    {
        f32 data1[4][4] = {{6, 6, 6, 6}, {7, 7, 7, 7}, {8, 8, 8, 8}, {9, 9, 9, 9}};
        f32 data2[4][4] = {{2, 3, 4, 5}, {2, 3, 4, 5}, {2, 3, 4, 5}, {2, 3, 4, 5}};
        const Matrix4x4f m1(data1);
        const Matrix4x4f m2(data2);

        Matrix4x4f m3 = m1 - m2;
        CHECK(m3(0, 0) == 4.0f);
        CHECK(m3(0, 1) == 3.0f);
        CHECK(m3(0, 2) == 2.0f);
        CHECK(m3(0, 3) == 1.0f);
        CHECK(m3(1, 0) == 5.0f);
        CHECK(m3(1, 1) == 4.0f);
        CHECK(m3(1, 2) == 3.0f);
        CHECK(m3(1, 3) == 2.0f);
        CHECK(m3(2, 0) == 6.0f);
        CHECK(m3(2, 1) == 5.0f);
        CHECK(m3(2, 2) == 4.0f);
        CHECK(m3(2, 3) == 3.0f);
        CHECK(m3(3, 0) == 7.0f);
        CHECK(m3(3, 1) == 6.0f);
        CHECK(m3(3, 2) == 5.0f);
        CHECK(m3(3, 3) == 4.0f);

        m3 = m1;
        CHECK(m3(0, 0) == 6.0f);
        CHECK(m3(0, 1) == 6.0f);
        CHECK(m3(0, 2) == 6.0f);
        CHECK(m3(0, 3) == 6.0f);
        CHECK(m3(1, 0) == 7.0f);
        CHECK(m3(1, 1) == 7.0f);
        CHECK(m3(1, 2) == 7.0f);
        CHECK(m3(1, 3) == 7.0f);
        CHECK(m3(2, 0) == 8.0f);
        CHECK(m3(2, 1) == 8.0f);
        CHECK(m3(2, 2) == 8.0f);
        CHECK(m3(2, 3) == 8.0f);
        CHECK(m3(3, 0) == 9.0f);
        CHECK(m3(3, 1) == 9.0f);
        CHECK(m3(3, 2) == 9.0f);
        CHECK(m3(3, 3) == 9.0f);

        m3 -= m2;
        CHECK(m3(0, 0) == 4.0f);
        CHECK(m3(0, 1) == 3.0f);
        CHECK(m3(0, 2) == 2.0f);
        CHECK(m3(0, 3) == 1.0f);
        CHECK(m3(1, 0) == 5.0f);
        CHECK(m3(1, 1) == 4.0f);
        CHECK(m3(1, 2) == 3.0f);
        CHECK(m3(1, 3) == 2.0f);
        CHECK(m3(2, 0) == 6.0f);
        CHECK(m3(2, 1) == 5.0f);
        CHECK(m3(2, 2) == 4.0f);
        CHECK(m3(2, 3) == 3.0f);
        CHECK(m3(3, 0) == 7.0f);
        CHECK(m3(3, 1) == 6.0f);
        CHECK(m3(3, 2) == 5.0f);
        CHECK(m3(3, 3) == 4.0f);
    }
    SECTION("double")
    {
        f64 data1[4][4] = {{6, 6, 6, 6}, {7, 7, 7, 7}, {8, 8, 8, 8}, {9, 9, 9, 9}};
        f64 data2[4][4] = {{2, 3, 4, 5}, {2, 3, 4, 5}, {2, 3, 4, 5}, {2, 3, 4, 5}};
        const Matrix4x4d m1(data1);
        const Matrix4x4d m2(data2);

        Matrix4x4d m3 = m1 - m2;
        CHECK(m3(0, 0) == 4.0);
        CHECK(m3(0, 1) == 3.0);
        CHECK(m3(0, 2) == 2.0);
        CHECK(m3(0, 3) == 1.0);
        CHECK(m3(1, 0) == 5.0);
        CHECK(m3(1, 1) == 4.0);
        CHECK(m3(1, 2) == 3.0);
        CHECK(m3(1, 3) == 2.0);
        CHECK(m3(2, 0) == 6.0);
        CHECK(m3(2, 1) == 5.0);
        CHECK(m3(2, 2) == 4.0);
        CHECK(m3(2, 3) == 3.0);
        CHECK(m3(3, 0) == 7.0);
        CHECK(m3(3, 1) == 6.0);
        CHECK(m3(3, 2) == 5.0);
        CHECK(m3(3, 3) == 4.0);

        m3 = m1;
        CHECK(m3(0, 0) == 6.0);
        CHECK(m3(0, 1) == 6.0);
        CHECK(m3(0, 2) == 6.0);
        CHECK(m3(0, 3) == 6.0);
        CHECK(m3(1, 0) == 7.0);
        CHECK(m3(1, 1) == 7.0);
        CHECK(m3(1, 2) == 7.0);
        CHECK(m3(1, 3) == 7.0);
        CHECK(m3(2, 0) == 8.0);
        CHECK(m3(2, 1) == 8.0);
        CHECK(m3(2, 2) == 8.0);
        CHECK(m3(2, 3) == 8.0);
        CHECK(m3(3, 0) == 9.0);
        CHECK(m3(3, 1) == 9.0);
        CHECK(m3(3, 2) == 9.0);
        CHECK(m3(3, 3) == 9.0);

        m3 -= m2;
        CHECK(m3(0, 0) == 4.0);
        CHECK(m3(0, 1) == 3.0);
        CHECK(m3(0, 2) == 2.0);
        CHECK(m3(0, 3) == 1.0);
        CHECK(m3(1, 0) == 5.0);
        CHECK(m3(1, 1) == 4.0);
        CHECK(m3(1, 2) == 3.0);
        CHECK(m3(1, 3) == 2.0);
        CHECK(m3(2, 0) == 6.0);
        CHECK(m3(2, 1) == 5.0);
        CHECK(m3(2, 2) == 4.0);
        CHECK(m3(2, 3) == 3.0);
        CHECK(m3(3, 0) == 7.0);
        CHECK(m3(3, 1) == 6.0);
        CHECK(m3(3, 2) == 5.0);
        CHECK(m3(3, 3) == 4.0);
    }
}

TEST_CASE("Matrix 4x4 multiplication by scalar", "[math][matrix]")
{
    {
        f32 data[4][4] = {{2, 2, 2, 2}, {3, 3, 3, 3}, {4, 4, 4, 4}, {5, 5, 5, 5}};
        const Matrix4x4f m(data);

        Matrix4x4f m2 = m * 2.0f;
        CHECK(m2(0, 0) == 4.0f);
        CHECK(m2(0, 1) == 4.0f);
        CHECK(m2(0, 2) == 4.0f);
        CHECK(m2(0, 3) == 4.0f);
        CHECK(m2(1, 0) == 6.0f);
        CHECK(m2(1, 1) == 6.0f);
        CHECK(m2(1, 2) == 6.0f);
        CHECK(m2(1, 3) == 6.0f);
        CHECK(m2(2, 0) == 8.0f);
        CHECK(m2(2, 1) == 8.0f);
        CHECK(m2(2, 2) == 8.0f);
        CHECK(m2(2, 3) == 8.0f);
        CHECK(m2(3, 0) == 10.0f);
        CHECK(m2(3, 1) == 10.0f);
        CHECK(m2(3, 2) == 10.0f);
        CHECK(m2(3, 3) == 10.0f);

        m2 = m;
        CHECK(m2(0, 0) == 2.0f);
        CHECK(m2(0, 1) == 2.0f);
        CHECK(m2(0, 2) == 2.0f);
        CHECK(m2(0, 3) == 2.0f);
        CHECK(m2(1, 0) == 3.0f);
        CHECK(m2(1, 1) == 3.0f);
        CHECK(m2(1, 2) == 3.0f);
        CHECK(m2(1, 3) == 3.0f);
        CHECK(m2(2, 0) == 4.0f);
        CHECK(m2(2, 1) == 4.0f);
        CHECK(m2(2, 2) == 4.0f);
        CHECK(m2(2, 3) == 4.0f);
        CHECK(m2(3, 0) == 5.0f);
        CHECK(m2(3, 1) == 5.0f);
        CHECK(m2(3, 2) == 5.0f);
        CHECK(m2(3, 3) == 5.0f);

        m2 *= 2.0f;
        CHECK(m2(0, 0) == 4.0f);
        CHECK(m2(0, 1) == 4.0f);
        CHECK(m2(0, 2) == 4.0f);
        CHECK(m2(0, 3) == 4.0f);
        CHECK(m2(1, 0) == 6.0f);
        CHECK(m2(1, 1) == 6.0f);
        CHECK(m2(1, 2) == 6.0f);
        CHECK(m2(1, 3) == 6.0f);
        CHECK(m2(2, 0) == 8.0f);
        CHECK(m2(2, 1) == 8.0f);
        CHECK(m2(2, 2) == 8.0f);
        CHECK(m2(2, 3) == 8.0f);
        CHECK(m2(3, 0) == 10.0f);
        CHECK(m2(3, 1) == 10.0f);
        CHECK(m2(3, 2) == 10.0f);
        CHECK(m2(3, 3) == 10.0f);

        m2 = 2.0f * m;
        CHECK(m2(0, 0) == 4.0f);
        CHECK(m2(0, 1) == 4.0f);
        CHECK(m2(0, 2) == 4.0f);
        CHECK(m2(0, 3) == 4.0f);
        CHECK(m2(1, 0) == 6.0f);
        CHECK(m2(1, 1) == 6.0f);
        CHECK(m2(1, 2) == 6.0f);
        CHECK(m2(1, 3) == 6.0f);
        CHECK(m2(2, 0) == 8.0f);
        CHECK(m2(2, 1) == 8.0f);
        CHECK(m2(2, 2) == 8.0f);
        CHECK(m2(2, 3) == 8.0f);
        CHECK(m2(3, 0) == 10.0f);
        CHECK(m2(3, 1) == 10.0f);
        CHECK(m2(3, 2) == 10.0f);
        CHECK(m2(3, 3) == 10.0f);
    }
    SECTION("double")
    {
        f64 data[4][4] = {{2, 2, 2, 2}, {3, 3, 3, 3}, {4, 4, 4, 4}, {5, 5, 5, 5}};
        const Matrix4x4d m(data);

        Matrix4x4d m2 = m * 2.0;
        CHECK(m2(0, 0) == 4.0);
        CHECK(m2(0, 1) == 4.0);
        CHECK(m2(0, 2) == 4.0);
        CHECK(m2(0, 3) == 4.0);
        CHECK(m2(1, 0) == 6.0);
        CHECK(m2(1, 1) == 6.0);
        CHECK(m2(1, 2) == 6.0);
        CHECK(m2(1, 3) == 6.0);
        CHECK(m2(2, 0) == 8.0);
        CHECK(m2(2, 1) == 8.0);
        CHECK(m2(2, 2) == 8.0);
        CHECK(m2(2, 3) == 8.0);
        CHECK(m2(3, 0) == 10.0);
        CHECK(m2(3, 1) == 10.0);
        CHECK(m2(3, 2) == 10.0);
        CHECK(m2(3, 3) == 10.0);

        m2 = m;
        CHECK(m2(0, 0) == 2.0);
        CHECK(m2(0, 1) == 2.0);
        CHECK(m2(0, 2) == 2.0);
        CHECK(m2(0, 3) == 2.0);
        CHECK(m2(1, 0) == 3.0);
        CHECK(m2(1, 1) == 3.0);
        CHECK(m2(1, 2) == 3.0);
        CHECK(m2(1, 3) == 3.0);
        CHECK(m2(2, 0) == 4.0);
        CHECK(m2(2, 1) == 4.0);
        CHECK(m2(2, 2) == 4.0);
        CHECK(m2(2, 3) == 4.0);
        CHECK(m2(3, 0) == 5.0);
        CHECK(m2(3, 1) == 5.0);
        CHECK(m2(3, 2) == 5.0);
        CHECK(m2(3, 3) == 5.0);

        m2 *= 2.0;
        CHECK(m2(0, 0) == 4.0);
        CHECK(m2(0, 1) == 4.0);
        CHECK(m2(0, 2) == 4.0);
        CHECK(m2(0, 3) == 4.0);
        CHECK(m2(1, 0) == 6.0);
        CHECK(m2(1, 1) == 6.0);
        CHECK(m2(1, 2) == 6.0);
        CHECK(m2(1, 3) == 6.0);
        CHECK(m2(2, 0) == 8.0);
        CHECK(m2(2, 1) == 8.0);
        CHECK(m2(2, 2) == 8.0);
        CHECK(m2(2, 3) == 8.0);
        CHECK(m2(3, 0) == 10.0);
        CHECK(m2(3, 1) == 10.0);
        CHECK(m2(3, 2) == 10.0);
        CHECK(m2(3, 3) == 10.0);

        m2 = 2.0 * m;
        CHECK(m2(0, 0) == 4.0);
        CHECK(m2(0, 1) == 4.0);
        CHECK(m2(0, 2) == 4.0);
        CHECK(m2(0, 3) == 4.0);
        CHECK(m2(1, 0) == 6.0);
        CHECK(m2(1, 1) == 6.0);
        CHECK(m2(1, 2) == 6.0);
        CHECK(m2(1, 3) == 6.0);
        CHECK(m2(2, 0) == 8.0);
        CHECK(m2(2, 1) == 8.0);
        CHECK(m2(2, 2) == 8.0);
        CHECK(m2(2, 3) == 8.0);
        CHECK(m2(3, 0) == 10.0);
        CHECK(m2(3, 1) == 10.0);
        CHECK(m2(3, 2) == 10.0);
        CHECK(m2(3, 3) == 10.0);
    }
}

TEST_CASE("Matrix 4x4 division by scalar", "[math][matrix]")
{
    SECTION("float")
    {
        f32 data[4][4] = {{2, 2, 2, 2}, {4, 4, 4, 4}, {6, 6, 6, 6}, {8, 8, 8, 8}};
        const Matrix4x4f m(data);

        Matrix4x4f m2 = m / 2.0f;
        CHECK(m2(0, 0) == 1.0f);
        CHECK(m2(0, 1) == 1.0f);
        CHECK(m2(0, 2) == 1.0f);
        CHECK(m2(0, 3) == 1.0f);
        CHECK(m2(1, 0) == 2.0f);
        CHECK(m2(1, 1) == 2.0f);
        CHECK(m2(1, 2) == 2.0f);
        CHECK(m2(1, 3) == 2.0f);
        CHECK(m2(2, 0) == 3.0f);
        CHECK(m2(2, 1) == 3.0f);
        CHECK(m2(2, 2) == 3.0f);
        CHECK(m2(2, 3) == 3.0f);
        CHECK(m2(3, 0) == 4.0f);
        CHECK(m2(3, 1) == 4.0f);
        CHECK(m2(3, 2) == 4.0f);
        CHECK(m2(3, 3) == 4.0f);

        m2 = m;
        CHECK(m2(0, 0) == 2.0f);
        CHECK(m2(0, 1) == 2.0f);
        CHECK(m2(0, 2) == 2.0f);
        CHECK(m2(0, 3) == 2.0f);
        CHECK(m2(1, 0) == 4.0f);
        CHECK(m2(1, 1) == 4.0f);
        CHECK(m2(1, 2) == 4.0f);
        CHECK(m2(1, 3) == 4.0f);
        CHECK(m2(2, 0) == 6.0f);
        CHECK(m2(2, 1) == 6.0f);
        CHECK(m2(2, 2) == 6.0f);
        CHECK(m2(2, 3) == 6.0f);
        CHECK(m2(3, 0) == 8.0f);
        CHECK(m2(3, 1) == 8.0f);
        CHECK(m2(3, 2) == 8.0f);
        CHECK(m2(3, 3) == 8.0f);

        m2 /= 2.0f;
        CHECK(m2(0, 0) == 1.0f);
        CHECK(m2(0, 1) == 1.0f);
        CHECK(m2(0, 2) == 1.0f);
        CHECK(m2(0, 3) == 1.0f);
        CHECK(m2(1, 0) == 2.0f);
        CHECK(m2(1, 1) == 2.0f);
        CHECK(m2(1, 2) == 2.0f);
        CHECK(m2(1, 3) == 2.0f);
        CHECK(m2(2, 0) == 3.0f);
        CHECK(m2(2, 1) == 3.0f);
        CHECK(m2(2, 2) == 3.0f);
        CHECK(m2(2, 3) == 3.0f);
        CHECK(m2(3, 0) == 4.0f);
        CHECK(m2(3, 1) == 4.0f);
        CHECK(m2(3, 2) == 4.0f);
        CHECK(m2(3, 3) == 4.0f);
    }
    SECTION("double")
    {
        f64 data[4][4] = {{2, 2, 2, 2}, {4, 4, 4, 4}, {6, 6, 6, 6}, {8, 8, 8, 8}};
        const Matrix4x4d m(data);

        Matrix4x4d m2 = m / 2.0;
        CHECK(m2(0, 0) == 1.0);
        CHECK(m2(0, 1) == 1.0);
        CHECK(m2(0, 2) == 1.0);
        CHECK(m2(0, 3) == 1.0);
        CHECK(m2(1, 0) == 2.0);
        CHECK(m2(1, 1) == 2.0);
        CHECK(m2(1, 2) == 2.0);
        CHECK(m2(1, 3) == 2.0);
        CHECK(m2(2, 0) == 3.0);
        CHECK(m2(2, 1) == 3.0);
        CHECK(m2(2, 2) == 3.0);
        CHECK(m2(2, 3) == 3.0);
        CHECK(m2(3, 0) == 4.0);
        CHECK(m2(3, 1) == 4.0);
        CHECK(m2(3, 2) == 4.0);
        CHECK(m2(3, 3) == 4.0);

        m2 = m;
        CHECK(m2(0, 0) == 2.0);
        CHECK(m2(0, 1) == 2.0);
        CHECK(m2(0, 2) == 2.0);
        CHECK(m2(0, 3) == 2.0);
        CHECK(m2(1, 0) == 4.0);
        CHECK(m2(1, 1) == 4.0);
        CHECK(m2(1, 2) == 4.0);
        CHECK(m2(1, 3) == 4.0);
        CHECK(m2(2, 0) == 6.0);
        CHECK(m2(2, 1) == 6.0);
        CHECK(m2(2, 2) == 6.0);
        CHECK(m2(2, 3) == 6.0);
        CHECK(m2(3, 0) == 8.0);
        CHECK(m2(3, 1) == 8.0);
        CHECK(m2(3, 2) == 8.0);
        CHECK(m2(3, 3) == 8.0);

        m2 /= 2.0;
        CHECK(m2(0, 0) == 1.0);
        CHECK(m2(0, 1) == 1.0);
        CHECK(m2(0, 2) == 1.0);
        CHECK(m2(0, 3) == 1.0);
        CHECK(m2(1, 0) == 2.0);
        CHECK(m2(1, 1) == 2.0);
        CHECK(m2(1, 2) == 2.0);
        CHECK(m2(1, 3) == 2.0);
        CHECK(m2(2, 0) == 3.0);
        CHECK(m2(2, 1) == 3.0);
        CHECK(m2(2, 2) == 3.0);
        CHECK(m2(2, 3) == 3.0);
        CHECK(m2(3, 0) == 4.0);
        CHECK(m2(3, 1) == 4.0);
        CHECK(m2(3, 2) == 4.0);
        CHECK(m2(3, 3) == 4.0);
    }
}

TEST_CASE("Matrix 4x4 transpose operator", "[math][matrix]")
{
    SECTION("float")
    {
        f32 data[16] = {1.0f, 4.0f, -4.0f, 5.0f, 3.0f, -2.0f, 1.0f, 10.0f, 7.0f, 10.0f, -5.0f, -3.0f, -6.0f, -2.0f, -1.0f, 9.0f};
        const Matrix4x4f m1(data);

        const Matrix4x4f m2 = Transpose(m1);
        CHECK(m2(0, 0) == 1.0f);
        CHECK(m2(0, 1) == 3.0f);
        CHECK(m2(0, 2) == 7.0f);
        CHECK(m2(0, 3) == -6.0f);
        CHECK(m2(1, 0) == 4.0f);
        CHECK(m2(1, 1) == -2.0f);
        CHECK(m2(1, 2) == 10.0f);
        CHECK(m2(1, 3) == -2.0f);
        CHECK(m2(2, 0) == -4.0f);
        CHECK(m2(2, 1) == 1.0f);
        CHECK(m2(2, 2) == -5.0f);
        CHECK(m2(2, 3) == -1.0f);
        CHECK(m2(3, 0) == 5.0f);
        CHECK(m2(3, 1) == 10.0f);
        CHECK(m2(3, 2) == -3.0f);
        CHECK(m2(3, 3) == 9.0f);
    }
    SECTION("double")
    {
        f64 data[16] = {1.0, 4.0, -4.0, 5.0, 3.0, -2.0, 1.0, 10.0, 7.0, 10.0, -5.0, -3.0, -6.0, -2.0, -1.0, 9.0};
        const Matrix4x4d m1(data);

        const Matrix4x4d m2 = Transpose(m1);
        CHECK(m2(0, 0) == 1.0);
        CHECK(m2(0, 1) == 3.0);
        CHECK(m2(0, 2) == 7.0);
        CHECK(m2(0, 3) == -6.0);
        CHECK(m2(1, 0) == 4.0);
        CHECK(m2(1, 1) == -2.0);
        CHECK(m2(1, 2) == 10.0);
        CHECK(m2(1, 3) == -2.0);
        CHECK(m2(2, 0) == -4.0);
        CHECK(m2(2, 1) == 1.0);
        CHECK(m2(2, 2) == -5.0);
        CHECK(m2(2, 3) == -1.0);
        CHECK(m2(3, 0) == 5.0);
        CHECK(m2(3, 1) == 10.0);
        CHECK(m2(3, 2) == -3.0);
        CHECK(m2(3, 3) == 9.0);
    }
}

TEST_CASE("Matrix 4x4 inverse operator", "[math][matrix]")
{
    SECTION("float")
    {
        Matrix4x4f m1(1);
        m1.elements[0][3] = 5.0f;
        m1.elements[1][3] = 10.0f;
        m1.elements[2][3] = -2.0f;

        const Matrix4x4f m2 = Inverse(m1);
        CHECK(m2.elements[0][0] == 1.0f);
        CHECK(m2.elements[0][1] == 0.0f);
        CHECK(m2.elements[0][2] == 0.0f);
        CHECK(m2.elements[0][3] == -5.0f);
        CHECK(m2.elements[1][0] == 0.0f);
        CHECK(m2.elements[1][1] == 1.0f);
        CHECK(m2.elements[1][2] == 0.0f);
        CHECK(m2.elements[1][3] == -10.0f);
        CHECK(m2.elements[2][0] == 0.0f);
        CHECK(m2.elements[2][1] == 0.0f);
        CHECK(m2.elements[2][2] == 1.0f);
        CHECK(m2.elements[2][3] == 2.0f);
        CHECK(m2.elements[3][0] == 0.0f);
        CHECK(m2.elements[3][1] == 0.0f);
        CHECK(m2.elements[3][2] == 0.0f);
        CHECK(m2.elements[3][3] == 1.0f);
    }
    SECTION("double")
    {
        Matrix4x4d m1(1);
        m1.elements[0][3] = 5.0;
        m1.elements[1][3] = 10.0;
        m1.elements[2][3] = -2.0;

        const Matrix4x4d m2 = Inverse(m1);
        CHECK(m2.elements[0][0] == 1.0);
        CHECK(m2.elements[0][1] == 0.0);
        CHECK(m2.elements[0][2] == 0.0);
        CHECK(m2.elements[0][3] == -5.0);
        CHECK(m2.elements[1][0] == 0.0);
        CHECK(m2.elements[1][1] == 1.0);
        CHECK(m2.elements[1][2] == 0.0);
        CHECK(m2.elements[1][3] == -10.0);
        CHECK(m2.elements[2][0] == 0.0);
        CHECK(m2.elements[2][1] == 0.0);
        CHECK(m2.elements[2][2] == 1.0);
        CHECK(m2.elements[2][3] == 2.0);
        CHECK(m2.elements[3][0] == 0.0);
        CHECK(m2.elements[3][1] == 0.0);
        CHECK(m2.elements[3][2] == 0.0);
        CHECK(m2.elements[3][3] == 1.0);
    }
}

TEST_CASE("To Matrix3x3 and Matrix4x4", "[math][matrix]")
{
    SECTION("To Matrix3x3")
    {
        Matrix3x3f m1(5.0f);
        Matrix4x4f m2(5.0f);
        Matrix3x3f m3 = m2.ToMatrix3x3();
        CHECK(m1 == m3);
    }
    SECTION("To Matrix4x4")
    {
        Matrix4x4f m1(5.0f);
        Matrix3x3f m2(5.0f);
        m1(3, 3) = 1.0f;
        Matrix4x4f m3 = m2.ToMatrix4x4();
        CHECK(m1 == m3);
    }
}

TEST_CASE("Calculate cofactor", "[math][matrix]")
{
    const Matrix4x4f m = Matrix4x4f::FromRows({1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12}, {13, 14, 21, 27});
    const f32 cofactor_12 = Opal::Cofactor(m, 1, 2);
    REQUIRE(cofactor_12 == 88.0f);
    const f32 cofactor_33 = Opal::Cofactor(m, 2, 2);
    REQUIRE(cofactor_33 == -44.0f);

}