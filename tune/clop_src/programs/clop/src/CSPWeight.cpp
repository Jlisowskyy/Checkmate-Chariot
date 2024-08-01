/////////////////////////////////////////////////////////////////////////////
//
// CSPWeight.cpp
//
// Rémi Coulom
//
// May, 2010
//
/////////////////////////////////////////////////////////////////////////////
#include "CSPWeight.h"
#include "CRegression.h"

/////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////
CSPWeight::CSPWeight(const CRegression &reg,
                     int ReplicationThreshold,
                     int nMCMC):
 vResult(reg.GetPF().GetDimensions()),
 reg(reg),
 ReplicationThreshold(ReplicationThreshold),
 nMCMC(nMCMC)
{
}

/////////////////////////////////////////////////////////////////////////////
// Draw a sample at random, with Gibbs sampling
/////////////////////////////////////////////////////////////////////////////
const double *CSPWeight::NextSample(int i)
{
 //
 // Re-use previous sample according to ReplicationThreshold
 //
 if (ReplicationThreshold &&
     i > 0 &&
     reg.GetReplications(i - 1) < (1 + i / ReplicationThreshold))
  return reg.GetSample(i - 1);

 //
 // Start from previous sample, or random when i == 0
 //
 for (int j = int(vResult.size()); --j >= 0;)
  if (i == 0)
   vResult[j] = -1.0 + 2.0 * rnd.NextDouble();
  else
   vResult[j] = reg.GetSample(i - 1)[j];
 double LogCurrent = reg.GetLogWeight(&vResult[0]);

 //
 // Loop of MCMC trials
 //
 for (int k = nMCMC * reg.GetPF().GetDimensions(); --k >= 0;)
 {
  //
  // Choose a dimension at random
  //
  const int Index = rnd.NewValue() % int(vResult.size());
  const double Old = vResult[Index];
 
  //
  // Find a proper interval for uniform sampling
  //
  double Bounds[2] = {-1.0, 1.0};
#if 0
  const double LogTolerance = 5.0;

  for (int b = 2; --b >= 0;)
  {
   vResult[Index] = Bounds[b];
   double BoundLog = reg.GetLogWeight(&vResult[0]);

   while (true)
   {
    const double x = Old + 0.5 * (Bounds[b] - Old);
    vResult[Index] = x;
    const double Log = reg.GetLogWeight(&vResult[0]);

    if (Log > LogCurrent - LogTolerance)
     break;
    else
     Bounds[b] = x;
   }
  }
#endif

  //
  // Choose at random in this interval
  //
  vResult[Index] = Bounds[0] + (Bounds[1] - Bounds[0]) * rnd.NextDouble();
  const double LogNew = reg.GetLogWeight(&vResult[0]);

  //
  // Metropolis-Hastings algorithm
  //
  if (rnd.NextDouble() < std::exp(LogNew - LogCurrent))
   LogCurrent = LogNew;
  else
   vResult[Index] = Old;
 }

 return &vResult[0];
}
