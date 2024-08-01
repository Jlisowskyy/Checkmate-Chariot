/////////////////////////////////////////////////////////////////////////////
//
// COlivier.h
//
// Rémi Coulom
//
// April, 2010
//
/////////////////////////////////////////////////////////////////////////////
#ifndef COlivier_Declared
#define COlivier_Declared

#include "CArtificialProblem.h"

class COlivier: public CArtificialProblem // olivier
{
 private: ///////////////////////////////////////////////////////////////////
  const int Dimensions;
  const double Power;
  const double PMax;
  const double Scale;

 public: ////////////////////////////////////////////////////////////////////
  COlivier(int Dimensions, double Power, double PMax):
   Dimensions(Dimensions),
   Power(Power),
   PMax(PMax),
   Scale(std::pow(PMax, 2.0 / Power))
  {
  }

  int GetDimensions() const {return Dimensions;}

  double GetStrength(const double v[]) const
  {
   return GetProba(v); // ???
  }

  double GetProba(const double v[]) const
  {
   double r2 = 0;
   for (int i = Dimensions; --i >= 0;)
    r2 += v[i] * v[i] * Scale;
   const double r = std::sqrt(r2);
   double Result = PMax - std::pow(r, Power);
   if (Result < 0.0)
    Result = 0.0;
   return Result;
  }
};

#endif
