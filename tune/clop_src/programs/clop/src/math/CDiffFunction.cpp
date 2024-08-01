/////////////////////////////////////////////////////////////////////////////
//
// CDiffFunction.cpp
//
// Rémi Coulom
//
// November, 2008
//
/////////////////////////////////////////////////////////////////////////////
#include "CDiffFunction.h"
#include "CMatrixOperations.h"
#include "debug.h"

#include <cmath>
#include <iostream>
#include <iomanip>
#include <limits>

static const double NewtonThreshold = 1e-5;
static const double MinNewtonStep = 1e-10;
static const double MaxBeta = 100.0;
static const double CGEpsilon = 1e-10;
static const int MaxNewtonIterations = 100;
static const int MaxSDIterations = 100;
static const int MaxCGIterations = 100;

/////////////////////////////////////////////////////////////////////////////
// Newton-Raphson's method
/////////////////////////////////////////////////////////////////////////////
void CDiffFunction::Newton(std::vector<double> &vMax, bool fTrace)
{
 const std::vector<double> &vG = GetGradient();
 const std::vector<double> &vH = GetHessian();
 int n = GetDimensions();

 if (fTrace)
  std::cout << '\n';

 for (int Iterations = MaxNewtonIterations; --Iterations >= 0;)
 {
  double L = GetOutput(&vMax[0]);
  ComputeGradient();
  ComputeHessian();

  if (fTrace)
  {
   std::cout << std::setw(5) << Iterations;
   std::cout << std::setw(12) << vMax[0];
   std::cout << std::setw(12) << L;
   std::cout << std::setw(12) << vG[0];
   std::cout << std::setw(12) << vH[0];
   std::cout << '\n';
  }

  //
  // Compute Gradient multiplied by inverse of opposite of Hessian
  //
  vStep = vG;
  if (CMatrixOperations::Cholesky(&vH[0], &vCholesky[0], n))
   CMatrixOperations::Solve(&vCholesky[0], &vStep[0], n);

  //
  // If the Hessian is not definite negative, CG
  //
  else
  {
   CG(&vMax[0], fTrace);
   return;
  }

  //
  // Apply change
  //
  for (int i = n; --i >= 0;)
   vxTemp[i] = Normalize(vMax[i] + vStep[i]);

  //
  // If probability did not improve, use CG
  //
  double LNew = GetOutput(&vxTemp[0]);
  double NewtonStep = 1.0;

  if ((LNew != LNew || LNew < L) && NewtonStep > MinNewtonStep)
  {
   CG(&vMax[0], fTrace);
   return;
  }

  //
  // Stop looping if small improvement
  //
  vMax = vxTemp;
  if (LNew - L < NewtonThreshold) 
   return;
 }

 //std::cerr << "warning: reached MaxNewtonIterations\n";
}

/////////////////////////////////////////////////////////////////////////////
// Set input for LineOpt
/////////////////////////////////////////////////////////////////////////////
double CDiffFunction::SetLineInput(const double vx0[],
                                   const double vDir[],
                                   double x)
{
 for (int i = Dimensions; --i >= 0;)
  vxTemp[i] = vx0[i] + x * vDir[i];
 return GetOutput(&vxTemp[0]);
}

/////////////////////////////////////////////////////////////////////////////
// Line optimization.
/////////////////////////////////////////////////////////////////////////////
double CDiffFunction::LineOpt(const double vx0[],
                              const double vDir[],
                              bool fTrace)
{
 const double Epsilon = 0.00001;
 const double Big = 10000;

 //
 // Normalize vDir for first step
 //
 double N2 = 0;
 for (int i = Dimensions; --i >= 0;)
  N2 += vDir[i] * vDir[i];
 double Scale = 1.0 / std::sqrt(N2);
 if (Scale == std::numeric_limits<double>::infinity())
  return 0.0;

 //
 // First, find a bracket, such that:
 // f(tx[1]) > f(tx[0]) && f(tx[1]) > f(tx[2])
 //
 double tx[3];
 double tf[3];

 tx[0] = 0;
 tf[0] = SetLineInput(vx0, vDir, tx[0]);

 tx[2] = Scale;
 tf[2] = SetLineInput(vx0, vDir, tx[2]);

 //
 // Find a close point that improves the function
 //
 while(true) 
 {
  tx[1] = tx[2] * 0.5;
  tf[1] = SetLineInput(vx0, vDir, tx[1]);

  if (tx[1] < Epsilon)
   return 0.0;

  if (tf[1] <= tf[0])
  {
   tx[2] = tx[1];
   tf[2] = tf[1];
  }
  else
   break;
 }

 //
 // Find a distant point that is worse
 //
 while (tf[1] <= tf[2])
 {
  if (tx[2] > Big)
   return tx[2];

  tx[1] = tx[2];
  tf[1] = tf[2];
  tx[2] = tx[1] * 2.0;
  tf[2] = SetLineInput(vx0, vDir, tx[2]);
 }

 //
 // Show the initial bracket
 //
 if (fTrace)
  {
   for (int i = 0; i < 3; i++)
   {
    std::cout << std::setw(12) << vx0[0] + tx[i] * vDir[0];
    std::cout << std::setw(12) << tf[i];
   }
   std::cout << '\n';
  }

 //
 // Do a quadratic regression
 //
 double bma = tx[1] - tx[0];
 double bmc = tx[1] - tx[2];
 double fbmfa = tf[1] - tf[0];
 double fbmfc = tf[1] - tf[2];

 double x = tx[1] - 0.5 * (bma * bma * fbmfc - bmc * bmc * fbmfa) /
            (bma * fbmfc - bmc * fbmfa);
 double f = SetLineInput(vx0, vDir, x);

 if (fTrace)
  std::cout << std::setw(12) << vx0[0] + x * vDir[0] << std::setw(12) << f << '\n';

 if (f > tf[1])
  return x;
 else
  return tx[1];
}

