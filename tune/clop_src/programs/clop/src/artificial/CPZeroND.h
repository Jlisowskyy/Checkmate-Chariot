/////////////////////////////////////////////////////////////////////////////
//
// CPZeroND.h
//
// Rémi Coulom
//
// March, 2009
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CPZeroND_Declared
#define CPZeroND_Declared

#include "CArtificialProblem.h"

class CPZeroND: public CArtificialProblem // znd
{
 private: ///////////////////////////////////////////////////////////////////
  const int Dimensions;

 public: ////////////////////////////////////////////////////////////////////
  CPZeroND(int Dimensions): Dimensions(Dimensions) {}

  double GetStrength(const double v[]) const {return 0.0;}
  double GetProba(const double *v) const {return 0.0;}
  int GetDimensions() const {return Dimensions;}
};

#endif
