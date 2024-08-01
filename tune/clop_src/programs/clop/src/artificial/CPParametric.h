/////////////////////////////////////////////////////////////////////////////
//
// CPParametric.h
//
// Rémi Coulom
//
// October, 2008
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CPParametric_Declared
#define CPParametric_Declared

#include "CArtificialProblem.h"
#include "CParametricFunction.h"

#include <vector>

class CPParametric: public CArtificialProblem // pparam
{
 private: ///////////////////////////////////////////////////////////////////
  const CParametricFunction &pf;
  const double *vParam;

 public: ////////////////////////////////////////////////////////////////////
  CPParametric(const CParametricFunction &pf, const double *vParam):
   pf(pf),
   vParam(vParam)
  {
  }

  double GetStrength(const double v[]) const
  {
   return pf.GetValue(vParam, v);
  }

  void GetOptimalParameters(double v[]) const
  {
   pf.GetMax(vParam, v);
  }

  int GetDimensions() const {return pf.GetDimensions();}
};

#endif
