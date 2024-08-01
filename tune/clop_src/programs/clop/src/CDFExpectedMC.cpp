/////////////////////////////////////////////////////////////////////////////
//
// CDFExpectedMC.cpp
//
// Rémi Coulom
//
// March, 2009
//
/////////////////////////////////////////////////////////////////////////////
#include "CDFExpectedMC.h"
#include "CLogistic.h"
#include "CRegression.h"
#include "random.h"

/////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////
CDFExpectedMC::CDFExpectedMC(CRegression &reg, int Samples, int Seed):
 CDiffFunction(reg.GetPF().GetDimensions()),
 reg(reg),
 vv(Samples, std::vector<double>(reg.GetPF().GetParameters()))
{
 CRandom<unsigned> rnd(Seed);
 for (int i = int(vv.size()); --i >= 0;)
  reg.GaussianSample(rnd, vv[i]);
}

/////////////////////////////////////////////////////////////////////////////
// GetOutput
/////////////////////////////////////////////////////////////////////////////
double CDFExpectedMC::GetOutput(const double *vInput)
{
 double Total = 0.0;

 for (int i = int(vv.size()); --i >= 0;)
 {
  double r = reg.GetPF().GetValue(&vv[i][0], &vInput[0]);
  Total += CLogistic::f(r);
 }

 return Total / double(vv.size());
}
