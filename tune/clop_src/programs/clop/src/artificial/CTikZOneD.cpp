/////////////////////////////////////////////////////////////////////////////
//
// CTikZOneD.cpp
//
// Rémi Coulom
//
// April, 2009
//
/////////////////////////////////////////////////////////////////////////////
#include "CTikZOneD.h"
#include "CResults.h"
#include "CRegression.h"
#include "CArtificialProblem.h"
#include "CPGF.h"
#include "CPParametric.h"
#include "CSplineFit.h"
#include "CDFExpectedMC.h"
#include "CDFConfidenceBound.h"
#include "CAPDF.h"
#include "CDiscretizedLine.h"
#include "CLogistic.h"

#include <iostream>

/////////////////////////////////////////////////////////////////////////////
// Plot 1D Problem
/////////////////////////////////////////////////////////////////////////////
void CTikZOneD::PlotProblem(const CArtificialProblem &problem) const
{
 CDiscretizedLine dl;
 dl.Resize(SplineSamples);

 double v[1];
 for (int i = 0; i < SplineSamples; i++)
 {
  v[0] = -1.0 + (2.0 * i) / (SplineSamples - 1);
  double y = problem.GetProba(v);

  dl.SetX(i) = v[0] * Scale;
  dl.SetY(i) = y * Scale;
 }

 const double D = SplineD * Scale;
 CSplineFit::PGF(dl, D, *pout);
}

/////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////
CTikZOneD::CTikZOneD():
 fTrue(false),
 fPosterior(false),
 fMAP(false),
 fExpected(false),
 fConfidence(false),
 fLCB(false)
{
 Scale = 5.0;
 Tickp = 1.0 + Tick;
 Tickm = 1.0 - Tick;
}

/////////////////////////////////////////////////////////////////////////////
// Beginning of clipping region
/////////////////////////////////////////////////////////////////////////////
void CTikZOneD::BeginClip() const
{
 *pout << "\\begin{scope}\n";
 *pout << "\\clip (" << -Scale << ",0) rectangle (";
 *pout << Scale << "," << Scale << ");\n";
}

/////////////////////////////////////////////////////////////////////////////
// End of clipping region
/////////////////////////////////////////////////////////////////////////////
void CTikZOneD::EndClip() const
{
 *pout << "\\end{scope}\n";
}

/////////////////////////////////////////////////////////////////////////////
// Results
/////////////////////////////////////////////////////////////////////////////
void CTikZOneD::PlotResult(const CResults &results, int i) const
{
 const double *v = results.GetSample(i);
 double y = (i + 1) / double(results.GetSamples() + 1);
 CPGF::Circle(*pout, CircleN, v[0] * Scale, y * Scale, Radius * Scale);
}

/////////////////////////////////////////////////////////////////////////////
// Posterior Samples
/////////////////////////////////////////////////////////////////////////////
void CTikZOneD::Posterior(int Samples,
                          CRegression &reg,
                          CRandom<unsigned> &rnd)
{
 fPosterior = true;

 *pout << "\\begin{scope}[style=Posterior]\n";
 
 std::vector<double> vParam(reg.GetPF().GetParameters());
 CPParametric pparam(reg.GetPF(), &vParam[0]);

 for (int i = Samples; --i >= 0;)
 {
  reg.GaussianSample(rnd, vParam);
  PlotProblem(pparam);
 }

 *pout << "\\pgfusepathqstroke\n";
 *pout << "\\end{scope}\n";
}

/////////////////////////////////////////////////////////////////////////////
// True probability
/////////////////////////////////////////////////////////////////////////////
void CTikZOneD::PlotTrue(const CArtificialProblem &problem)
{
 fTrue = true;

 *pout << "\\begin{scope}[style=True]\n";
 PlotProblem(problem);
 *pout << "\\pgfusepathqstroke\n";
 *pout << "\\end{scope}\n";
}

/////////////////////////////////////////////////////////////////////////////
// MAP
/////////////////////////////////////////////////////////////////////////////
void CTikZOneD::MAP(CRegression &reg)
{
 fMAP = true;

 *pout << "\\begin{scope}[style=MAP]\n";

 CPParametric pparam(reg.GetPF(), &reg.MAP()[0]);
 PlotProblem(pparam);

 *pout << "\\pgfusepathqstroke\n";
 *pout << "\\end{scope}\n";
}

/////////////////////////////////////////////////////////////////////////////
// Expected
/////////////////////////////////////////////////////////////////////////////
void CTikZOneD::Expected(CRegression &reg, int Samples, int Seed)
{
 fExpected = true;

 *pout << "\\begin{scope}[style=Expected]\n";

 CDFExpectedMC dfexp(reg, Samples, Seed);
 CAPDF apdf(dfexp);
 PlotProblem(apdf);

 *pout << "\\pgfusepathqstroke\n";
 *pout << "\\end{scope}\n";
}

