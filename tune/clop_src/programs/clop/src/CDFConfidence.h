/////////////////////////////////////////////////////////////////////////////
//
// CDFConfidence.h
//
// Rémi Coulom
//
// April, 2009
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CDFConfidence_Declared
#define CDFConfidence_Declared

#include "CDFVariance.h"

class CDFConfidence: public CDFVariance // dfconf
{
 protected: /////////////////////////////////////////////////////////////////
  std::vector<double> vz;
  std::vector<double> vZ;
  std::vector<double> vX;
  std::vector<double> vSigmaX;

  double r;
  double var;
  double dev;

  void ComputeZ();

 public: ////////////////////////////////////////////////////////////////////
  explicit CDFConfidence(CRegression &reg);
  void ComputeVariance(const double *vInput);
  double GetDeviation() const {return dev;}
  double GetVariance() const {return var;}
};

#endif
