/////////////////////////////////////////////////////////////////////////////
//
// CAPDF.h
//
// Rémi Coulom
//
// April, 2009
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CAPDF_Declared
#define CAPDF_Declared

#include "CArtificialProblem.h"
#include "CDiffFunction.h"

class CAPDF: public CArtificialProblem
{
 private: ///////////////////////////////////////////////////////////////////
  CDiffFunction &df;

 public: ////////////////////////////////////////////////////////////////////
  explicit CAPDF(CDiffFunction &df): df(df) {}

  double GetProba(const double v[]) const
  {
   return df.GetOutput(v);
  }

  int GetDimensions() const {return df.GetDimensions();}
  double GetStrength(const double v[]) const {return 0.0;}
  void GetOptimalParameters(double v[]) const {}
};

#endif
