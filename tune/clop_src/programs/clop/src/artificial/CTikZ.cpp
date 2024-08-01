/////////////////////////////////////////////////////////////////////////////
//
// CTikZ.cp
//
// Rémi Coulom
//
// April, 2009
//
/////////////////////////////////////////////////////////////////////////////
#include "CTikZ.h"
#include "CResults.h"
#include "CRegression.h"

#include <iostream>

/////////////////////////////////////////////////////////////////////////////
// Add style
/////////////////////////////////////////////////////////////////////////////
void CTikZ::AddStyle(const std::string &s,
                     float R,
                     float G,
                     float B,
                     const std::string &sStyle)
{
 mapStyle.insert(std::pair<std::string, CStyle>(s, CStyle(R, G, B, sStyle)));
}

/////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////
CTikZ::CTikZ():
 pout(&std::cout),
 Scale(1.0),
 Radius(0.007),
 Tick(0.02),
 CircleN(2),
 fGrayscale(false),
 SplineSamples(1000),
 SplineD(0.001),
 fResults(false)
{
 AddStyle("Confidence", 0.1f, 0.1f, 0.8f, "");
 AddStyle("Posterior", 0.5f, 1.0f, 0.5f, "");
 AddStyle("MAP", 0.6f, 0.1f, 0.0f, "thick");
 AddStyle("Expected", 0.9f, 0.0f, 0.1f, "thick,dash pattern = on 12pt off 2pt");
 AddStyle("True", 1.0f, 0.0f, 0.0f, "thick,densely dotted");
 AddStyle("Contour", 0.8f, 0.8f, 1.0f, "");
 AddStyle("Mean", 0.2f, 0.2f, 1.0f, "thick");
 AddStyle("LCB", 0.2f, 0.2f, 1.0f, "thick,dashed");
}

/////////////////////////////////////////////////////////////////////////////
// Set style
/////////////////////////////////////////////////////////////////////////////
void CTikZ::SetStyle(const std::string &s,
                     float R,
                     float G,
                     float B,
                     const std::string &sStyle)
{
 std::map<std::string, CStyle>::iterator i = mapStyle.find(s);
 if (i != mapStyle.end())
  i->second = CStyle(R, G, B, sStyle);
}

/////////////////////////////////////////////////////////////////////////////
// Prolog
/////////////////////////////////////////////////////////////////////////////
void CTikZ::Prolog() const
{
 *pout << "\\begin{tikzpicture}\n";

 //
 // Loop over all styles
 //
 for (std::map<std::string, CStyle>::const_iterator i = mapStyle.begin();
      i != mapStyle.end();
      i++)
 {
  //
  // Color
  //
  CColor col = fGrayscale ? i->second.col.Grayscale() : i->second.col;
  *pout << "\\xdefinecolor{" << i->first << "_color}{rgb}{";
  *pout << col.GetR() << ',';
  *pout << col.GetG() << ',';
  *pout << col.GetB() << "}\n";

  //
  // Style
  //
  *pout << "\\tikzstyle{" << i->first << "}=[" << i->first << "_color";
  if (i->second.sStyle != "")
   *pout << ',' << i->second.sStyle;
  *pout << "]\n";
 }
}

/////////////////////////////////////////////////////////////////////////////
// Epilog
/////////////////////////////////////////////////////////////////////////////
void CTikZ::Epilog() const
{
 *pout << "\\end{tikzpicture}\n";
}

/////////////////////////////////////////////////////////////////////////////
// Key
/////////////////////////////////////////////////////////////////////////////
void CTikZ::Key(const std::string &s) const
{
 *pout << " \\tikz[baseline=-0.5ex] \\draw[style=" << s;
 *pout << "] (-9pt,0) -- (9pt,0); & " << s << "\\\\\n";
}

/////////////////////////////////////////////////////////////////////////////
// Results
/////////////////////////////////////////////////////////////////////////////
void CTikZ::Results(const CResults &results, const CRegression *preg)
{
 fResults = true;

 COutcome previous = COutcome::Unknown;

 for (unsigned i = 0; i < results.GetSamples(); i++)
 {
  PlotResult(results, i);

  if (preg)
  {
   double x = 1.0 - preg->GetSampleWeight(i);
   *pout << "\\definecolor{mycolor}{rgb}{" << x << ',' << x << ',' << x << "}\n";
   *pout << "\\pgfsetstrokecolor{mycolor}\n";
   if (results.GetOutcome(i) == COutcome::Win)
    *pout << "\\pgfsetfillcolor{white}\n";
   else
    *pout << "\\pgfsetfillcolor{mycolor}\n";
   *pout << "\\pgfusepathqfillstroke\n";
  }
  else
  {
   if (results.GetOutcome(i) != previous)
   {
    previous = results.GetOutcome(i);
    if (results.GetOutcome(i) == COutcome::Win)
     *pout << "\\pgfsetfillcolor{white}\n";
    else
     *pout << "\\pgfsetfillcolor{black}\n";
   }

   if (results.GetOutcome(i) == COutcome::Win ||
       i == results.GetSamples() - 1 ||
       results.GetOutcome(i + 1) != results.GetOutcome(i))
    *pout << "\\pgfusepathqfillstroke\n";
  }
 }
}
