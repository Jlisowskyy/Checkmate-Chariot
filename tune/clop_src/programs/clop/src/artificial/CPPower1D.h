/////////////////////////////////////////////////////////////////////////////
//
// CPPower1D.h
//
// Rémi Coulom
//
// July, 2011
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CPPower1D_Declared
#define CPPower1D_Declared

#include "CArtificialProblem.h"
#include <cmath>

class CPPower1D: public CArtificialProblem // p1d
{
 private: ///////////////////////////////////////////////////////////////////
  const double SquareMagnitude;
  const double Power;

 public: ////////////////////////////////////////////////////////////////////
  CPPower1D(double SquareMagnitude = 0.2, double Power = 20.0):
   SquareMagnitude(SquareMagnitude),
   Power(Power)
  {
  }

  double GetStrength(const double v[]) const
  {
   const double xx = (v[0] + 1.0) * 0.5;
   return SquareMagnitude * xx * xx - std::pow(xx, Power);
  }

  int GetDimensions() const {return 1;}

  void GetOptimalParameters(double v[]) const
  {
   v[0] = 2 * std::pow(2 * SquareMagnitude / Power, 1 / (Power - 2)) - 1;
  }
};

#endif
