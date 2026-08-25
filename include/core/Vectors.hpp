#pragma once

#include <stdexcept>

struct Vec4 {
    float x, y, z, w;

    Vec4()
        : x(0), y(0), z(0), w(0)
    {
    }

    Vec4(float x, float y, float z, float w)
        : x(x), y(y), z(z), w(w)
    {
    }

    Vec4 operator-(const Vec4& other) const
    {
        return Vec4(
            x - other.x,
            y - other.y,
            z - other.z,
            w - other.w
        );
    }

    Vec4 operator+(const Vec4& other) const
    {
        return Vec4(
            x + other.x,
            y + other.y,
            z + other.z,
            w + other.w
        );
    }

    Vec4 operator/(float scalar) const
    {
        return Vec4(
            x / scalar,
            y / scalar,
            z / scalar,
            w / scalar
        );
    }

    Vec4 operator*(float scalar) const
    {
        return Vec4(
            x * scalar,
            y * scalar,
            z * scalar,
            w * scalar
        );
    }

    float& operator[](int index)
    {
        if (index == 0) return x;
        if (index == 1) return y;
        if (index == 2) return z;
        if (index == 3) return w;

        throw std::out_of_range("Vec4 index out of range");
    }

    const float& operator[](int index) const
    {
        if (index == 0) return x;
        if (index == 1) return y;
        if (index == 2) return z;
        if (index == 3) return w;

        throw std::out_of_range("Vec4 index out of range");
    }

};

struct Vec3 {
    float x, y, z;

    Vec3()
        : x(0), y(0), z(0)
    {
    }

    Vec3(float x, float y, float z)
        : x(x), y(y), z(z)
    {
    }

    Vec3 operator-(const Vec3& other) const
    {
        return Vec3(
            x - other.x,
            y - other.y,
            z - other.z
        );
    }

    Vec3 operator+(const Vec3& other) const
    {
        return Vec3(
            x + other.x,
            y + other.y,
            z + other.z
        );
    }

    Vec3 operator/(float scalar) const
    {
        return Vec3(
            x / scalar,
            y / scalar,
            z / scalar
        );
    }

    Vec3 operator*(float scalar) const
    {
        return Vec3(
            x * scalar,
            y * scalar,
            z * scalar
        );
    }

    float& operator[](int index)
    {
        if (index == 0) return x;
        if (index == 1) return y;
        if (index == 2) return z;

        throw std::out_of_range("Vec3 index out of range");
    }

    const float& operator[](int index) const
    {
        if (index == 0) return x;
        if (index == 1) return y;
        if (index == 2) return z;

        throw std::out_of_range("Vec3 index out of range");
    }

};