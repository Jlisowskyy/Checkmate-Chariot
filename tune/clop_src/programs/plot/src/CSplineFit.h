/////////////////////////////////////////////////////////////////////////////
//
// CSplineFit.h
//
// Rémi Coulom
// December, 2001
//
/////////////////////////////////////////////////////////////////////////////
#ifndef Plot_CSplineFit_Declared
#define Plot_CSplineFit_Declared

#include "random.h"

class CBezier;
class CDiscretizedLine;

#include <iosfwd>
#include <vector>

class CSplineFit // sfit
{
 private: /////////////////////////////////////////////////////////////////
  CRandom<unsigned> rnd;

  const CDiscretizedLine &dl;

  int Points;
  std::vector<int> vIndex;

  double GetError(int i, const CBezier &bez) const;
  mutable int jMax;
  mutable double tMax;

  void GetBezier(int i, CBezier &bez, double u, double v) const;
  mutable double tdx[2];
  mutable double tdy[2];

  static double Normalize(double x);
  double OptimizeUV(int i, CBezier &bez, double DMax) const;

 public: //////////////////////////////////////////////////////////////////
  explicit CSplineFit(const CDiscretizedLine &dlInit);

  void Fit(double DMax);

  void TikZ(int Precision = 2,
            bool fMoveTo = true,
            std::ostream &out = std::cout) const;

  static void PGF(const CDiscretizedLine &dl,
                  double DMax,
                  std::ostream &out = std::cout);
};

#endif
