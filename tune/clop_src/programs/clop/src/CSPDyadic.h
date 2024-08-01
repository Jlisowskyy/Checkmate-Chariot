/////////////////////////////////////////////////////////////////////////////
//
// CSPDyadic.h
//
// Rémi Coulom
//
// May, 2010
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CSPDyadic_Declared
#define CSPDyadic_Declared

#include "CSamplingPolicy.h"

#include <vector>

class CSPDyadic: public CSamplingPolicy
{
 private:
  std::vector<double> v;

 public:
  explicit CSPDyadic(int Dimensions): v(Dimensions) {}
  const double *NextSample(int i);
};

#endif
