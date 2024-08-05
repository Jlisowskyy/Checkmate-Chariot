/////////////////////////////////////////////////////////////////////////////
//
// CPMultiple.cpp
//
// Rémi Coulom
//
// August, 2011
//
/////////////////////////////////////////////////////////////////////////////
#include "CPMultiple.h"

/////////////////////////////////////////////////////////////////////////////
// GetStrength
/////////////////////////////////////////////////////////////////////////////
double CPMultiple::GetStrength(const double v[]) const
{
 double Result = 0.0;
 for (int j = Multiplier; --j >= 0;)
  Result += problem.GetStrength(v + j * problem.GetDimensions());
 return Result / Multiplier;
}

/////////////////////////////////////////////////////////////////////////////
// GetOptimalParameters
/////////////////////////////////////////////////////////////////////////////
void CPMultiple::GetOptimalParameters(double v[]) const
{
 problem.GetOptimalParameters(v);
 for (int j = Multiplier; --j > 0;)
  for (int i = problem.GetDimensions(); --i >= 0;)
   v[i + j * problem.GetDimensions()] = v[i];
}