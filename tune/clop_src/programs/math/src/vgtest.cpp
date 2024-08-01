#include "CValueWithGradient.h"

#include <iostream>

int main()
{
 CValueWithGradient vgA(2);
 vgA.Value = 1;
 vgA.vGradient[0] = 2;
 vgA.vGradient[1] = 3;

 CValueWithGradient vgB(2);
 vgB.Value = 4;
 vgB.vGradient[0] = 5;
 vgB.vGradient[1] = 6;

 std::cout << "A = " << vgA << '\n';
 std::cout << "double(A) = " << double(vgA) << '\n';
 std::cout << "B = " << vgB << '\n';
 std::cout << "A + B = " << vgA + vgB << '\n';
 std::cout << "A - B = " << vgA - vgB << '\n';
 std::cout << "A * B = " << vgA * vgB << '\n';
 std::cout << "A / B = " << vgA / vgB << '\n';
 return 0;
}
