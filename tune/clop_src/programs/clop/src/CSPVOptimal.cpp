/////////////////////////////////////////////////////////////////////////////
//
// CSPVOptimal.cpp
//
// Rémi Coulom
//
// November, 2008
//
/////////////////////////////////////////////////////////////////////////////
#include "CSPVOptimal.h"
#include "CRegression.h"

/////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////
CSPVOptimal::CSPVOptimal(const CRegression &reg,
                         CDFVariance &dfvar,
                         int Trials):
 reg(reg),
 dfvar(dfvar),
 Trials(Trials),
 v(reg.GetPF().GetDimensions()),
 vTrial(reg.GetPF().GetDimensions()),
 nTotal(0),
 spCG(reg),
 spFail(reg)
{
}

/////////////////////////////////////////////////////////////////////////////
// Seed
/////////////////////////////////////////////////////////////////////////////
void CSPVOptimal::Seed(unsigned n)
{
 rnd.Seed(n); 
 spCG.Seed(n + 1);
 spFail.Seed(n + 2);
 nTotal = 0;
}

/////////////////////////////////////////////////////////////////////////////
// Next sample
/////////////////////////////////////////////////////////////////////////////
const double *CSPVOptimal::NextSample(int i)
{
 nTotal++;

 //
 // Maximize variance as soon as enough wins and losses have been found
 //
 if (reg.GetCount(COutcome::Loss) >= dfvar.GetMinSamples() &&
     reg.GetCount(COutcome::Win)  >= dfvar.GetMinSamples())
 {
  dfvar.SelectAtRandom(rnd);
  double Max;

  {
   const double *vSP = spCG.NextSample(i);
   std::copy(vSP, vSP + reg.GetPF().GetDimensions(), v.begin());
  }
  dfvar.CG(&v[0]);
  Max = dfvar.GetOutput(&v[0]);

  for (int j = Trials; --j >= 0;)
  {
   {
    const double *vSP = spCG.NextSample(i);
    std::copy(vSP, vSP + reg.GetPF().GetDimensions(), vTrial.begin());
   }
   dfvar.CG(&vTrial[0]);
   double x = dfvar.GetOutput(&vTrial[0]);
   if (x > Max)
   {
    Max = x;
    v = vTrial;
   }
  }

  //
  // Reject samples on the border
  //
  bool fBorder = false;
  for (int j = int(v.size()); --j >= 0;)
   if (v[j] == -1.0 || v[j] == 1.0)
   {
    fBorder = true;
    break;
   }

  if (!fBorder)
   return &v[0];
 }

 //
 // Otherwise, pick a sample at random
 //
 return spFail.NextSample(i);
}
