/////////////////////////////////////////////////////////////////////////////
//
// CTikZOneD.h
//
// Rémi Coulom
//
// April, 2009
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CTikZOneD_Declared
#define CTikZOneD_Declared

#include "CTikZ.h"

class CRegression;
class CArtificialProblem;

class CTikZOneD: public CTikZ // tikzone
{
 private: //////////////////////////////////////////////////////////////////
  double Tickp;
  double Tickm;

  bool fTrue;
  bool fPosterior;
  bool fMAP;
  bool fExpected;
  bool fConfidence;
  bool fLCB;

  void PlotProblem(const CArtificialProblem &problem) const;

 protected: ////////////////////////////////////////////////////////////////
  void PlotResult(const CResults &results, int i) const;

 public: ///////////////////////////////////////////////////////////////////
  CTikZOneD();

  //
  // Write TikZ to output stream
  //
  void BeginClip() const;
  void EndClip() const;

  void Posterior(int n, CRegression &reg, CRandom<unsigned> &rnd);
  void PlotTrue(const CArtificialProblem &problem);
  void MAP(CRegression &reg);
  void Expected(CRegression &reg, int Samples, int Seed = 0);
  void Confidence(CRegression &reg, double Alpha = -1.96);
  void LCB(CRegression &reg);

  void Frame(int Samples) const;
  void Key() const;
};

#endif
