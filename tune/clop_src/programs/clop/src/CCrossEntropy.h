/////////////////////////////////////////////////////////////////////////////
//
// CCrossEntropy.h
//
// Rémi Coulom
//
// April, 2009
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CCrossEntropy_Declared
#define CCrossEntropy_Declared

#include "CSamplingPolicy.h"
#include "CMaxEstimator.h"
#include "CObserver.h"
#include "CQuickMap.h"
#include "random.h"

#include <vector>

class CCrossEntropy:
 public CSamplingPolicy,
 public CMaxEstimator,
 private CObserver
{
 private: ///////////////////////////////////////////////////////////////////
  const int Dimensions;
  const double Smoothing;
  const double DynamicSmoothing;
  const bool fDistributionSmoothing;
  const int PopulationSize;
  const int SelectionSize;
  const int InitialBatchSize;
  const double BatchGrowth;
  bool fDiagonal;

  CRandom<unsigned> rnd;

  struct CIndividual
  {
   std::vector<double> vParameters;
   int Victories;
   int Games;

   bool operator<(const CIndividual &ind) const
   {
    return Victories > ind.Victories;
   }
  };

  std::vector<CIndividual> vPopulation;
  std::vector<int> vScore; // Number of individuals with more than this score

  std::vector<double> vMean;
  std::vector<double> vVariance;
  std::vector<double> vOldMean;
  std::vector<double> vOldVariance;

  void ComputeMeanAndVariance(int n);

  CQuickMap<CIndividual> qmInProgress;

  int Generation;
  int BatchSize;
  int PopulationIndex;
  int BatchIndex;

  void NewGeneration();

  //
  // CObserver
  //
  void OnOutcome(int i);
  void OnReset();

 public: ////////////////////////////////////////////////////////////////////
  explicit CCrossEntropy(CResults &results,
                         double Smoothing = 1.0,
                         double DynamicSmoothing = 0.1,
                         bool fDistributionSmoothing = true,
                         int PopulationSize = 100,
                         int SelectionSize = 10,
                         int InitialBatchSize = 10,
                         double BatchGrowth = 1.15,
                         bool fDiagonal = true);

  //
  // CSamplingPolicy
  //
  void Seed(unsigned n) {rnd.Seed(n);}
  const double *NextSample(int i);
  bool IsFlexible() const {return false;}

  //
  // CMaxEstimator
  //
  bool MaxParameter(double vMax[]) const;
};

#endif
