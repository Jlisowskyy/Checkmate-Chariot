/////////////////////////////////////////////////////////////////////////////
//
// CPNonQuadraticND.h
//
// Rémi Coulom
//
// March, 2009
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CPNonQuadraticND_Declared
#define CPNonQuadraticND_Declared

#include "CArtificialProblem.h"

class CPNonQuadraticND: public CArtificialProblem // nqnd
{
 private: ///////////////////////////////////////////////////////////////////
  const int Dimensions;

 public: ////////////////////////////////////////////////////////////////////
  explicit CPNonQuadraticND(int Dimensions = 1): Dimensions(Dimensions) {}
  double GetStrength(const double v[]) const;
  double GetOptimalProba() const;
  void GetOptimalParameters(double v[]) const;
  int GetDimensions() const {return Dimensions;}
};

#endif
