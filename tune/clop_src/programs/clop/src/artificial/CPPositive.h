/////////////////////////////////////////////////////////////////////////////
//
// CPPositive.h
//
// Rémi Coulom
//
// November, 2009
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CPPositive_Declared
#define CPPositive_Declared

#include "CArtificialProblem.h"
#include <cmath>

class CPPositive: public CArtificialProblem // disc
{
 private: ///////////////////////////////////////////////////////////////////
  const double a;
  const double b;
  const double c;

 public: ////////////////////////////////////////////////////////////////////
  CPPositive():
   a(-40),
   b(-3.6 + a * 1.6),
   c(2.24 + 0.8 * b - 0.64 * a)
  {}

  double GetStrength(const double v[]) const
  {
   double x = v[0];
   double y;
   if (x > -0.8)
    y = (1 - x) * (1 - x) - 1;
   else
    y = a * x * x + b * x + c;
   return y;
  }

  void GetOptimalParameters(double v[]) const
  {
   v[0] = -b / (2 * a);
  }

  int GetDimensions() const {return 1;}
};

#endif
