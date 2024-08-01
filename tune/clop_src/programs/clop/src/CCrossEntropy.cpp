/////////////////////////////////////////////////////////////////////////////
//
// CCrossEntropy.cpp
//
// Rémi Coulom
//
// April, 2009
//
/////////////////////////////////////////////////////////////////////////////
#include "CCrossEntropy.h"
#include "CResults.h"
#include "CMatrixOperations.h"
#include "debug.h"

#include <algorithm>

/////////////////////////////////////////////////////////////////////////////
// Compute mean and variance
/////////////////////////////////////////////////////////////////////////////
void CCrossEntropy::ComputeMeanAndVariance(int n)
{
 //
 // Mean
 //
 for (int i = Dimensions; --i >= 0;)
 {
  vMean[i] = 0.0;
  for (int k = n; --k >= 0;)
   vMean[i] += vPopulation[k].vParameters[i];
  vMean[i] /= n;
 }

 //
 // Variance
 //
 for (int i = Dimensions; --i >= 0;)
  for (int j = Dimensions; --j >= i;)
  {
   int Index = i * Dimensions + j;
   vVariance[Index] = 0.0;
   if (i == j || !fDiagonal)
   {
    for (int k = n; --k >= 0;)
     vVariance[Index] += (vMean[i] - vPopulation[k].vParameters[i]) *
                         (vMean[j] - vPopulation[k].vParameters[j]);
    vVariance[Index] /= (n - 1);
   }
  }
}

/////////////////////////////////////////////////////////////////////////////
// Generate a new population
/////////////////////////////////////////////////////////////////////////////
void CCrossEntropy::NewGeneration()
{
 Generation++;

 //
 // Initial, uniform random population
 //
 if (Generation == 1)
 {
  vPopulation.resize(PopulationSize);

  for (int i = 0; i < PopulationSize; i++)
  {
   vPopulation[i].Victories = 0;
   vPopulation[i].Games = 0;
   vPopulation[i].vParameters.resize(Dimensions);
   for (int j = Dimensions; --j >= 0;)
    vPopulation[i].vParameters[j] = rnd.NextDouble() * 2.0 - 1.0;
  }

  ComputeMeanAndVariance(PopulationSize);
 }

 //
 // Population generated from best individuals
 //
 else
 {
  //
  // Mean and variance of elite samples
  //
  std::sort(vPopulation.begin(), vPopulation.end());
  ComputeMeanAndVariance(SelectionSize);

  //
  // Smooth mean
  //
  for (int i = Dimensions; --i >= 0;)
   vMean[i] = Smoothing * vMean[i] + (1.0 - Smoothing) * vOldMean[i];

  //
  // Decayed smoothing for variance
  //
  {
   double x = Smoothing / (1.0 + std::sqrt(Generation - 2) * DynamicSmoothing);

   for (int i = Dimensions; --i >= 0;)
    for (int j = Dimensions; --j >= i;)
    {
     int Index = i * Dimensions + j;
     double OldVariance = vOldVariance[Index];
     if (fDistributionSmoothing)
      OldVariance += (vMean[i] - vOldMean[i]) * (vMean[j] - vOldMean[j]);
     vVariance[Index] = x * vVariance[Index] + (1.0 - x) * OldVariance;
    }
  }

  //
  // Generate new population with this Gaussian distribution
  //
  for (int k = PopulationSize; --k >= 0;)
  {
   std::vector<double> vIID(Dimensions);
   for (int i = Dimensions; --i >= 0;)
    vIID[i] = rnd.NextGaussian();

   std::vector<double> vCholesky(Dimensions * Dimensions);
   CMatrixOperations::Cholesky(&vVariance[0], &vCholesky[0], Dimensions);

   vPopulation[k].Victories = 0;
   vPopulation[k].Games = 0;
   vPopulation[k].vParameters = vMean;
   for (int i = Dimensions; --i >= 0;)
    for (int j = 0; j <= i; j++)
    {
     double x = vPopulation[k].vParameters[i];
     x += vIID[j] * vCholesky[j + i * Dimensions];
     if (x < -1.0)
      x = -1.0;
     else if (x > 1.0)
      x = 1.0;
     vPopulation[k].vParameters[i] = x;
    }
  }

  //
  // Increase batch size
  //
  BatchSize = int(BatchSize * BatchGrowth);
 }

 //
 // Remember mean and variance (for smoothing)
 //
 vOldMean = vMean;
 vOldVariance = vVariance;

 //
 // Initialize array of scores
 //
 vScore.resize(BatchSize);
 std::fill_n(vScore.begin(), BatchSize, 0);
}

