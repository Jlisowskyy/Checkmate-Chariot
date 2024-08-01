/////////////////////////////////////////////////////////////////////////////
//
// CPIllCorrelated.h
//
// Rémi Coulom
//
// August, 2011
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CPIllCorrelated_Declared
#define CPIllCorrelated_Declared

#include "CArtificialProblem.h"

class CPIllCorrelated: public CArtificialProblem
{
 private: ///////////////////////////////////////////////////////////////////
  static double f(double x)
  {
   return x * x * (x * (1.0 - x) - 1.0);
  }

 public: ////////////////////////////////////////////////////////////////////
  double GetStrength(const double v[]) const
  {
   const double a = v[0] + v[1] + 0.1;
   const double b = v[0] - v[1] + 0.9;
   return 0.2 * (f(10.0 * a) + f(b)) + 0.1;
  }

  int GetDimensions() const
  {
   return 2;
  }

  void GetOptimalParameters(double v[]) const
  {
   v[0] = -0.5;
   v[1] = 0.4;
  }
};

#endif
