/////////////////////////////////////////////////////////////////////////////
//
// CPAbsND.h
//
// Rémi Coulom
//
// March, 2009
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CPAbsND_Declared
#define CPAbsND_Declared

#include "CArtificialProblem.h"
#include <cmath>

class CPAbsND: public CArtificialProblem // and
{
 private: ///////////////////////////////////////////////////////////////////
  const int Dimensions;
  const double Scale;
  const double BestParam;

 public: ////////////////////////////////////////////////////////////////////
  CPAbsND(int Dimensions, double Scale):
   Dimensions(Dimensions),
   Scale(Scale),
   BestParam(-0.3)
  {}

  double GetStrength(const double v[]) const
  {
   double s = 0.0;
   for (int i = Dimensions; --i >= 0;)
    s -= std::fabs((v[i] - BestParam) * Scale);
   return s;
  }

  void GetOptimalParameters(double v[]) const
  {
   for (int i = Dimensions; --i >= 0;)
    v[i] = BestParam;
  }

  int GetDimensions() const {return Dimensions;}
};

#endif
