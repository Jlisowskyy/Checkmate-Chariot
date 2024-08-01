/////////////////////////////////////////////////////////////////////////////
//
// CPFCubic.cpp
//
// Rémi Coulom
//
// December, 2008
//
/////////////////////////////////////////////////////////////////////////////
#include "CPFCubic.h"

#include <cmath>

/////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////
CPFCubic::CPFCubic(int Dimensions):
 CParametricFunction(
  Dimensions,
  ((Dimensions + 1) * (Dimensions + 2) * (Dimensions + 3)) / 6)
{
}

/////////////////////////////////////////////////////////////////////////////
// Compute value. v is an augmented vector
/////////////////////////////////////////////////////////////////////////////
double CPFCubic::GetValue(const double *vParam, const double *vx) const
{
 int p = 0;

 double Result = vParam[p++];

 for (int i = Dimensions; --i >= 0; p++)
  Result += vParam[p] * vx[i];

 for (int i = Dimensions; --i >= 0;)
  for (int j = i + 1; --j >= 0; p++)
   Result += vParam[p] * vx[i] * vx[j];

 for (int i = Dimensions; --i >= 0;)
  for (int j = i + 1; --j >= 0;)
   for (int k = j + 1; --k >= 0; p++)
    Result += vParam[p] * vx[i] * vx[j] * vx[k];

 return Result;
}

/////////////////////////////////////////////////////////////////////////////
// Find point that maximizes this function
/////////////////////////////////////////////////////////////////////////////
bool CPFCubic::GetMax(const double * restrict vParam,
                      double * restrict vx) const
{
 if (Dimensions > 1)
  return CParametricFunction::GetMax(vParam, vx);

 double a = 3.0 * vParam[3];
 double b = 2.0 * vParam[2];
 double c = 1.0 * vParam[1];

 double Delta = b * b - 4.0 * a * c;

 if (Delta < 0)
  return false;

 double s = std::sqrt(Delta);

 vx[0] = (-b - s) / (2 * a);

 if (vx[0] < -1.0)
  vx[0] = -1.0;
 else if (vx[0] > 1.0)
  vx[0] = 1.0;

 return true;
}

/////////////////////////////////////////////////////////////////////////////
// Compute monomials
/////////////////////////////////////////////////////////////////////////////
void CPFCubic::GetMonomials(const double * restrict vx,
                            double * restrict vMonomial) const
{
 int p = 0;

 vMonomial[p++] = 1.0;

 for (int i = Dimensions; --i >= 0; p++)
  vMonomial[p] = vx[i];

 for (int i = Dimensions; --i >= 0;)
  for (int j = i + 1; --j >= 0; p++)
   vMonomial[p] = vx[i] * vx[j];

 for (int i = Dimensions; --i >= 0;)
  for (int j = i + 1; --j >= 0;)
   for (int k = j + 1; --k >= 0; p++)
    vMonomial[p] = vx[i] * vx[j] * vx[k];
}

/////////////////////////////////////////////////////////////////////////////
// Gradient
/////////////////////////////////////////////////////////////////////////////
void CPFCubic::GetGradient(const double * restrict vParam,
                           const double * restrict vx,
                           double * restrict vG) const
{
 int p = 1;

 for (int i = Dimensions; --i >= 0; p++)
  vG[i] = vParam[p];

 for (int i = Dimensions; --i >= 0;)
  for (int j = i + 1; --j >= 0; p++)
  {
   vG[i] += vParam[p] * vx[j];
   vG[j] += vParam[p] * vx[i];
  }

 for (int i = Dimensions; --i >= 0;)
  for (int j = i + 1; --j >= 0;)
   for (int k = j + 1; --k >= 0; p++)
   {
    vG[i] += vParam[p] * vx[j] * vx[k];
    vG[j] += vParam[p] * vx[i] * vx[k];
    vG[k] += vParam[p] * vx[i] * vx[j];
   }
}
