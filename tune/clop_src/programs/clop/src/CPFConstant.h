/////////////////////////////////////////////////////////////////////////////
//
// CPFConstant.h
//
// Rémi Coulom
//
// May, 2010
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CPFConstant_Declared
#define CPFConstant_Declared

#include "CParametricFunction.h"

class CPFConstant: public CParametricFunction // pfc
{
 public: ////////////////////////////////////////////////////////////////////

  explicit CPFConstant(int Dimensions): CParametricFunction(Dimensions, 1) {}

  //
  // Overrides of CParametricFunction
  //
  double GetValue(const double *vParam, const double *vx) const;
  bool GetMax(const double *vParam, double *vx) const;
  void GetMonomials(const double *vx, double *vMonomial) const;
  void GetGradient(const double *vParam, const double *vx, double *vG) const;
};

#endif
