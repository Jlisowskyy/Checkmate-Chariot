/////////////////////////////////////////////////////////////////////////////
//
// CPRosenbrock.h
//
// Rémi Coulom
//
// March, 2010
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CPRosenbrock_Declared
#define CPRosenbrock_Declared

#include "CArtificialProblem.h"

class CPRosenbrock: public CArtificialProblem // rosenbrock
{
 private: ///////////////////////////////////////////////////////////////////
  static const double xMul;
  static const double yMul;
  static const double xOffset;
  static const double yOffset;
  static const double Scale;

 public: ////////////////////////////////////////////////////////////////////
  double GetStrength(const double v[]) const;
  int GetDimensions() const {return 2;}
  void GetOptimalParameters(double v[]) const;
};

#endif
