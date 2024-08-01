/////////////////////////////////////////////////////////////////////////////
//
// CTikZ.h
//
// Rémi Coulom
//
// April, 2009
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CTikZ_Declared
#define CTikZ_Declared

#include <iosfwd>
#include <string>
#include <map>

#include "random.h"
#include "CColor.h"

class CResults;
class CRegression;

class CTikZ // tikz
{
 protected: ////////////////////////////////////////////////////////////////
  std::ostream *pout;

  double Scale;
  double Radius;
  double Tick;
  int CircleN;
  bool fGrayscale;
  int SplineSamples;
  double SplineD;
  bool fResults;

  struct CStyle
  {
   std::string sStyle;
   CColor col;

   CStyle(float R, float G, float B, const std::string &sStyle):
    sStyle(sStyle),
    col(CColor(R, G, B))
   {}
  };

  std::map<std::string, CStyle> mapStyle;

  void AddStyle(const std::string &s,
                float R,
                float G,
                float B,
                const std::string &sStyle);

  virtual void PlotResult(const CResults &results, int i) const = 0;

 public: ///////////////////////////////////////////////////////////////////
  CTikZ();

  void SetOutput(std::ostream &out) {pout = &out;}
  void SetScale(double NewScale) {Scale = NewScale;}
  void SetStyle(const std::string &s,
                float R,
                float G,
                float B,
                const std::string &sStyle);
  void SetGrayscale(bool f) {fGrayscale = f;}
  void SetCircleN(int n) {CircleN = n;}
  void SetSplineSamples(int Samples) {SplineSamples = Samples;}
  void SetSplineD(double D) {SplineD = D;}

  virtual void Prolog() const;
  virtual void Epilog() const;
  virtual void Results(const CResults &results, const CRegression *preg = 0);

  void Key(const std::string &s) const;

  virtual ~CTikZ() {}
};

#endif
