/////////////////////////////////////////////////////////////////////////////
//
// CSPNiederreiter.h
//
// Rémi Coulom
//
// March, 2009
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CSPNiederreiter_Declared
#define CSPNiederreiter_Declared

#include "CSPQuasiRandom.h"

class CSPNiederreiter: public CSPQuasiRandom
{
 public:
  explicit CSPNiederreiter(int Dimensions): CSPQuasiRandom(Dimensions)
  {
   q = gsl_qrng_alloc(gsl_qrng_niederreiter_2, Dimensions);
  }
};

#endif
