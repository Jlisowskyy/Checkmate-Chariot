/////////////////////////////////////////////////////////////////////////////
//
// CPComplicated1D.h
//
// Rémi Coulom
//
// November, 2008
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CPComplicated1D_Declared
#define CPComplicated1D_Declared

#include "CArtificialProblem.h"
#include <cmath>

class CPComplicated1D: public CArtificialProblem // c1d
{
 public: ////////////////////////////////////////////////////////////////////

  double GetStrength(const double v[]) const
  {
   if (v[0] < -0.8)
    return -2.0;
   else if (v[0] <= -0.3)
    return -2.0 + 3.0 * (v[0] + 0.8) / (-0.3 + 0.8);
   else if (v[0] < 0.8)
    return -1.0 * (v[0] + 0.3) / (0.8 + 0.3);
   else
    return -2.0;
  }

  void GetOptimalParameters(double v[]) const
  {
   v[0] = -0.3;
  }

  int GetDimensions() const {return 1;}
};

#endif
