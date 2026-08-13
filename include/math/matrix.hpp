#ifndef __MATRIX_HPP_INCLUDED__
#define __MATRIX_HPP_INCLUDED__

#include "vec3.hpp"

inline static constexpr int32_t MATRIX_SIZE = 3;

class c_proxy {
public:
    c_proxy(const double* array) : cp_array(array) {};

    double operator [] (size_t index) const
    {
        if(index >= MATRIX_SIZE) throw std::invalid_argument("Invalid index!");
        return cp_array[index];
    }

private:
    const double* cp_array;
};

class proxy {
public:
    proxy(double* array) : p_array(array) {};

    double& operator [] (size_t index) 
    {
        if(index >= MATRIX_SIZE) throw std::invalid_argument("Invalid index!");
        return p_array[index];
    }

private:
    double* p_array;
};

struct matrix {
public:

    double m_data[MATRIX_SIZE][MATRIX_SIZE] =
    {
        {1,0,0},
        {0,1,0},
        {0,0,1}
    };

public:

    friend matrix   operator * (const matrix& left, const matrix& right);
    friend vec3     operator * (const vec3& left, const matrix& right);

    c_proxy operator [] (size_t index) const
    {
        if(index >= MATRIX_SIZE) throw std::out_of_range("Invalid index!");
        return c_proxy(m_data[index]);
    }

    proxy operator [] (size_t index)
    {
        if(index >= MATRIX_SIZE) throw std::out_of_range("Invalid index!");
        return proxy(m_data[index]);
    }
};

matrix operator * (const matrix& left, const matrix& right);
vec3   operator * (const vec3& left, const matrix& right);

#endif