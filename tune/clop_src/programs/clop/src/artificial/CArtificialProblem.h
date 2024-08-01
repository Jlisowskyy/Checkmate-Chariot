/////////////////////////////////////////////////////////////////////////////
//
// CArtificialProblem.h
//
// Rémi Coulom
//
// October, 2008
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CArtificialProblem_Declared
#define CArtificialProblem_Declared

#include "random.h"
#include "COutcome.h"

class CArtificialProblem // aproblem
{
 private: ///////////////////////////////////////////////////////////////////
  CRandom<unsigned> rnd;

 public: ////////////////////////////////////////////////////////////////////
  COutcome GetOutcome(const double v[]);

  void Seed(unsigned n) {rnd.Seed(n);}

  virtual int GetDimensions() const = 0;
  virtual double GetStrength(const double v[]) const = 0;
  virtual double GetProba(const double v[]) const;

  virtual double GetOptimalProba() const;

  virtual void GetOptimalParameters(double v[]) const = 0;

  virtual ~CArtificialProblem() {}
};

#endif
