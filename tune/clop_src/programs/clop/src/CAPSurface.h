/////////////////////////////////////////////////////////////////////////////
//
// CAPSurface.h
//
// Rémi Coulom
//
// January, 2008
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CAPSurface_Declared
#define CAPSurface_Declared

#include "CSurface.h"
#include "CArtificialProblem.h"

class CAPSurface: public CSurface // apsurf
{
 private: ///////////////////////////////////////////////////////////////////
  const CArtificialProblem &aproblem;

 public: ////////////////////////////////////////////////////////////////////
  explicit CAPSurface(const CArtificialProblem &aproblem):
   CSurface(0),
   aproblem(aproblem)
  {
  }

  double GetValue(double x, double y) const
  {
   double v[2];
   v[0] = x;
   v[1] = y;
   return aproblem.GetProba(v);
  }
};

#endif