/////////////////////////////////////////////////////////////////////////////
// Steepest descent
/////////////////////////////////////////////////////////////////////////////
void CDiffFunction::SteepestDescent(std::vector<double> &vMax, bool fTrace)
{
 const std::vector<double> &vG = GetGradient();
 int n = GetDimensions();

 //
 // Loop of iterations
 //
 for (int Iteration = MaxSDIterations; --Iteration >= 0;)
 {
  GetOutput(&vMax[0]);
  ComputeGradient();

  //
  // Maximize in gradient direction
  //
  vGCopy = vG;
  double x = LineOpt(&vMax[0], &vGCopy[0], fTrace);

  double Delta2 = 0.0;

  for (int i = n; --i >= 0;)
  {
   double New = Normalize(vMax[i] + x * vGCopy[i]);
   double Delta = New - vMax[i];
   vMax[i] = New;
   Delta2 += Delta * Delta;
  }

  if (Delta2 < CGEpsilon)
   break;
 }
}

/////////////////////////////////////////////////////////////////////////////
// Conjugate Gradient
/////////////////////////////////////////////////////////////////////////////
void CDiffFunction::CG(double vMax[], bool fTrace)
{
 const std::vector<double> &vG = GetGradient();
 std::vector<double> vPrevG;
 std::vector<double> vD;

 const int Cycles = GetDimensions();

 //
 // Loop of iterations
 //
 for (int Iteration = 0; Iteration < MaxCGIterations; Iteration++)
 {
  //
  // Compute output and gradient at current point
  //
  GetOutput(&vMax[0]);
  ComputeGradient();

  //
  // At the beginning of a cycle, go in the gradient direction
  //
  int Cycle = Iteration % Cycles;
  if (Cycle == 0)
  {
   vPrevG = vG;
   vD = vG;
  }

  //
  // Otherwise, compute conjugate direction
  //
  else
  {
   double Num = 0;
   double Den = 0;

   for (int i = GetDimensions(); --i >= 0;)
   {
    Num += vG[i] * (vG[i] - vPrevG[i]);
    Den += vPrevG[i] * vPrevG[i];
   }

   double Beta = Num / Den;

   if (Den == 0 || Beta > MaxBeta)
    Beta = MaxBeta;

   for (int i = GetDimensions(); --i >= 0;)
   {
    vD[i] = vG[i] + Beta * vD[i];
    vPrevG[i] = vG[i];
   }
  }

  //
  // Trick to avoid getting stuck at a minimum
  //
  {
   int x = Iteration % GetDimensions();
   if (vD[x] * vD[x] == 0.0)
   {
    if (vD[x] >= 0)
     vD[x] = 1.0;
    else
     vD[x] = -1.0;
   }
  }
 
  //
  // Perform line optimization
  //
  double x = LineOpt(vMax, &vD[0], fTrace);

  double Delta2 = 0.0;
  for (int i = GetDimensions(); --i >= 0;)
  {
   double New = Normalize(vMax[i] + x * vD[i]);
   double Delta = New - vMax[i];
   vMax[i] = New;
   Delta2 += Delta * Delta;
  }

  if (Cycle == 0 && Delta2 < CGEpsilon)
   return;
 }

 // std::cerr << "warning: reached MaxCGIterations\n";
}
