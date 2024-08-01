/////////////////////////////////////////////////////////////////////////////
//
// CDFLogP.h
//
// Rémi Coulom
//
// November, 2008
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CDFLogP_Declared
#define CDFLogP_Declared

#include "CDiffFunction.h"
#include "CRegression.h"

class CDFLogP: public CDiffFunction // dflogp
{
 private: ///////////////////////////////////////////////////////////////////
  CRegression &reg;

 public: ////////////////////////////////////////////////////////////////////
  explicit CDFLogP(CRegression &reg):
   CDiffFunction(reg.GetPF().GetParameters()),
   reg(reg)
  {}

  const std::vector<double> &GetGradient() {return reg.vGradient;}
  const std::vector<double> &GetHessian() {return reg.vHessian;}

  double GetOutput(const double *vInput)
  {
   for (int i = reg.GetPF().GetParameters(); --i >= 0;)
    if (vInput[i] != reg.vParamMAP[i])
    {
     reg.vParamMAP[i] = vInput[i];
     reg.State = 0;
    }
   reg.EnsureState(CRegression::S_LogP);
   return reg.L;
  }

  void ComputeGradient() {reg.EnsureState(CRegression::S_Gradient);}
  void ComputeHessian() {reg.EnsureState(CRegression::S_Hessian);}
};

#endif
