/////////////////////////////////////////////////////////////////////////////
//
// CMERegressionMAPMax.h
//
// Rémi Coulom
//
// December, 2008
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CMERegressionMAPMax_Declared
#define CMERegressionMAPMax_Declared

#include "CMaxEstimator.h"
#include "CRegression.h"

class CMERegressionMAPMax: public CMaxEstimator // mermm
{
 private: ///////////////////////////////////////////////////////////////////
  CRegression &reg;

 public: ////////////////////////////////////////////////////////////////////
  explicit CMERegressionMAPMax(CRegression &reg):
   reg(reg)
  {}

  bool MaxParameter(double vMax[]) const
  {
   reg.GetAverageSample(vMax);

   if (reg.GetPF().GetMax(&reg.MAP()[0], &vMax[0]) &&
       reg.GetWeight(vMax) == 1.0)
    return true;
   else
   {
    reg.GetAverageSample(vMax);
    return true;
   }
  }
};

#endif
