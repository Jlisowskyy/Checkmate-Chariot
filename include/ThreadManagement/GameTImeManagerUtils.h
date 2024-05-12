//
// Created by wookie on 5/12/24.
//

#ifndef CHECKMATE_CHARIOT_GAMETIMEMANAGERUTILS_H
#define CHECKMATE_CHARIOT_GAMETIMEMANAGERUTILS_H
#include <limits>

/*
 * Solution from: https://stackoverflow.com/questions/8622256/in-c11-is-sqrt-defined-as-constexpr
 * The Newton-Raphson method is ok, since this is a constexpr function, it will be evaluated at compile time.
 * (We don't need to worry about the performance of this function at runtime)
 * In my usage I also don't worry that much about the precision of the result
 */

namespace ConstexprMath
{
    /// <summary>
    /// Calculate the square root of a number using the Newton-Raphson method
    /// </summary>
    double constexpr sqrtNewtonRaphson(double x, double curr, double prev)
    {
        return curr == prev ? curr : sqrtNewtonRaphson(x, 0.5 * (curr + x / curr), curr);
    }

    /// <summary>
    /// Approx the square root of a number using the Newton-Raphson method as a constexpr function
    /// </summary>
    double constexpr sqrt(double x)
    {
        return x >= 0 && x < std::numeric_limits<double>::infinity() ? sqrtNewtonRaphson(x, x, 0) : std::numeric_limits<double>::quiet_NaN();
    }
}

#endif //CHECKMATE_CHARIOT_GAMETIMEMANAGERUTILS_H
