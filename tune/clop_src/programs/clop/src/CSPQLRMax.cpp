/////////////////////////////////////////////////////////////////////////////
//
// CSPQLRMax.cpp
//
// Rémi Coulom
//
// November, 2008
//
/////////////////////////////////////////////////////////////////////////////
#include "CSPQLRMax.h"
#include "CRegression.h"

/////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////
CSPQLRMax::CSPQLRMax(CRegression &reg):
 reg(reg),
 vMax(reg.GetPF().GetDimensions()),
 Retries(1),
 sp(reg.GetPF().GetDimensions())
{
}

/////////////////////////////////////////////////////////////////////////////
// Pick next sample
/////////////////////////////////////////////////////////////////////////////
const double *CSPQLRMax::NextSample(int i)
{
 bool fFound = false;

 //
 // Try to find a definite negative sample of the posterior
 //
 if (reg.GetCount(COutcome::Loss) >= 1 && reg.GetCount(COutcome::Win) >= 1)
 {
  std::vector<double> vParam(reg.GetPF().GetDimensions());
  
  for (int j = Retries; --j >= 0;)
  {
   reg.GaussianSample(rnd, vParam);
   if (reg.GetPF().GetMax(&vParam[0], &vMax[0]))
   {
    fFound = true;
    break;
   }
  }
 }

 //
 // If no sample was found, quasi-random sample
 //
 if (!fFound)
  return sp.NextSample(i);

 return &vMax[0];
}
