/////////////////////////////////////////////////////////////////////////////
//
// CPDiscontinuous.h
//
// Rémi Coulom
//
// November, 2009
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CPDiscontinuous_Declared
#define CPDiscontinuous_Declared

#include "CArtificialProblem.h"
#include <cmath>

class CPDiscontinuous: public CArtificialProblem // disc
{
 public: ////////////////////////////////////////////////////////////////////
  double GetStrength(const double v[]) const
  {
   double x = v[0] * 2 + 1.0;
   return int(2.0 * std::log(2.0 * x + 2.1) - 2.0 * x - 0.35);
  }

  void GetOptimalParameters(double v[]) const
  {
   v[0] = -0.525;
  }

  int GetDimensions() const {return 1;}
};

#endif
