/////////////////////////////////////////////////////////////////////////////
//
// CPLog1D.h
//
// Rémi Coulom
//
// October, 2008
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CPLog1D_Declared
#define CPLog1D_Declared

#include "CArtificialProblem.h"
#include <cmath>

class CPLog1D: public CArtificialProblem // l1d
{
 public: ////////////////////////////////////////////////////////////////////
  double GetStrength(const double v[]) const
  {
   double x = v[0] * 2 + 1.0;
   return 2.0 * std::log(2.0 * x + 2.1) - 2.0 * x - 1.0;
  }

  void GetOptimalParameters(double v[]) const
  {
   v[0] = -0.525;
  }

  int GetDimensions() const {return 1;}
};

#endif
