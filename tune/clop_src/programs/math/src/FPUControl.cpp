#include <float.h>
#include <iostream>
#include <cmath>

float SquareRoots()
{
 float x = 0.0;
 for (float y = 1; y < 2; y += 0.001)
  x += std::sqrt(y);
 return x;
 return 0;
}

float Divisions()
{
 float x = 1.0;
 for (float y = 1; y < 2; y += 0.001)
  x /= y;
 return x;
}

int main()
{
 unsigned orig_cw;
 orig_cw = _controlfp(0,0);

 //_controlfp(_PC_24, MCW_PC);
 //_controlfp(_PC_53, MCW_PC);
 _controlfp(_PC_64, MCW_PC);

 std::cout << "Divisions() = " << Divisions() << '\n';
 float y = 0;
 for (int i = 100000; --i >= 0;)
  y += SquareRoots();
 std::cout << "y = " << y << '\n';

 _controlfp(orig_cw, MCW_PC);

 return 0;
}