/////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////
CCrossEntropy::CCrossEntropy(CResults &results,
                             double Smoothing,
                             double DynamicSmoothing,
                             bool fDistributionSmoothing,
                             int PopulationSize,
                             int SelectionSize,
                             int InitialBatchSize,
                             double BatchGrowth,
                             bool fDiagonal):
 CObserver(results),
 Dimensions(results.GetDimensions()),
 Smoothing(Smoothing),
 DynamicSmoothing(DynamicSmoothing),
 fDistributionSmoothing(fDistributionSmoothing),
 PopulationSize(PopulationSize),
 SelectionSize(SelectionSize),
 InitialBatchSize(InitialBatchSize),
 BatchGrowth(BatchGrowth),
 fDiagonal(fDiagonal),
 vMean(Dimensions),
 vVariance(Dimensions * Dimensions),
 vOldMean(Dimensions),
 vOldVariance(Dimensions * Dimensions)
{
 FATAL(SelectionSize > PopulationSize);
 FATAL(SelectionSize < 2);
 OnReset();
}

/////////////////////////////////////////////////////////////////////////////
// Find next sample
/////////////////////////////////////////////////////////////////////////////
const double *CCrossEntropy::NextSample(int i)
{
 while (true)
 {
  if (--PopulationIndex < 0)
  {
   PopulationIndex += PopulationSize;

   if (--BatchIndex < 0)
   {
    BatchIndex += BatchSize;
    FATAL(!qmInProgress.empty()); // TODO: wait for empty()
    NewGeneration();
   }
  }

  CIndividual &ind = vPopulation[PopulationIndex];

  //
  // Return this individual if it still has a chance to be selected
  //
  if (vScore[ind.Victories + BatchIndex] < SelectionSize)
  {
   qmInProgress.add(i, ind);
   return &ind.vParameters[0];
  }
 }
}

/////////////////////////////////////////////////////////////////////////////
// Estimate optimal parameter
/////////////////////////////////////////////////////////////////////////////
bool CCrossEntropy::MaxParameter(double vMax[]) const
{
 if (Generation == 0)
  return false;

 if (Generation == 1)
 {
  int iMax = 0;
  for (int i = int(vPopulation.size()); --i >= 0;)
   if (vPopulation[i] < vPopulation[iMax])
    iMax = i;
  std::copy(vPopulation[iMax].vParameters.begin(),
            vPopulation[iMax].vParameters.end(),
            vMax);
 }
 else
  std::copy(vOldMean.begin(), vOldMean.end(), vMax);

 return true;
}

/////////////////////////////////////////////////////////////////////////////
// Record a game outcome
/////////////////////////////////////////////////////////////////////////////
void CCrossEntropy::OnOutcome(int i)
{
 CIndividual *pind = qmInProgress.remove(i);
 FATAL(!pind);
 if (results.GetOutcome(i) == COutcome::Win)
 { 
  vScore[pind->Victories]++;
  pind->Victories++;
 }
 pind->Games++;
}

/////////////////////////////////////////////////////////////////////////////
// Reset all data
/////////////////////////////////////////////////////////////////////////////
void CCrossEntropy::OnReset()
{
 vPopulation.clear();
 qmInProgress.clear();
 Generation = 0;
 PopulationIndex = 0;
 BatchIndex = 0;
 BatchSize = InitialBatchSize;
 std::fill_n(vOldMean.begin(), Dimensions, 0.0);
}
