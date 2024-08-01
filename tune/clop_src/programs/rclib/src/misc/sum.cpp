////////////////////////////////////////////////////////////////////////////
//
// sum.cpp
//
// computes the sum of integers given as input
//
// Rémi Coulom
//
// July 1996
//
////////////////////////////////////////////////////////////////////////////
#include <iostream>

int main(void)
{
 int i;
 int Result = 0;

 for(;;)
 {
  cin >> i;
  if (cin.eof())
   break;
  Result += i;
 }

 cout << Result << '\n';

 return 0;
}
