/////////////////////////////////////////////////////////////////////////////
//
// CPQuadratic2D.h
//
// Rémi Coulom
//
// October, 2008
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CPQuadratic2D_Declared
#define CPQuadratic2D_Declared

#include "CArtificialProblem.h"

class CPQuadratic2D: public CArtificialProblem // q2d
{
 private:
  static const double x0;

 public: ////////////////////////////////////////////////////////////////////
  double GetStrength(const double v[]) const;
  int GetDimensions() const {return 2;}
  void GetOptimalParameters(double v[]) const;
};

#endif
