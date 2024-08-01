/////////////////////////////////////////////////////////////////////////////
//
// CPVolcano.h
//
// Rémi Coulom
//
// July, 2011
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CPVolcano_Declared
#define CPVolcano_Declared

#include "CArtificialProblem.h"

class CPVolcano: public CArtificialProblem // volcano
{
 private: ///////////////////////////////////////////////////////////////////
  const int dimensions;
  const double alpha;
  const double x_star;

 public: ////////////////////////////////////////////////////////////////////
  CPVolcano(int dimensions = 2, double alpha = 0.3);

  double GetStrength(const double v[]) const;
  int GetDimensions() const {return dimensions;}
  void GetOptimalParameters(double v[]) const;
};

#endif
