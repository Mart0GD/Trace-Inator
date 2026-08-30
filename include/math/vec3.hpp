#ifndef __VEC3_HPP_INCLUDED__
#define __VEC3_HPP_INCLUDED__

#include "utils/defines.hpp"

#include <cmath>
#include <iostream>
#include <cstdint>
#include <random>

struct vec3 {
    fp x,y,z;

    vec3() : x(0), y(0), z(0) {};
    vec3(fp x, fp y, fp z) : x(x), y(y), z(z) {};

    fp length()         const { return std::sqrt(x * x + y * y + z * z); }
    fp length_pow2()    const { return x * x + y * y + z * z; }
    bool is_near_zero() const { return abs(x) < 1e-9 && abs(y) < 1e-9 && abs(z) < 1e-9; }
    bool is_zero() const { return x == 0 && y == 0 && z == 0; }

    fp    operator [] (const uint32_t index) const
    {
        ASSERT_OR_THROW(index < 3);
        if(index == 0) return x;
        if(index == 1) return y;
        return z;
    }

    vec3& operator += (const vec3& other) 
    {
        x += other.x;
        y += other.y;
        z += other.z;

        return *this;
    }

    vec3& operator *= (const vec3& other) 
    {
        x *= other.x;
        y *= other.y;
        z *= other.z;

        return *this;
    }

    vec3& operator *= (double scalar) 
    {
        x *= scalar;
        y *= scalar;
        z *= scalar;

        return *this;
    }

    vec3& operator /= (double scalar) 
    {
        return *this *= 1 / scalar;
    }

    friend std::ostream& operator << (std::ostream& os, const vec3& vec);
};

// -- OPERATIONS --

inline vec3 operator - (const vec3& vec) { return vec3(-vec.x, -vec.y, -vec.z); }

inline vec3 operator + (const vec3& left, const vec3& right)
{
    return vec3(left.x + right.x, left.y + right.y, left.z + right.z);
}

inline vec3 operator - (const vec3& left, const vec3& right)
{
    return vec3(left.x - right.x, left.y - right.y, left.z - right.z);
}

inline vec3 operator * (const vec3& left, const vec3& right)
{
    return vec3(left.x * right.x, left.y * right.y, left.z * right.z);
}

inline vec3 operator * (const vec3& vec, double scalar)
{
    vec3 copy(vec);
    return copy *= scalar;
}

inline vec3 operator * (double scalar, const vec3& vec)
{
    return vec * scalar;
}

inline vec3 operator / (const vec3& vec, double scalar)
{
    return vec * (1 / scalar);
}

inline vec3 operator / (double scalar, const vec3& vec)
{
    return vec3(scalar / vec.x, scalar / vec.y, scalar / vec.z);
}

inline fp dot(const vec3& vec1, const vec3& vec2) {
    return vec1.x * vec2.x
        +  vec1.y * vec2.y
        +  vec1.z * vec2.z;
}

inline vec3 cross(const vec3& vec1, const vec3& vec2) {
    return vec3(vec1.y * vec2.z - vec1.z * vec2.y,
                vec1.z * vec2.x - vec1.x * vec2.z, 
                vec1.x * vec2.y - vec1.y * vec2.x);
}

inline vec3 unit_vector(const vec3& v) {
    return v / v.length();
}

inline std::ostream& operator << (std::ostream& os, const vec3& vec)
{
    os << "(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
    return os;
}

// https://math.stackexchange.com/a/1585996
inline vec3 random_unit_vector(std::mt19937& gen)
{
    static thread_local std::normal_distribution<fp> normal(0, 1);
    return unit_vector(vec3(normal(gen), normal(gen), normal(gen)));
}

#endif