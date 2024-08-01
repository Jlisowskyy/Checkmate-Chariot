/////////////////////////////////////////////////////////////////////////////
//
// CPFGaussian.h
//
// Rémi Coulom
//
// April, 2010
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CPFGaussian_Declared
#define CPFGaussian_Declared

#include <vector>

#include "CParametricFunction.h"

class CPFGaussian: public CParametricFunction // pfg
{
 private: ///////////////////////////////////////////////////////////////////
  const int Resolution;
  std::vector<double> vCenter;
  const double ScaleInverse;

  double g(double x) const;

 public: ////////////////////////////////////////////////////////////////////

  explicit CPFGaussian(int Dimensions, int Resolution = 9);

  //
  // Overrides of CParametricFunction
  //
  double GetValue(const double *vParam, const double *vx) const;
  void GetMonomials(const double *vx, double *vMonomial) const;
  void GetGradient(const double *vParam, const double *vx, double *vG) const;
};

#endif
