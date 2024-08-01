/////////////////////////////////////////////////////////////////////////////
//
// CPFGaussian.cpp
//
// Rémi Coulom
//
// April, 2010
//
/////////////////////////////////////////////////////////////////////////////
#include "CPFGaussian.h"

#include <cmath>

/////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////
CPFGaussian::CPFGaussian(int Dimensions, int Resolution):
 CParametricFunction(Dimensions, Dimensions * Resolution),
 Resolution(Resolution),
 vCenter(Resolution),
 ScaleInverse(double(Resolution - 1) / 2.0)
{
 for (int i = Resolution; --i >= 0;)
  vCenter[i] = -1.0 + 2 * double(i) / double(Resolution - 1);
}

/////////////////////////////////////////////////////////////////////////////
// Gaussian basis function
/////////////////////////////////////////////////////////////////////////////
double CPFGaussian::g(double x) const
{
 double d = x * ScaleInverse;
 return std::exp(-d * d);
}

/////////////////////////////////////////////////////////////////////////////
// Compute value. vx is an augmented vector
/////////////////////////////////////////////////////////////////////////////
double CPFGaussian::GetValue(const double *vParam,
                             const double *vx) const
{
 double Result = 0;

 for (int i = Dimensions, p = Parameters; --i >= 0;)
  for (int j = Resolution; --j >= 0;)
   Result += vParam[--p] * g(vx[i] - vCenter[j]);

 return Result;
}

/////////////////////////////////////////////////////////////////////////////
// Compute monomials
/////////////////////////////////////////////////////////////////////////////
void CPFGaussian::GetMonomials(const double * restrict vx,
                               double * restrict vMonomial) const
{
 for (int i = Dimensions, p = Parameters; --i >= 0;)
  for (int j = Resolution; --j >= 0;)
   vMonomial[--p] = g(vx[i] - vCenter[j]);
}

/////////////////////////////////////////////////////////////////////////////
// Gradient
/////////////////////////////////////////////////////////////////////////////
void CPFGaussian::GetGradient(const double * restrict vParam,
                              const double * restrict vx,
                              double * restrict vG) const
{
 for (int i = Dimensions, p = Parameters; --i >= 0;)
 {
  vG[i] = 0.0;
  for (int j = Resolution; --j >= 0;)
  {
   double Delta = vx[i] - vCenter[j];
   vG[i] -= 2.0 * vParam[--p] * ScaleInverse * ScaleInverse * Delta * g(Delta);
  }
 }
}
