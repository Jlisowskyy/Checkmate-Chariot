/////////////////////////////////////////////////////////////////////////////
//
// CSPQLRMax.h
//
// Rémi Coulom
//
// November, 2008
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CSPQLRMax_Declared
#define CSPQLRMax_Declared

#include "CSamplingPolicy.h"
#include "CSPDyadic.h"
#include "random.h"

class CRegression;

class CSPQLRMax: public CSamplingPolicy // qlrmax
{
 private: ////////////////////////////////////////////////////////////////////
  CRegression &reg;
  CRandom<unsigned> rnd;
  std::vector<double> vMax;
  int Retries;

  CSPDyadic sp;

 public: ////////////////////////////////////////////////////////////////////
  explicit CSPQLRMax(CRegression &reg);
  void Seed(unsigned n) {rnd.Seed(n); sp.Seed(n);}
  const double *NextSample(int i);
};

#endif
