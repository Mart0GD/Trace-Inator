#ifndef __VEC3_HPP_INCLUDED__
#define __VEC3_HPP_INCLUDED__

#include <cmath>
#include <iostream>

struct vec3 {
    double x,y,z;

    vec3() : x(0), y(0), z(0) {};
    vec3(double x, double y, double z) : x(x), y(y), z(z) {};

    double length()         const { return std::sqrt(x * x + y * y + z * z); }
    double length_pow2()    const { return x * x + y * y + z * z; }

    vec3& operator += (const vec3& other) 
    {
        x += other.x;
        y += other.y;
        z += other.z;

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

typedef vec3 point3D;

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


inline double dot(const vec3& vec1, const vec3& vec2) {
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

inline double dist(const point3D& p1, const point3D& p2)
{
    return sqrt
    (
        (p2.x - p1.x) * (p2.x - p1.x) + 
        (p2.y - p1.y) * (p2.y - p1.y) +
        (p2.z - p1.z) * (p2.z - p1.z) 
    );
}


inline std::ostream& operator << (std::ostream& os, const vec3& vec)
{
    os << "(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
    return os;
}


#endif