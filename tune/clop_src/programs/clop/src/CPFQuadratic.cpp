/////////////////////////////////////////////////////////////////////////////
//
// CPFQuadratic.cpp
//
// Rémi Coulom
//
// December, 2008
//
/////////////////////////////////////////////////////////////////////////////
#include "CPFQuadratic.h"
#include "CMatrixOperations.h"

#include <vector>

/////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////
CPFQuadratic::CPFQuadratic(int Dimensions):
 CParametricFunction(Dimensions,
                     ((Dimensions + 1) * (Dimensions + 2)) / 2)
{
}

/////////////////////////////////////////////////////////////////////////////
// Compute value
/////////////////////////////////////////////////////////////////////////////
double CPFQuadratic::GetValue(const double *vParam,
                                  const double *vx) const
{
 int p = 0;

 double Result = vParam[p++];

 for (int i = Dimensions; --i >= 0;)
  Result += vParam[p++] * vx[i];

 for (int i = Dimensions; --i >= 0;)
  for (int j = i + 1; --j >= 0;)
   Result += vParam[p++] * vx[i] * vx[j];

 return Result;
}

/////////////////////////////////////////////////////////////////////////////
// Get Hessian matrix
/////////////////////////////////////////////////////////////////////////////
void CPFQuadratic::GetHessian(const double *vParam, double *vH) const
{
 for (int p = Dimensions + 1, i = Dimensions; --i >= 0;)
  for (int j = i + 1; --j >= 0; p++)
  {
   if (i == j)
    vH[i * Dimensions + j] = vParam[p];
   else
    vH[i * Dimensions + j] = 0.5 * vParam[p];
  }
}

/////////////////////////////////////////////////////////////////////////////
// Find input parameters that maximize output
/////////////////////////////////////////////////////////////////////////////
bool CPFQuadratic::GetMax(const double * restrict vParam,
                          double * restrict vx) const
{
 //
 // Initialize the matrix of the quadratic term (opposite)
 //
 std::vector<double> vMatrix(Dimensions * Dimensions);
 for (int p = Dimensions + 1, i = Dimensions; --i >= 0;)
  for (int j = i + 1; --j >= 0; p++)
  {
   if (i == j)
    vMatrix[i * Dimensions + j] = -vParam[p];
   else
    vMatrix[j * Dimensions + i] = -0.5 * vParam[p];
  }

 //
 // Check that the form is definite negative with Cholesky
 //
 if (!CMatrixOperations::Cholesky(&vMatrix[0], &vMatrix[0], Dimensions))
  return false;
 else
 {
  for (int i = Dimensions; --i >= 0;)
   vx[i] = 0.5 * vParam[Dimensions - i];
  CMatrixOperations::Solve(&vMatrix[0], &vx[0], Dimensions);
 }

 //
 // Check that the maximum is within boundaries
 //
 for (int i = Dimensions; --i >= 0;)
  if (vx[i] < -1.0)
   return false;
  else if (vx[i] > 1.0)
   return false;

 return true;
}

/////////////////////////////////////////////////////////////////////////////
// Compute monomials
/////////////////////////////////////////////////////////////////////////////
void CPFQuadratic::GetMonomials(const double * restrict vx,
                                double * restrict vMonomial) const
{
 int p = 0;

 vMonomial[p++] = 1.0;

 for (int i = Dimensions; --i >= 0;)
  vMonomial[p++] = vx[i];

 for (int i = Dimensions; --i >= 0;)
  for (int j = i + 1; --j >= 0;)
   vMonomial[p++] = vx[i] * vx[j];
}

/////////////////////////////////////////////////////////////////////////////
// Gradient
/////////////////////////////////////////////////////////////////////////////
void CPFQuadratic::GetGradient(const double * restrict vParam,
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
}
