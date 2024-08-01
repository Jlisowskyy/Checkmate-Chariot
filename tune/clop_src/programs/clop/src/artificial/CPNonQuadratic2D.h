/////////////////////////////////////////////////////////////////////////////
//
// CPNonQuadratic2D.h
//
// Rémi Coulom
//
// February, 2009
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CPNonQuadratic2D_Declared
#define CPNonQuadratic2D_Declared

#include "CArtificialProblem.h"

class CPNonQuadratic2D: public CArtificialProblem // nq2d
{
 public: ////////////////////////////////////////////////////////////////////
  double GetStrength(const double v[]) const;
  void GetOptimalParameters(double v[]) const;
  int GetDimensions() const {return 2;}
};

#endif
