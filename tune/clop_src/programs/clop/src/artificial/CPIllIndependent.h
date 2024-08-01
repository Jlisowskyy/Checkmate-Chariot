/////////////////////////////////////////////////////////////////////////////
//
// CPIllIndependent.h
//
// Rémi Coulom
//
// August, 2011
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CPIllIndependent_Declared
#define CPIllIndependent_Declared

#include "CArtificialProblem.h"

class CPIllIndependent: public CArtificialProblem
{
 private: ///////////////////////////////////////////////////////////////////
  static double f(double x)
  {
   double y = x * x * x - x * x * x * x;
   if (y < 0)
   {
    y *= 0.01;
    if (y < -1.0)
     y = -1.0 + 0.0001 * (y + 1.0);
   }
   return y;
  }

 public: ////////////////////////////////////////////////////////////////////
  double GetStrength(const double v[]) const
  {
   const double x0 = 0.75 + 4.0 * (v[0] - 0.2);
   const double x1 = 0.75 + 0.2 * (v[1] - 0.2);
   return 10.0 * (f(x0) + f(x1)) - 1.0;
  }

  int GetDimensions() const
  {
   return 2;
  }

  void GetOptimalParameters(double v[]) const
  {
   v[0] = 0.2;
   v[1] = 0.2;
  }
};

#endif
