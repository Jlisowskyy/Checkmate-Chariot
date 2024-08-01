/////////////////////////////////////////////////////////////////////////////
//
// CDiffFunction.h
//
// Rémi Coulom
//
// November, 2008
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CDiffFunction_Declared
#define CDiffFunction_Declared

#include <vector>

class CDiffFunction // func
{
 private: ///////////////////////////////////////////////////////////////////
  double SetLineInput(const double vx0[], const double vDir[], double x);

 protected: /////////////////////////////////////////////////////////////////
  const int Dimensions;
  std::vector<double> vCholesky;
  std::vector<double> vStep;
  std::vector<double> vGCopy;
  std::vector<double> vxTemp;

 public: ////////////////////////////////////////////////////////////////////

  explicit CDiffFunction(int Dimensions):
   Dimensions(Dimensions),
   vCholesky(Dimensions * Dimensions),
   vStep(Dimensions),
   vGCopy(Dimensions),
   vxTemp(Dimensions)
  {}

  int GetDimensions() const {return Dimensions;}

  virtual double GetOutput(const double *vInput) = 0;
  virtual void ComputeGradient() {}
  virtual const std::vector<double> &GetGradient() = 0;
  virtual void ComputeHessian() {}
  virtual const std::vector<double> &GetHessian() = 0;

  virtual double Normalize(double x) const {return x;}

  void Newton(std::vector<double> &vMax, bool fTrace = false);
  void SteepestDescent(std::vector<double> &vMax, bool fTrace = false);
  void CG(double vMax[], bool fTrace = false);

  double LineOpt(const double vx0[],
                 const double vDir[],
                 bool fTrace = false);

  virtual ~CDiffFunction() {}
};

#endif
