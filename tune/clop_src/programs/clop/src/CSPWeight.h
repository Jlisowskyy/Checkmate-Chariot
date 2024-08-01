/////////////////////////////////////////////////////////////////////////////
//
// CSPWeight.h
//
// Rémi Coulom
//
// May, 2010
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CSPWeight_Declared
#define CSPWeight_Declared

#include "CSamplingPolicy.h"
#include "random.h"

#include <vector>

class CRegression;

class CSPWeight: public CSamplingPolicy // spw
{
 private: ////////////////////////////////////////////////////////////////////
  std::vector<double> vResult; 
  const CRegression &reg;
  const int ReplicationThreshold;
  const int nMCMC;
  CRandom<unsigned> rnd;

 public: ////////////////////////////////////////////////////////////////////
  CSPWeight(const CRegression &reg,
            int ReplicationThreshold = 0,
            int nMCMC = 100);
  void Seed(unsigned n) {rnd.Seed(n);}
  const double *NextSample(int i);
};

#endif
