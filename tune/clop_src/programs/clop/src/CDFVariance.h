/////////////////////////////////////////////////////////////////////////////
//
// CDFVariance.h
//
// Rémi Coulom
//
// March, 2009
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CDFVariance_Declared
#define CDFVariance_Declared

#include "CDiffFunction.h"
#include "random.h"

class CRegression;

class CDFVariance: public CDiffFunction // dfvar
{
 protected: /////////////////////////////////////////////////////////////////
  CRegression &reg;
  const int Dimensions;
  const int Parameters;

  const double *pvx;
  std::vector<double> vGradient;

  void CholeskySolve(std::vector<double> &v);

 private: ///////////////////////////////////////////////////////////////////
  std::vector<double> vH; // dummy: this is not computed
  int MinSamples;

 public: ////////////////////////////////////////////////////////////////////
  explicit CDFVariance(CRegression &reg);

  double GetOutput(const double *v) {return 0.0;}

  void ComputeGradient();
  const std::vector<double> &GetGradient() {return vGradient;}
  const std::vector<double> &GetHessian() {return vH;}
  double Normalize(double x) const;

  virtual void SelectAtRandom(CRandom<unsigned> &rnd) {}
  virtual double GetVariance() const {return 0;}

  int GetMinSamples() const {return MinSamples;}
  void SetMinSamples(int n) {MinSamples = n;}
};

#endif
