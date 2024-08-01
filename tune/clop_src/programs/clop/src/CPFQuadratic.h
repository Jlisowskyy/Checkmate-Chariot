/////////////////////////////////////////////////////////////////////////////
//
// CPFQuadratic.h
//
// Rémi Coulom
//
// December, 2008
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CPFQuadratic_Declared
#define CPFQuadratic_Declared

#include "CParametricFunction.h"

class CPFQuadratic: public CParametricFunction // pfq
{
 public: ////////////////////////////////////////////////////////////////////

  explicit CPFQuadratic(int Dimensions);

  //
  // Overrides of CParametricFunction
  //
  double GetValue(const double *vParam, const double *vx) const;
  bool GetMax(const double *vParam, double *vx) const;
  void GetMonomials(const double *vx, double *vMonomial) const;
  void GetGradient(const double *vParam, const double *vx, double *vG) const;

  //
  // Hessian (lower triangle)
  //
  void GetHessian(const double *vParam, double *vH) const;
};

#endif
