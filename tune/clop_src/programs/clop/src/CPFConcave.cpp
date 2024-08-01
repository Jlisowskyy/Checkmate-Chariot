/////////////////////////////////////////////////////////////////////////////
//
// CPFConcave.cpp
//
// Rémi Coulom
//
// April, 2010
//
/////////////////////////////////////////////////////////////////////////////
#include "CPFConcave.h"

#include <cmath>

/////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////
CPFConcave::CPFConcave(int Dimensions, int Resolution):
 CParametricFunction(Dimensions, 1 + Dimensions * Resolution * 2),
 Resolution(Resolution),
 vCenter(Resolution)
{
 for (int i = Resolution; --i >= 0;)
  vCenter[i] = -1.0 + 2 * double(i + 1) / double(Resolution + 1);
 SetPriorStrength(1e-2);
}

/////////////////////////////////////////////////////////////////////////////
// Basis function
/////////////////////////////////////////////////////////////////////////////
double CPFConcave::Basis(double Delta) const
{
 if (Delta < 0)
  return 0.0;
 else
  return -Delta;
}

/////////////////////////////////////////////////////////////////////////////
// Compute monomials
/////////////////////////////////////////////////////////////////////////////
void CPFConcave::GetMonomials(const double * restrict vx,
                              double * restrict vMonomial) const
{
 for (int i = Dimensions, p = Parameters; --i >= 0;)
  for (int j = Resolution; --j >= 0;)
  {
   double c = vCenter[j];
   double Delta = vx[i] - c;
   vMonomial[--p] = Basis(Delta) + (1.0 - c) * (1.0 - c) * 0.25;
   vMonomial[--p] = Basis(-Delta) + (c + 1.0) * (c + 1.0) * 0.25;
  }

 vMonomial[0] = 1.0;
}

/////////////////////////////////////////////////////////////////////////////
// Gradient
/////////////////////////////////////////////////////////////////////////////
void CPFConcave::GetGradient(const double * restrict vParam,
                             const double * restrict vx,
                             double * restrict vG) const
{
 for (int i = Dimensions, p = Parameters; --i >= 0;)
 {
  vG[i] = 0.0;
  for (int j = Resolution; --j >= 0;)
  {
   double Delta = vx[i] - vCenter[j];
   --p;
   if (Delta > 0)
    vG[i] -= vParam[p];
   --p;
   if (Delta < 0)
    vG[i] += vParam[p];
  }
 }
}

/////////////////////////////////////////////////////////////////////////////
// Prior parameters
/////////////////////////////////////////////////////////////////////////////
void CPFConcave::GetPriorParam(double *vParam) const
{
 for (int p = Parameters; --p > 0;)
  vParam[p] = 1.0;
 vParam[0] = 0.0;
}

/////////////////////////////////////////////////////////////////////////////
// Helper function to make sure log(p) is not -infinity
/////////////////////////////////////////////////////////////////////////////
const double CPFConcave::Epsilon = 1e-100;

double CPFConcave::MakePositive(double x)
{
 if (x < Epsilon)
  return Epsilon * std::exp(x - Epsilon);
 else
  return x;
}

double CPFConcave::MPDerivative(double y)
{
 if (y < Epsilon)
  return y;
 else
  return 1.0;
}

double CPFConcave::MPSecondDerivative(double y)
{
 if (y < Epsilon)
  return y;
 else
  return 0.0;
}

/////////////////////////////////////////////////////////////////////////////
// Prior
/////////////////////////////////////////////////////////////////////////////
double CPFConcave::GetPrior(const double *vParam) const
{
 double Result = 0.0;

 for (int p = Parameters; --p > 0;)
 {
  double x = MakePositive(vParam[p]);
  Result += GetPriorStrength() * (std::log(x) - x);
 }

 Result -= GetPriorStrength() * vParam[0] * vParam[0] * 0.5;

 return Result;
}

/////////////////////////////////////////////////////////////////////////////
// Prior gradient
/////////////////////////////////////////////////////////////////////////////
void CPFConcave::GetPriorGradient(const double * restrict vParam,
                                  double * restrict vGradient) const
{
 for (int p = Parameters; --p > 0;)
 {
  double x = MakePositive(vParam[p]);
  vGradient[p] = GetPriorStrength() * (1.0 / x - 1.0) * MPDerivative(x);
 }

 vGradient[0] = -GetPriorStrength() * vParam[0];
}

/////////////////////////////////////////////////////////////////////////////
// Prior Hessian
/////////////////////////////////////////////////////////////////////////////
void CPFConcave::GetPriorHessian(const double * restrict vParam,
                                 double * restrict vHessian) const
{
 for (int i = Parameters * Parameters; --i >= 0;)
  vHessian[i] = 0.0;

 for (int p = Parameters; --p > 0;)
 {
  double x = MakePositive(vParam[p]);
  double xp = MPDerivative(x);
  double xpp = MPSecondDerivative(x);

  vHessian[p * (Parameters + 1)] = GetPriorStrength() *
   ((xp * xp) / (x * x) - xpp * (1.0 / x - 1.0));
 }

 vHessian[0] = GetPriorStrength();
}
