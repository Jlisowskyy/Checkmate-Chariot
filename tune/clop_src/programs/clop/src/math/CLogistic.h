/////////////////////////////////////////////////////////////////////////////
//
// CLogistic.h
//
// Rémi Coulom
//
// April, 2009
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CLogistic_Declared
#define CLogistic_Declared

#include <cmath>

struct CLogistic
{
 static double f(double x)
 {
  return 1.0 / (1.0 + std::exp(-x));
 }

 static double Difference(double x1, double x0)
 {
  if (x1 < 0)
   return f(x1) - f(x0);
  else
   return f(-x0) - f(-x1);
 }
 
 static double Derivative(double x)
 {
  double h = 0.5 * x;
  double s = std::exp(h) + std::exp(-h);
  return 1.0 / (s * s);
 }

 static double Inverse(double x)
 {
  return -std::log(1.0 / x - 1.0);
 }
};

#endif
