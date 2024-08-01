/////////////////////////////////////////////////////////////////////////////
//
// CTikZTwoD.h
//
// Rémi Coulom
//
// April, 2009
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CTikZTwoD_Declared
#define CTikZTwoD_Declared

#include "CTikZ.h"

class CRegression;
class CArtificialProblem;

class CTikZTwoD: public CTikZ // tikztwo
{
 private: //////////////////////////////////////////////////////////////////
  int ContourResolution;

 protected: ////////////////////////////////////////////////////////////////
  void PlotResult(const CResults &results, int i) const;

 public: ///////////////////////////////////////////////////////////////////
  CTikZTwoD();

  //
  // Parameters
  //
  void SetContourResolution(int n) {ContourResolution = n;}

  //
  // Write TikZ to output stream
  //
  void BeginClip() const;
  void EndClip() const;

  void Posterior(int Samples, CRegression &reg, CRandom<unsigned> &rnd);
  void PlotTrue(const CArtificialProblem &problem);
  void Contour(const CArtificialProblem &problem, int Levels = 8) const;
  void MAP(CRegression &reg);
  void Key() const;

  void Frame(int Samples = 0, bool fLabels = true) const;
};

#endif
