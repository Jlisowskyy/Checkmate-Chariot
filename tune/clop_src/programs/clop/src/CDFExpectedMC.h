/////////////////////////////////////////////////////////////////////////////
//
// CDFExpectedMC.h
//
// Rémi Coulom
//
// April, 2009
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CDFExpectedMC_Declared
#define CDFExpectedMC_Declared

#include "CDiffFunction.h"

class CRegression;

class CDFExpectedMC: public CDiffFunction // dfexp
{
 private: ///////////////////////////////////////////////////////////////////
  CRegression &reg;
  std::vector<std::vector<double> > vv;
  std::vector<double> vDummy;

 public: ////////////////////////////////////////////////////////////////////
  CDFExpectedMC(CRegression &reg, int Samples, int Seed = 0);

  double GetOutput(const double *vInput);
  const std::vector<double> &GetGradient() {return vDummy;}
  const std::vector<double> &GetHessian() {return vDummy;}
};

#endif
