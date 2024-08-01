/////////////////////////////////////////////////////////////////////////////
//
// CRSPSA.cpp
//
// Rémi Coulom
//
// August, 2011
//
/////////////////////////////////////////////////////////////////////////////
#include "CRSPSA.h"
#include "CResults.h"
#include "COutcome.h"
#include "debug.h"

#include <cmath>

const double CRSPSA::tOutcomeValue[COutcome::Outcomes] = {0.0, 1.0, 0.5};

/////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////
CRSPSA::CRSPSA(CResults &results,
               int BatchSize,
               double BatchGrowth,
               double EtaPlus,
               double EtaMinus,
               double DeltaMin,
               double DeltaMax,
               double Delta0,
               double Rho):
 CObserver(results),
 Dimensions(results.GetDimensions()),
 BatchSize(BatchSize),
 BatchGrowth(BatchGrowth),
 EtaPlus(EtaPlus),
 EtaMinus(EtaMinus),
 DeltaMin(DeltaMin),
 DeltaMax(DeltaMax),
 Delta0(Delta0),
 Rho(Rho),
 vTheta(Dimensions),
 vDelta(Dimensions),
 vPerturbation(Dimensions),
 vPreviousGradient(Dimensions),
 vCurrentGradient(Dimensions),
 vSample(Dimensions)
{
 OnReset();
}

/////////////////////////////////////////////////////////////////////////////
// OnOutcome
/////////////////////////////////////////////////////////////////////////////
void CRSPSA::OnOutcome(int i)
{
 FATAL(i != k);

 //
 // Accumulate gradient estimates
 //
 if ((k & 1) != 0)
 {
  const double yPlus = tOutcomeValue[results.GetOutcome(i)];
  const double yMinus = tOutcomeValue[results.GetOutcome(i - 1)];

  for (int i = Dimensions; --i >= 0;)
   vCurrentGradient[i] += (yPlus - yMinus) / (2.0 * vPerturbation[i]);
 }

 k = k + 1;

 //
 // New batch
 //
 if (k == NextBatch)
  for (int i = Dimensions; --i >= 0;)
  {
   const double Direction = vPreviousGradient[i] * vCurrentGradient[i];

   if (Direction > 0)
   {
    vDelta[i] *= EtaPlus;
    if (vDelta[i] > DeltaMax)
     vDelta[i] = DeltaMax;
   }
   else if (Direction < 0)
   {
    vDelta[i] *= EtaMinus;
    if (vDelta[i] < DeltaMin)
     vDelta[i] = DeltaMin;
    vCurrentGradient[i] = 0.0;
   } 

   if (vCurrentGradient[i] > 0)
    vTheta[i] += vDelta[i];
   else if (vCurrentGradient[i] < 0)
    vTheta[i] -= vDelta[i];

   Bound(vTheta[i], 1.0 - Rho * vDelta[i]);

   vPreviousGradient[i] = vCurrentGradient[i];
   vCurrentGradient[i] = 0.0;

   CurrentBatchSize = int(CurrentBatchSize * BatchGrowth);
   NextBatch += CurrentBatchSize;
  }
}

/////////////////////////////////////////////////////////////////////////////
// OnReset
/////////////////////////////////////////////////////////////////////////////
void CRSPSA::OnReset()
{
 k = 0;
 std::fill(vTheta.begin(), vTheta.end(), 0.0);
 std::fill(vDelta.begin(), vDelta.end(), Delta0);
 std::fill(vCurrentGradient.begin(), vCurrentGradient.end(), 0.0);
 std::fill(vPreviousGradient.begin(), vPreviousGradient.end(), 0.0);
 CurrentBatchSize = BatchSize;
 NextBatch = 2 * CurrentBatchSize;
}

/////////////////////////////////////////////////////////////////////////////
// NextSample
/////////////////////////////////////////////////////////////////////////////
const double *CRSPSA::NextSample(int i)
{
 //
 // Even k: generate random perturbation
 //
 if ((k & 1) == 0)
  for (int i = Dimensions; --i >= 0;)
   vPerturbation[i] = Rho * ((rnd.NewValue() & 1) ? -vDelta[i] : vDelta[i]);

 //
 // Compute sample
 //
 const double Sign = ((k & 1) * 2) - 1;
 for (int i = Dimensions; --i >= 0;)
  vSample[i] = vTheta[i] + Sign * vPerturbation[i];

 return &vSample[0];
}

/////////////////////////////////////////////////////////////////////////////
// MaxParameter
/////////////////////////////////////////////////////////////////////////////
bool CRSPSA::MaxParameter(double vMax[]) const
{
 std::copy(vTheta.begin(), vTheta.end(), vMax);
 return true;
}
