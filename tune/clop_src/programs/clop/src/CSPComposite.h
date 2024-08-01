/////////////////////////////////////////////////////////////////////////////
//
// CSPComposite.h
//
// Rémi Coulom
//
// May, 2009
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CSPComposite_Declared
#define CSPComposite_Declared

#include "CSamplingPolicy.h"

class CRegression;

class CSPComposite: public CSamplingPolicy // comp
{
 private: ////////////////////////////////////////////////////////////////////
  const CRegression &reg;
  CSamplingPolicy &sp1;
  CSamplingPolicy &sp2;
  const int MinSamples;

 public: ////////////////////////////////////////////////////////////////////
  CSPComposite(const CRegression &reg,
               CSamplingPolicy &sp1,
               CSamplingPolicy &sp2,
               int MinSamples):
   reg(reg),
   sp1(sp1),
   sp2(sp2),
   MinSamples(MinSamples)
  {}

  void Seed(unsigned n) {sp1.Seed(n); sp2.Seed(n + 1);}
  const double *NextSample(int i);
};

#endif
