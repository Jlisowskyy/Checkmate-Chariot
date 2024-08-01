/////////////////////////////////////////////////////////////////////////////
//
// CRSPSA.h
//
// Rémi Coulom
//
// August, 2011
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CRSPSA_Declared
#define CRSPSA_Declared

#include "CSamplingPolicy.h"
#include "CMaxEstimator.h"
#include "CObserver.h"
#include "BobJenkinsRNG.h"

#include <vector>

class CRSPSA:
 public CSamplingPolicy,
 public CMaxEstimator,
 private CObserver
{
 private: ///////////////////////////////////////////////////////////////////
  //
  // Dimensions
  //
  const int Dimensions;

  BobJenkinsRNG rnd;

  //
  // Value by outcome index
  //
  static const double tOutcomeValue[];

  //
  // Constant parameters
  //
  const int BatchSize;
  const double BatchGrowth;
  const double EtaPlus;
  const double EtaMinus;
  const double DeltaMin;
  const double DeltaMax;
  const double Delta0;
  const double Rho;

  //
  // State of the optimization
  //
  int k;
  int CurrentBatchSize;
  int NextBatch;
  std::vector<double> vTheta;
  std::vector<double> vDelta;
  std::vector<double> vPerturbation;
  std::vector<double> vPreviousGradient;
  std::vector<double> vCurrentGradient;
  std::vector<double> vSample;

  //
  // CObserver
  //
  void OnOutcome(int i);
  void OnReset();

  //
  // Useful function
  //
  static void Bound(double &x, double Max)
  {
   if (x < -Max)
    x = -Max;
   else if (x > Max)
    x = Max;
  }

 public: ////////////////////////////////////////////////////////////////////
  explicit CRSPSA(CResults &results,
                  int BatchSize = 100,
                  double BatchGrowth = 1.15,
                  double EtaPlus = 1.5,
                  double EtaMinus = 0.4,
                  double DeltaMin = 0.001,
                  double DeltaMax = 0.5,
                  double Delta0 = 0.5,
                  double Rho = 2.0);

  //
  // CSamplingPolicy
  //
  void Seed(unsigned n) {rnd = BobJenkinsRNG(n);}
  const double *NextSample(int i);
  bool IsFlexible() const {return false;}

  //
  // CMaxEstimator
  //
  bool MaxParameter(double vMax[]) const;
};

#endif
