/////////////////////////////////////////////////////////////////////////////
//
// CTikZTwoD.cpp
//
// Rémi Coulom
//
// April, 2009
//
/////////////////////////////////////////////////////////////////////////////
#include "CTikZTwoD.h"
#include "CResults.h"
#include "CRegression.h"
#include "CArtificialProblem.h"
#include "CPGF.h"
#include "CAPSurface.h"
#include "CContour.h"

#include <iostream>

/////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////
CTikZTwoD::CTikZTwoD():
 ContourResolution(400)
{
 Scale = 2.5;
 Radius = 0.02;
 Tick = 0.04;
 CircleN = 2;
}

/////////////////////////////////////////////////////////////////////////////
// Beginning of clipping region
/////////////////////////////////////////////////////////////////////////////
void CTikZTwoD::BeginClip() const
{
 *pout << "\\begin{scope}\n";
 *pout << "\\clip (" << -Scale << ',' << -Scale << ") rectangle (";
 *pout << Scale << "," << Scale << ");\n";
}

/////////////////////////////////////////////////////////////////////////////
// End of clipping region
/////////////////////////////////////////////////////////////////////////////
void CTikZTwoD::EndClip() const
{
 *pout << "\\end{scope}\n";
}

/////////////////////////////////////////////////////////////////////////////
// Results
/////////////////////////////////////////////////////////////////////////////
void CTikZTwoD::PlotResult(const CResults &results, int i) const
{
 const double *v = results.GetSample(i);
 CPGF::Circle(*pout, CircleN, v[0] * Scale, v[1] * Scale, Radius * Scale);
}

/////////////////////////////////////////////////////////////////////////////
// Posterior Samples
/////////////////////////////////////////////////////////////////////////////
void CTikZTwoD::Posterior(int Samples,
                          CRegression &reg,
                          CRandom<unsigned> &rnd)
{
 *pout << "\\begin{scope}[green]\n";
 {
  std::vector<double> vParam(reg.GetPF().GetParameters());
  std::vector<double> v(2);

  for (int i = Samples; --i >= 0;)
  {
   reg.GaussianSample(rnd, vParam);
   reg.GetPF().GetMax(&vParam[0], &v[0]);
   CPGF::Cross(*pout, v[0] * Scale, v[1] * Scale, Radius * Scale);
  }
 }
 *pout << "\\pgfusepathqstroke\n";
 *pout << "\\end{scope}\n";
}

/////////////////////////////////////////////////////////////////////////////
// True probability
/////////////////////////////////////////////////////////////////////////////
void CTikZTwoD::PlotTrue(const CArtificialProblem &problem)
{
 *pout << "\\begin{scope}[thick, red]\n";
 {
  double v[2];
  problem.GetOptimalParameters(v);
  CPGF::Cross(*pout, v[0] * Scale, v[1] * Scale, Radius * Scale);
 }
 *pout << "\\pgfusepathqstroke\n";
 *pout << "\\end{scope}\n";
}

/////////////////////////////////////////////////////////////////////////////
// Contour plot of true probability
/////////////////////////////////////////////////////////////////////////////
void CTikZTwoD::Contour(const CArtificialProblem &problem, int Levels) const
{
 *pout << "\\begin{scope}[Contour]\n";

 CAPSurface surf(problem);
 CContour contour(surf);
 contour.SetBounds(-1.0, 1.0, -1.0, 1.0);
 contour.SetGridSize(ContourResolution, ContourResolution);

 for (int i = 1; i < Levels; i++)
 {
  double x = double(i) / double(Levels);
  contour.SetLevel(x);
  contour.PlotLines(*pout, SplineD, Scale);
 }

 *pout << "\\pgfusepathqstroke\n";
 *pout << "\\end{scope}\n";
}

/////////////////////////////////////////////////////////////////////////////
// MAP
/////////////////////////////////////////////////////////////////////////////
void CTikZTwoD::MAP(CRegression &reg)
{
 *pout << "\\begin{scope}[red]\n";
 {
  std::vector<double> v(2);
  reg.GetPF().GetMax(&reg.MAP()[0], &v[0]);
  CPGF::Cross(*pout, v[0] * Scale, v[1] * Scale, Radius * Scale);
 }
 *pout << "\\pgfusepathqstroke\n";
 *pout << "\\end{scope}\n";
}

/////////////////////////////////////////////////////////////////////////////
// Frame
/////////////////////////////////////////////////////////////////////////////
void CTikZTwoD::Frame(int Samples, bool fLabels) const
{
 *pout << "\\draw[line width=0.2pt] (" << -Scale << ',' << -Scale << ") rectangle (";
 *pout << Scale << "," << Scale << ");\n";
 if (fLabels)
 {
  *pout <<  "\\scriptsize\n";
  *pout <<  "\\draw (-" << Scale << ",-" << Scale * (1.0 + Tick) << ") -- (-" << Scale << "," << -Scale << ");\n"
            "\\draw (+" << Scale << ",-" << Scale * (1.0 + Tick) << ") -- (+" << Scale << "," << -Scale << ");\n"
            "\\path (-" << Scale << ",-" << Scale * (1.0 + Tick) << ") -- (+" << Scale << ",-" << Scale * (1.0 + Tick) << ")\n"
            "      node[below,midway] {$x_1$}\n"
            "      node[below,pos=0] {-1}\n"
            "      node[below,pos=1] {1};\n";

  *pout <<  "\\draw (-" << Scale * (1.0 + Tick) << ",-" << Scale << ") -- (-" << Scale << "," << -Scale << ");\n"
            "\\draw (-" << Scale * (1.0 + Tick) << ",+" << Scale << ") -- (-" << Scale << "," << +Scale << ");\n"
            "\\path (-" << Scale * (1.0 + Tick) << ",-" << Scale << ") -- (-" << Scale * (1.0 + Tick) << "," << Scale << ")\n"
            "      node[left,midway] {$x_2$}\n"
            "      node[left,pos=0] {-1}\n"
            "      node[left,pos=1] {1};\n";
 }
}

/////////////////////////////////////////////////////////////////////////////
// Key
/////////////////////////////////////////////////////////////////////////////
void CTikZTwoD::Key() const
{
}
