/////////////////////////////////////////////////////////////////////////////
//
// CPFConcave.h
//
// Rémi Coulom
//
// April, 2010
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CPFConcave_Declared
#define CPFConcave_Declared

#include <vector>

#include "CParametricFunction.h"

class CPFConcave: public CParametricFunction // pfg
{
 private: ///////////////////////////////////////////////////////////////////
  const int Resolution;
  std::vector<double> vCenter;

  double Basis(double Delta) const;

  //
  // Helper functions for the prior
  //
  static const double Epsilon;
  static double MakePositive(double x);
  static double MPDerivative(double y);
  static double MPSecondDerivative(double y);

 public: ////////////////////////////////////////////////////////////////////

  explicit CPFConcave(int Dimensions, int Resolution = 9);

  //
  // Overrides of CParametricFunction
  //
  void GetMonomials(const double *vx, double *vMonomial) const;
  void GetGradient(const double *vParam, const double *vx, double *vG) const;

  void GetPriorParam(double *vParam) const;
  double GetPrior(const double *vParam) const;
  void GetPriorGradient(const double *vParam, double *vGradient) const;
  void GetPriorHessian(const double *vParam, double *vHessian) const;
};

#endif
