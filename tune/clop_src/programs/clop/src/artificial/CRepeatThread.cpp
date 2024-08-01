/////////////////////////////////////////////////////////////////////////////
//
// CRepeatThread.cpp
//
// Rémi Coulom
//
// March, 2009
//
/////////////////////////////////////////////////////////////////////////////
#include "CRepeatThread.h"
#include "CRepeatThreads.h"
#include "CArtificialExperiment.h"
#include "CArtificialProblem.h"
#include "CSamplingPolicy.h"
#include "CMaxEstimator.h"
#include "CResults.h"

/////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////
CRepeatThread::CRepeatThread(CRepeatThreads &rts,
                             CArtificialExperiment &artexp):
 rts(rts),
 artexp(artexp),
 Seed(0),
 vMax(artexp.problem.GetDimensions()),
 vOptimal(artexp.problem.GetDimensions())
{
 artexp.problem.GetOptimalParameters(&vOptimal[0]);
}

/////////////////////////////////////////////////////////////////////////////
// Thread method
/////////////////////////////////////////////////////////////////////////////
void CRepeatThread::operator()()
{
 while(rts.Dispatch(*this))
 {
  //
  // Initialize loop
  //
  artexp.results.Reset();
  artexp.sp.Seed(Seed);
  artexp.problem.Seed(Seed + 1);

  //
  // Loop over checkpoints
  //
  for (int i = int(rts.vpcpd.size()); --i >= 0;)
  {
   //
   // Loop over samples
   //
   const int Max = rts.vpcpd[i]->GetSamples();
   const int Min =
    i + 1 < int(rts.vpcpd.size()) ? rts.vpcpd[i + 1]->GetSamples() : 0;

   for (int j = Max; --j >= Min;)
   {
    const double *v = artexp.sp.NextSample(artexp.results.GetSamples());
    COutcome outcome = artexp.problem.GetOutcome(v);
    artexp.results.AddSample(v, outcome);
   }

   artexp.results.Refresh();

   //
   // Count a miss if failure of maximum estimation
   //
   int Misses = 0;
   if (!artexp.me.MaxParameter(&vMax[0]))
   {
    Misses = 1;
    std::fill(vMax.begin(), vMax.end(), 0.0);
   }

   //
   // Compute error
   //
   double Error = artexp.problem.GetOptimalProba() -
                  artexp.problem.GetProba(&vMax[0]);

   //
   // Report result
   //
   rts.vpcpd[i]->Collect(Seed, Error, Misses);
  }
 }
}
