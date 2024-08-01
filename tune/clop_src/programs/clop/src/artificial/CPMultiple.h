/////////////////////////////////////////////////////////////////////////////
//
// CPMultiple.h
//
// Rémi Coulom
//
// August, 2011
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CPMultiple_Declared
#define CPMultiple_Declared

#include "CArtificialProblem.h"

class CPMultiple: public CArtificialProblem // multi
{
 private: ///////////////////////////////////////////////////////////////////
  const int Multiplier;
  const CArtificialProblem &problem;

 public: ////////////////////////////////////////////////////////////////////
  CPMultiple(int Multiplier, const CArtificialProblem &problem):
   Multiplier(Multiplier),
   problem(problem)
  {}

  double GetStrength(const double v[]) const;
  int GetDimensions() const {return Multiplier * problem.GetDimensions();}
  void GetOptimalParameters(double v[]) const;
};

#endif
