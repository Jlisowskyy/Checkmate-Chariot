/////////////////////////////////////////////////////////////////////////////
//
// CPFCubic.h
//
// Rémi Coulom
//
// February, 2010
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CPFCubic_Declared
#define CPFCubic_Declared

#include "CParametricFunction.h"

class CPFCubic: public CParametricFunction // pfc
{
 public: ////////////////////////////////////////////////////////////////////

  explicit CPFCubic(int Dimensions);

  //
  // Overrides of CParametricFunction
  //
  double GetValue(const double *vParam, const double *vx) const;
  bool GetMax(const double *vParam, double *vx) const;
  void GetMonomials(const double *vx, double *vMonomial) const;
  void GetGradient(const double *vParam, const double *vx, double *vG) const;
};

#endif
