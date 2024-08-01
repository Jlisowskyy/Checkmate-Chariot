/////////////////////////////////////////////////////////////////////////////
//
// CParametricFunction.h
//
// Rémi Coulom
//
// December, 2008
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CParametricFunction_Declared
#define CParametricFunction_Declared

class CParametricFunction // pf
{
 private: ///////////////////////////////////////////////////////////////////
  double PriorStrength;

 protected: /////////////////////////////////////////////////////////////////
  const int Dimensions;
  const int Parameters;

 public: ////////////////////////////////////////////////////////////////////
  CParametricFunction(int Dimensions, int Parameters);

  //
  // Dimensions and Parameters
  //
  int GetDimensions() const {return Dimensions;}
  int GetParameters() const {return Parameters;}

  //
  // Utility function
  //
  double DotProduct(const double *vParam, const double *vx) const;

  //
  // Virtual Functions
  //
  virtual double GetValue(const double *vParam, const double *vx) const;
  virtual bool GetMax(const double *vParam, double *vx) const;
  virtual void GetMonomials(const double *vx, double *vMonomial) const = 0;
  virtual void GetGradient(const double *vParam,
                           const double *vx,
                           double *vG) const = 0;

  //
  // Prior
  //
  virtual double GetPriorStrength() const {return PriorStrength;}
  virtual void SetPriorStrength(double Strength) {PriorStrength = Strength;}

  virtual void GetPriorParam(double *vParam) const;
  virtual double GetPrior(const double *vParam) const;
  virtual void GetPriorGradient(const double *vParam, double *vGradient) const;
  virtual void GetPriorHessian(const double *vParam, double *vHessian) const;

  virtual ~CParametricFunction() {}
};

#endif
