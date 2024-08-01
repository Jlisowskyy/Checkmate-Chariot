/////////////////////////////////////////////////////////////////////////////
//
// CPFish1D.h
//
// Rémi Coulom
//
// December, 2009
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CPFish1D_Declared
#define CPFish1D_Declared

#include "CArtificialProblem.h"
#include "CLogistic.h"
#include <cmath>

class CPFish1D: public CArtificialProblem // f1d
{
 private: ///////////////////////////////////////////////////////////////////
  const double a;

 public: ////////////////////////////////////////////////////////////////////
  CPFish1D(double a = 0.1): a(a) {}

  double GetProba(const double v[]) const
  {
   double x = (v[0] + 1) * 0.5;
   double b = 3.6 * x * (1 - x);
   double c = 1 - std::abs(a - x) / a;

   double y;
   if (b > c)
    y = b;
   else
    y = c;

   const double epsilon = 0.01;
   return epsilon + (1 - 2 * epsilon) * y;
  }

  double GetStrength(const double v[]) const
  {
   return CLogistic::Inverse(GetProba(v));
  }

  void GetOptimalParameters(double v[]) const
  {
   v[0] = a * 2 - 1;
  }

  int GetDimensions() const {return 1;}
};

#endif
