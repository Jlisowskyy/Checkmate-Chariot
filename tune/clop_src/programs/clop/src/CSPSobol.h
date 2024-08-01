/////////////////////////////////////////////////////////////////////////////
//
// CSPSobol.h
//
// Rémi Coulom
//
// March, 2009
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CSPSobol_Declared
#define CSPSobol_Declared

#include "CSPQuasiRandom.h"

class CSPSobol: public CSPQuasiRandom
{
 public:
  CSPSobol(int Dimensions): CSPQuasiRandom(Dimensions)
  {
   q = gsl_qrng_alloc(gsl_qrng_sobol, Dimensions);
  }
};

#endif
