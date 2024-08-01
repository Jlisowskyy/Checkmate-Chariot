/////////////////////////////////////////////////////////////////////////////
//
// CDFVarianceDelta.h
//
// Rémi Coulom
//
// November, 2008
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CDFVarianceDelta_Declared
#define CDFVarianceDelta_Declared

#include "CDFVariance.h"

class CDFVarianceDelta: public CDFVariance // dfvarDelta
{
 private: ///////////////////////////////////////////////////////////////////
  std::vector<double> vz;
  std::vector<double> vX;
  std::vector<double> vSigmaX;
  std::vector<double> vH; // dummy: this is not computed

  double r;
  double s;
  double var;

 public: ////////////////////////////////////////////////////////////////////
  explicit CDFVarianceDelta(CRegression &reg);

  double GetOutput(const std::vector<double> &vInput);
  void ComputeGradient();
  double GetVariance() const {return var;}
};

#endif
