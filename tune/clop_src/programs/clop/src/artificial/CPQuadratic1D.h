/////////////////////////////////////////////////////////////////////////////
//
// CPQuadratic1D.h
//
// Rémi Coulom
//
// October, 2008
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CPQuadratic1D_Declared
#define CPQuadratic1D_Declared

#include "CArtificialProblem.h"

class CPQuadratic1D: public CArtificialProblem // q1d
{
 private: ///////////////////////////////////////////////////////////////////
  const double x0;
  const double Strength;
  const double Steepness;

 public: ////////////////////////////////////////////////////////////////////
  CPQuadratic1D(double x0, double Strength, double Steepness);

  double GetAlpha() const {return Strength;}
  double GetBeta() const {return -Steepness;}
  double GetMu() const {return x0;}

  double GetStrength(const double v[]) const;
  void GetOptimalParameters(double v[]) const;
  int GetDimensions() const {return 1;}
};

#endif
