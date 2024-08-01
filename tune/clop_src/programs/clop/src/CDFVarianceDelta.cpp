/////////////////////////////////////////////////////////////////////////////
//
// CDFVarianceDelta.cpp
//
// Rémi Coulom
//
// November, 2008
//
/////////////////////////////////////////////////////////////////////////////
#include "CDFVarianceDelta.h"
#include "CRegression.h"
#include "CLogistic.h"

#include <cmath>

/////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////
CDFVarianceDelta::CDFVarianceDelta(CRegression &reg):
 CDFVariance(reg),
 vz(Dimensions),
 vX(Parameters),
 vSigmaX(Parameters),
 vH(Dimensions * Dimensions)
{
 SetMinSamples(5);
}

/////////////////////////////////////////////////////////////////////////////
// GetOutput
/////////////////////////////////////////////////////////////////////////////
double CDFVarianceDelta::GetOutput(const std::vector<double> &vInput)
{
 pvx = &vInput[0];
 const double *vParamMAP = reg.MAP();
 reg.GetPF().GetGradient(vParamMAP, pvx, &vz[0]);
 reg.GetPF().GetMonomials(pvx, &vX[0]);
 r = reg.GetPF().DotProduct(vParamMAP, &vX[0]);
 vSigmaX = vX;
 CholeskySolve(vSigmaX);
 var = reg.GetPF().DotProduct(&vX[0], &vSigmaX[0]);
 s = CLogistic::f(r);
 double Derivative = s * CLogistic::f(-r);
 return std::log(var * Derivative * Derivative) * 0.5;
}

/////////////////////////////////////////////////////////////////////////////
// Compute Gradient
/////////////////////////////////////////////////////////////////////////////
void CDFVarianceDelta::ComputeGradient()
{
 reg.GetPF().GetGradient(&vSigmaX[0], pvx, &vGradient[0]);

 const double m = 1.0 / var;
 const double fppr = 1.0 - 2.0 * s;

 for (int i = Dimensions; --i >= 0;)
  vGradient[i] = vGradient[i] * m + fppr * vz[i];
}
