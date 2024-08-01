/////////////////////////////////////////////////////////////////////////////
//
// CPSinExp.h
//
// Rémi Coulom
//
// June, 2010
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CPSinExp_Declared
#define CPSinExp_Declared

#include "CArtificialProblem.h"
#include <cmath>

class CPSinExp: public CArtificialProblem
{
 private: ///////////////////////////////////////////////////////////////////
  const double Freq;
  const double Offset;
  const double Decay;

 public: ////////////////////////////////////////////////////////////////////
  CPSinExp(double Freq = 2.0, double Offset = 0.7, double Decay = -3.0):
   Freq(Freq),
   Offset(Offset),
   Decay(Decay)
  {}

  double GetStrength(const double v[]) const
  {
   double x = v[0];
   return 2.0 * std::sin(Freq * (x + Offset)) * std::exp(Decay * (x + 1.0));
  }

  void GetOptimalParameters(double v[]) const
  {
   v[0] = std::atan(-Freq / Decay) / Freq - Offset;
  }

  int GetDimensions() const {return 1;}
};

#endif
