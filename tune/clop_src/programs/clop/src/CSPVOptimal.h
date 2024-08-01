/////////////////////////////////////////////////////////////////////////////
//
// CSPVOptimal.h
//
// Rémi Coulom
//
// November, 2008
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CSPVOptimal_Declared
#define CSPVOptimal_Declared

#include "CSamplingPolicy.h"
#include "CDFVariance.h"
#include "random.h"
#include "CRegression.h"
#include "CSPWeight.h"

class CSPVOptimal: public CSamplingPolicy // vopt
{
 private: ////////////////////////////////////////////////////////////////////
  const CRegression &reg;
  CDFVariance &dfvar;
  const int Trials;

  CRandom<unsigned> rnd;
  std::vector<double> v;
  std::vector<double> vTrial;

  int nTotal;

  CSPWeight spCG;
  CSPWeight spFail;

 public: ////////////////////////////////////////////////////////////////////
  CSPVOptimal(const CRegression &reg,
              CDFVariance &dfvar,
              int Trials = 0);
  void Seed(unsigned n);
  const double *NextSample(int i);
  void Reserve(unsigned n) {reg.Reserve(n);}
};

#endif
