/////////////////////////////////////////////////////////////////////////////
//
// CSPUCB.h
//
// Rémi Coulom
//
// May, 2009
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CSPUCB_Declared
#define CSPUCB_Declared

#include "CSamplingPolicy.h"
#include "CRegression.h"
#include "random.h"

#include <vector>

class CSPUCB: public CSamplingPolicy // ucb
{
 private: ////////////////////////////////////////////////////////////////////
  CRegression &reg;
  const double alpha;

  CRandom<unsigned> rnd;
  std::vector<double> vSample;

 public: ////////////////////////////////////////////////////////////////////
  CSPUCB(CRegression &reg, double alpha):
   reg(reg),
   alpha(alpha),
   vSample(reg.GetPF().GetDimensions())
  {}

  void Seed(unsigned n) {rnd.Seed(n);}

  const double *NextSample(int i);
};

#endif
