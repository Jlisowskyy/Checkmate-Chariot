/////////////////////////////////////////////////////////////////////////////
//
// CSPCOptimal.h
//
// C-optimal sampling policy, according to the thesis of E. Fackle Fornius
// 1D quadratic problems only, Beta must not be zero
//
// Rémi Coulom
//
// July, 2009
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CSPCOptimal_Declared
#define CSPCOptimal_Declared

#include "CSamplingPolicy.h"

#include <vector>

class CPQuadratic1D;

class CSPCOptimal: public CSamplingPolicy // copt
{
 private: ////////////////////////////////////////////////////////////////////
  const double Alpha;
  const double Beta;
  const double Mu;
  const double s; // sqrt(t_mu/Beta)

  int N;
  std::vector<double> v;

  double f(double x) const;
  double fprime(double x) const;
  double Solve() const;

 public: ////////////////////////////////////////////////////////////////////
  explicit CSPCOptimal(const CPQuadratic1D &q1d);
  explicit CSPCOptimal(const double *vParam);

  double GetMin() const {return Mu - s;}
  double GetMax() const {return Mu + s;}

  const double *NextSample(int i);
};

#endif