/////////////////////////////////////////////////////////////////////////////
// Confidence bound
/////////////////////////////////////////////////////////////////////////////
void CTikZOneD::Confidence(CRegression &reg, double Alpha)
{
 fConfidence = true;

 *pout << "\\begin{scope}[style=Confidence]\n";

 CDFConfidenceBound dfcb(reg, Alpha);
 CAPDF apdf(dfcb);
 PlotProblem(apdf);

 *pout << "\\pgfusepathqstroke\n";
 *pout << "\\end{scope}\n";
}

/////////////////////////////////////////////////////////////////////////////
// Frame
/////////////////////////////////////////////////////////////////////////////
void CTikZOneD::Frame(int Samples) const
{
 *pout << "\\node[rotate=90,below,white] at (" << Scale * Tickp << "," << Scale << ") {9999};\n";
 *pout << "\\draw (-" << Scale << ",0) rectangle (";
 *pout << Scale << "," << Scale << ");\n"
          "\\scriptsize\n"
          "\\draw (-" << Scale << ",-" << Scale * Tick << ") -- (-" << Scale << ",0);\n"
          "\\draw (+" << Scale << ",-" << Scale * Tick << ") -- (+" << Scale << ",0);\n"
          "\\path (-" << Scale << ",-" << Scale * Tick << ") -- (+" << Scale << ",-" << Scale * Tick << ")\n"
          "      node[below,midway] {Parameter Value}\n"
          "      node[below,pos=0] {-1}\n"
          "      node[below,pos=1] {1};\n"
          "\n"
          "\\draw (-" << Scale * Tickp << ",0) -- (-" << Scale << ",0);\n"
          "\\draw (-" << Scale * Tickp << "," << Scale << ") -- (-" << Scale << "," << Scale << ");\n"
          "\\path (-" << Scale * Tickp << ",0) -- (-" << Scale * Tickp << "," << Scale << ")\n"
          "      node[left,midway] {$P$}\n"
          "      node[left,pos=0] {0}\n"
          "      node[left,pos=1] {1};\n";

 if (Samples > 0)
 {
  *pout << "\n"
           "\\draw (" << Scale * Tickp << ",0) -- (" << Scale << ",0);\n"
           "\\draw (" << Scale * Tickp << "," << Scale << ") -- (" << Scale << "," << Scale << ");\n"
           "\\path (" << Scale * Tickp << ",0) -- (" << Scale * Tickp << "," << Scale << ")\n"
           "      node[right,midway] {$n$}\n"
           "      node[right,pos=0] {1}\n"
           "      node[right,pos=1] {" << Samples << "};\n";
 }
}

/////////////////////////////////////////////////////////////////////////////
// Key
/////////////////////////////////////////////////////////////////////////////
void CTikZOneD::Key() const
{
 *pout << "\\node[draw,fill=white,rounded corners,anchor=north east] at (" << Scale * Tickm << "," << Scale * Tickm << ")\n";
 *pout << "{\\tiny\\begin{tabular}{c@{\\hspace{0.5em}}l}\n";
 if (fResults)
  *pout << " \\tikz \\draw[fill=white] (0,0) circle (" << Scale * Radius << "); /\n" <<
           " \\tikz \\draw[fill=black] (0,0) circle (" << Scale * Radius << "); & Win/Loss \\\\\n";

 if (fConfidence)
  CTikZ::Key("Confidence");
 if (fPosterior)
  CTikZ::Key("Posterior");
 if (fMAP)
  CTikZ::Key("MAP");
 if (fExpected)
  CTikZ::Key("Expected");
 if (fTrue)
  CTikZ::Key("True");
 if (fLCB)
 {
  CTikZ::Key("Mean");
  CTikZ::Key("LCB");
 }

 *pout << " \\end{tabular}};\n";
}

/////////////////////////////////////////////////////////////////////////////
// LCB
/////////////////////////////////////////////////////////////////////////////
void CTikZOneD::LCB(CRegression &reg)
{
 fLCB = true;

 double Mean;
 double Deviation;
 reg.GetMeanAndDeviation(Mean, Deviation);

 double PMean = CLogistic::f(Mean);
 double PLCB = CLogistic::f(Mean - reg.GetLocalizationHeight() * Deviation);

 *pout << "\\begin{scope}[style=Mean]\n";
 *pout << "\\draw (-" << Scale << "," << PMean*Scale << ") -- (" << Scale << "," << PMean*Scale << ");\n";
 *pout << "\\end{scope}\n";

 *pout << "\\begin{scope}[style=LCB]\n";
 *pout << "\\draw (-" << Scale << "," << PLCB*Scale << ") -- (" << Scale << "," << PLCB*Scale << ");\n";
 *pout << "\\end{scope}\n";
}
