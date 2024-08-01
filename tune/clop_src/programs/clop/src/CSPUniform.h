/////////////////////////////////////////////////////////////////////////////
//
// CSPUniform.h
//
// Rémi Coulom
//
// October, 2008
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CSPUniform_Declared
#define CSPUniform_Declared

#include "CSamplingPolicy.h"
#include "random.h"

#include <vector>

class CSPUniform: public CSamplingPolicy // unif
{
 private: ////////////////////////////////////////////////////////////////////
  std::vector<double> v; 
  CRandom<unsigned> rnd;
  const double Min;
  const double Max;

 public: ////////////////////////////////////////////////////////////////////
  explicit CSPUniform(int Dimensions, double Min = -1.0, double Max = 1.0);
  void Seed(unsigned n) {rnd.Seed(n);}
  const double *NextSample(int i);
};

#endif
