/////////////////////////////////////////////////////////////////////////////
//
// CPFIndependentQuadratic.h
//
// Rémi Coulom
//
// April, 2010
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CPFIndependentQuadratic_Declared
#define CPFIndependentQuadratic_Declared

#include "CParametricFunction.h"

class CPFIndependentQuadratic: public CParametricFunction // iq
{
 public: ////////////////////////////////////////////////////////////////////

  explicit CPFIndependentQuadratic(int Dimensions);

  //
  // Overrides of CParametricFunction
  //
  double GetValue(const double *vParam, const double *vx) const;
  bool GetMax(const double *vParam, double *vx) const;
  void GetMonomials(const double *vx, double *vMonomial) const;
  void GetGradient(const double *vParam, const double *vx, double *vG) const;
};

#endif
