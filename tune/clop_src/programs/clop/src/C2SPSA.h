/////////////////////////////////////////////////////////////////////////////
//
// C2SPSA.h
//
// Rémi Coulom
//
// August, 2011
//
/////////////////////////////////////////////////////////////////////////////
#ifndef C2SPSA_Declared
#define C2SPSA_Declared

#include "CSamplingPolicy.h"
#include "CMaxEstimator.h"
#include "CObserver.h"
#include "BobJenkinsRNG.h"

#include <vector>

class C2SPSA:
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
  const double d;
  const double w;

  //
  // State of the optimization
  //
  int ExpectedIndex;
  double k;
  enum {yplus = 0, yminus = 1, yplustilda = 2, yminustilda = 3};
  double ty[4];
  double c_k;
  double ctilda_k;
  std::vector<double> vTheta;
  std::vector<double> vDelta;
  std::vector<double> vDeltaTilda;
  std::vector<double> vSample;
  std::vector<double> vGHat;
  std::vector<double> vDeltaGHat;
  std::vector<double> vHHat;
  std::vector<double> vHBar;
  std::vector<double> vHBarBar;

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
  explicit C2SPSA(CResults &results,
                 double a = 0.05,
                 double A = 0.0,
                 double c = 0.00001,
                 double alpha = 1.0,
                 double gamma = 1.0 / 6.0,
                 double d = 0.501,
                 double w = 0.1);

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
