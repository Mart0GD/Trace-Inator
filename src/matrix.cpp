#include "math/matrix.hpp"

matrix operator * (const matrix& left, const matrix& right)
{
    matrix res;

    for (int i = 0; i < MATRIX_SIZE; i++)
    {
        for (int j = 0; j < MATRIX_SIZE; j++)
        {
            fp sum = 0;
            for (int x = 0; x < MATRIX_SIZE; x++)
            {
                sum += left[i][x] * right[x][j];
            }
            res[i][j] = sum;
        }
    }
    
    return res;
}

vec3   operator * (const vec3& left, const matrix& right)
{
    vec3 res =
    {
        left.x * right[0][0] +
        left.y * right[1][0] +
        left.z * right[2][0],

        left.x * right[0][1] +
        left.y * right[1][1] +
        left.z * right[2][1],

        left.x * right[0][2] +
        left.y * right[1][2] +
        left.z * right[2][2],
    };

    return res;
}
