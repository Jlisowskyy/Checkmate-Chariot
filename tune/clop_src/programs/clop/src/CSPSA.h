/////////////////////////////////////////////////////////////////////////////
//
// CSPSA.h
//
// Rémi Coulom
//
// August, 2011
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CSPSA_Declared
#define CSPSA_Declared

#include "CSamplingPolicy.h"
#include "CMaxEstimator.h"
#include "CObserver.h"
#include "BobJenkinsRNG.h"

#include <vector>

class CSPSA:
 public CSamplingPolicy,
 public CMaxEstimator,
 private CObserver
{
 private: ///////////////////////////////////////////////////////////////////
  BobJenkinsRNG rnd;

  //
  // Value by outcome index
  //
  static const double tOutcomeValue[];

  //
  // Constant parameters
  //
  const int Dimensions;
  const double a;
  const double A;
  const double c;
  const double alpha;
  const double gamma;

  //
  // State of the optimization
  //
  int k;
  double yPlus;
  double c_k;
  std::vector<double> vTheta;
  std::vector<double> vDelta;
  std::vector<double> vThetaPlusMinus;

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
  explicit CSPSA(CResults &results,
                 double a = 0.05,
                 double A = 0.0,
                 double c = 0.00001,
                 double alpha = 0.602,  // asymptotically optimal = 1
                 double gamma = 0.101); // asymptotically optimal = 1/6

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
