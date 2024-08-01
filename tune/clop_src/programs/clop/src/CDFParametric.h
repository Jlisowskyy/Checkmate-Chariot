/////////////////////////////////////////////////////////////////////////////
//
// CDFParametric.h
//
// Rémi Coulom
//
// August, 2011
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CDFParametric_Declared
#define CDFParametric_Declared

#include "CDiffFunction.h"
#include "CParametricFunction.h"

class CDFParametric: public CDiffFunction
{
 private: ///////////////////////////////////////////////////////////////////
  const CParametricFunction &pf;
  const double *pParam;

  std::vector<double> vGradient;
  std::vector<double> vHessian;

 public: ////////////////////////////////////////////////////////////////////
  CDFParametric(const CParametricFunction &pf, const double *pParam):
   CDiffFunction(pf.GetDimensions()),
   pf(pf),
   pParam(pParam),
   vGradient(pf.GetDimensions()),
   vHessian(pf.GetDimensions() * pf.GetDimensions())
  {
  }

  double GetOutput(const double *vInput)
  {
   pf.GetGradient(pParam, vInput, &vGradient[0]);
   return pf.GetValue(pParam, vInput);
  }

  const std::vector<double> &GetGradient() {return vGradient;}
  const std::vector<double> &GetHessian() {return vHessian;}

  double Normalize(double x) const
  {
   if (x < -1.0)
    return -1.0;
   else if (x > 1.0)
    return 1.0;
   else
    return x;
  }
};

#endif
