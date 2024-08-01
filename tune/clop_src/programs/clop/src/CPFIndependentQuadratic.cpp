/////////////////////////////////////////////////////////////////////////////
//
// CPFIndependentQuadratic.cpp
//
// Rémi Coulom
//
// April, 2010
//
/////////////////////////////////////////////////////////////////////////////
#include "CPFIndependentQuadratic.h"

/////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////
CPFIndependentQuadratic::CPFIndependentQuadratic(int Dimensions):
 CParametricFunction(Dimensions, 2 * Dimensions + 1)
{
}

/////////////////////////////////////////////////////////////////////////////
// Compute value. vx is an augmented vector
/////////////////////////////////////////////////////////////////////////////
double CPFIndependentQuadratic::GetValue(const double *vParam,
                                         const double *vx) const
{
 double Result = vParam[0];

 for (int i = Dimensions; --i >= 0;)
 {
  double x = vx[i];
  double x2 = x * x;
  Result += x * vParam[i + 1] + x2 * vParam[i + 1 + Dimensions];
 }

 return Result;
}

/////////////////////////////////////////////////////////////////////////////
// Find input parameters that maximize output
/////////////////////////////////////////////////////////////////////////////
bool CPFIndependentQuadratic::GetMax(const double * restrict vParam,
                                     double * restrict vx) const
{
 for (int i = Dimensions; --i >= 0;)
 {
  double a = vParam[i + 1 + Dimensions];
  if (a >= 0)
   return false;
  vx[i] = -0.5 * vParam[i + 1] / a;
 }

 for (int i = Dimensions; --i >= 0;)
  if (vx[i] < -1)
   vx[i] = -1;
  else if (vx[i] > 1)
   vx[i] = 1;

 return true;
}

/////////////////////////////////////////////////////////////////////////////
// Compute monomials
/////////////////////////////////////////////////////////////////////////////
void CPFIndependentQuadratic::GetMonomials(const double * restrict vx,
                                           double * restrict vMonomial) const
{
 vMonomial[0] = 1.0;

 for (int i = Dimensions; --i >= 0;)
 {
  double x = vx[i];
  double x2 = x * x;
  vMonomial[i + 1] = x;
  vMonomial[i + 1 + Dimensions] = x2;
 }
}

/////////////////////////////////////////////////////////////////////////////
// Gradient
/////////////////////////////////////////////////////////////////////////////
void CPFIndependentQuadratic::GetGradient(const double * restrict vParam,
                                          const double * restrict vx,
                                          double * restrict vG) const
{
 for (int i = Dimensions; --i >= 0;)
  vG[i] = vParam[i + 1] + 2.0 * vParam[i + 1 + Dimensions] * vx[i];
}
