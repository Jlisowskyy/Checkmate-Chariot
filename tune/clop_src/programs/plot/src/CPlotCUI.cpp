////////////////////////////////////////////////////////////////////////////
//
// CPlotCUI.cpp
//
// Remi Coulom
//
// May, 2000
//
////////////////////////////////////////////////////////////////////////////
#include "CPlotCUI.h"
#include "CPlot.h"
#include "CSplineFit.h"
#include "CSurface.h"
#include "CContour.h"
#include "CDiscretizedLine.h"

#include <sstream>
#include <iomanip>
#include <cmath>
#include <string>
#include <cstring>

////////////////////////////////////////////////////////////////////////////
// Command strings
////////////////////////////////////////////////////////////////////////////
const char * const CPlotCUI::tszCommands[] =
{
 "?",
 "obs",
 "target",
 "ranges",
 "dist",
 "grid",
 "line",
 "dmax",
 "visible",
 "exclude",
 "smooth",
 "dots",
 "steps",
 "plotx",
 "ploty",
 "gnuplot",
 "bbox",
 "axis",
 "lscale",
 "label",
 "project",
 "write",
 "contour",
 "contours",
 0
};

//
// Command identifiers
//
enum
{
 IDC_Help,
 IDC_Obs,
 IDC_Target,
 IDC_Ranges,
 IDC_Dist,
 IDC_Grid,
 IDC_Line,
 IDC_DMax,
 IDC_Visible,
 IDC_Exclude,
 IDC_Smooth,
 IDC_Dots,
 IDC_Steps,
 IDC_PlotX,
 IDC_PlotY,
 IDC_Gnuplot,
 IDC_BBox,
 IDC_Axis,
 IDC_LScale,
 IDC_Label,
 IDC_Project,
 IDC_Write,
 IDC_Contour,
 IDC_Contours
};

////////////////////////////////////////////////////////////////////////////
// Constructor
////////////////////////////////////////////////////////////////////////////
CPlotCUI::CPlotCUI(CPlot &plotInit, CConsoleUI *pcui, int openmode) :
 CConsoleUI(pcui, openmode),
 plot(plotInit),
 GridSamples(50),
 LineSamples(500),
 Exclude(0),
 fSmooth(1),
 fDots(0),
 Steps(1),
 DMax(0.01),
 LabelScale(1.0)
{
}

/////////////////////////////////////////////////////////////////////////////
// Local prompt
/////////////////////////////////////////////////////////////////////////////
void CPlotCUI::PrintLocalPrompt(std::ostream &out)
{
 out << "CPlot";
}

/////////////////////////////////////////////////////////////////////////////
// Plot a line (with hidden parts) (should use better method)
/////////////////////////////////////////////////////////////////////////////
void CPlotCUI::PlotLine(double x0, double y0, double z0,
                        double x1, double y1, double z1,
                        std::ostream &out) const
{
 double t = 0;
 const double Step = 0.0009999999;
 int fVisible = 0;
 double t0 = 0;

 while (t < 1.0)
 {
  double x = x0 + t * (x1 - x0);
  double y = y0 + t * (y1 - y0);
  double z = z0 + t * (z1 - z0);
  int f = plot.Visible3(x, y, z);

  if (!fVisible && f)
   t0 = t;
  else if (fVisible && (!f || t >= 0.9999))
  {
   double x20;
   double y20;
   plot.Transform(x0 + t0 * (x1 - x0),
                  y0 + t0 * (y1 - y0),
                  z0 + t0 * (z1 - z0),
                  x20, y20);

   double x21;
   double y21;
   plot.Transform(x, y, z, x21, y21);

   out << "\\draw";
   out << '(' << x20 << ',' << y20 << ')';
   out << " -- ";
   out << '(' << x21 << ',' << y21 << ')';
   out << ';';
   out << '\n';
  }

  fVisible = f;
  t += Step;
 }
}

////////////////////////////////////////////////////////////////////////////
// Find change in visibility with accuracy
////////////////////////////////////////////////////////////////////////////
double CPlotCUI::VisibilityChange(double x0, double y0, int f0,
                                  double x1, double y1, int f1) const
{
 double tMin = 0.0;
 double tMax = 1.0;

 for (int i = 10; --i >= 0;)
 {
  double t = (tMin + tMax) / 2;
  int f = plot.Visible(x0 + t * (x1 - x0), y0 + t * (y1 - y0));
  if (f == f0)
   tMin = t;
  else
   tMax = t;
 }

 if (tMin == 0.0)
  return tMin;
 else if (tMax == 1.0)
  return tMax;
 else
  return (tMin + tMax) / 2;
}

////////////////////////////////////////////////////////////////////////////
// Plot a part of line
////////////////////////////////////////////////////////////////////////////
void CPlotCUI::psline(int n,
                      const double *px,
                      const double *py,
                      const int *pfDot,
                      std::ostream &out) const
{
 out.setf(std::ios::fixed, std::ios::floatfield);

 if (fDots)
 {
  if (!pfDot[0])
  {
   out << "\\psline";
   if (n == 2 && pfDot[1])
    out << "{-*}";
   out << '(' << px[0] << ',' << py[0] << ')';
   out << '(' << px[1] << ',' << py[1] << ')';
   out << '\n';
   if (--n < 2)
    return;
   px++;
   py++;
  }
  if (!pfDot[1])
  {
   out << "\\psline";
   if (n == 2)
    out << "{-*}";
   out << '(' << px[n - 1] << ',' << py[n - 1] << ')';
   out << '(' << px[n - 2] << ',' << py[n - 2] << ')';
   out << '\n';
   if (--n < 2)
    return;
  }
 }

 out << "\\psline";
 if (fDots)
  out << "[showpoints=true]";
 for (int i = 0; i < n; i++)
  out << '(' << px[i] << ',' << py[i] << ')';
 out << '\n';
}

////////////////////////////////////////////////////////////////////////////
// Plot a curve
////////////////////////////////////////////////////////////////////////////
void CPlotCUI::curve(int n,
                     const double *px,
                     const double *py,
                     double DMax,
                     std::ostream &out) const
{
 CDiscretizedLine dl;
 for (int i = 0; i < n; i++)
  dl.PushBack(px[i], py[i]);
 CSplineFit::PGF(dl, DMax, out);
}

////////////////////////////////////////////////////////////////////////////
// Plot an isoline
////////////////////////////////////////////////////////////////////////////
void CPlotCUI::PlotIsoLine(double x0,
                           double y0,
                           double x1,
                           double y1,
                           std::ostream &out) const
{
 double *px = new double[LineSamples + 1];
 double *py = new double[LineSamples + 1];

 int n = 0;
 int tfDot[2] = {1, 1};
 double xPrev = x0;
 double yPrev = y0;
 int fVisiblePrev = 1;

 for (int j = 0; j <= LineSamples * Steps; j++)
 {
  double x = x0 + (x1 - x0) * (double(j) / double (LineSamples * Steps));
  double y = y0 + (y1 - y0) * (double(j) / double (LineSamples * Steps));
  int fVisible = plot.Visible(x, y);

  if (fVisible != fVisiblePrev && j > 0)
  {
   double t = VisibilityChange(xPrev, yPrev, fVisiblePrev, x, y, fVisible);
   double xChange = xPrev + t * (x - xPrev);
   double yChange = yPrev + t * (y - yPrev);
   if (fSmooth)
   {
    x = xChange;
    y = yChange;
   }
   else // if ((fVisible && t < 1.0) || (!fVisible && t > 0.0))
   {
    plot.Projection(xChange, yChange, px[n], py[n]);
    n++;
    tfDot[fVisible ^ 1] = 0;
   }
  }

  if (!fVisible)
  {
   if (n > 1)
   {
    if (fSmooth)
     curve(n, px, py, DMax, out); 
    else
     psline(n, px, py, tfDot, out); 
   }
   n = 0;
   tfDot[0] = 1;
   tfDot[1] = 1;
  }
  else if (!(j % Steps))
  {
   plot.Projection(x, y, px[n], py[n]);
   n++;
  }

  xPrev = x;
  yPrev = y;
  fVisiblePrev = fVisible;
 }

 if (n > 1)
 {
  if (fSmooth)
   curve(n, px, py, DMax, out); 
  else
   psline(n, px, py, tfDot, out); 
 }

 delete[] px;
 delete[] py;
}

////////////////////////////////////////////////////////////////////////////
// Parse a corner
////////////////////////////////////////////////////////////////////////////
void CPlotCUI::ParseCorner(const char *psz,
                           double &x,
                           double &y,
                           double &z) const
{
 if (psz[0] == '0')
  x = plot.GetXMin();
 else
  x = plot.GetXMax();

 if (psz[1] == '0')
  y = plot.GetYMin();
 else
  y = plot.GetYMax();

 if (psz[2] == '0')
  z = plot.GetZMin();
 else
  z = plot.GetZMax();
}

////////////////////////////////////////////////////////////////////////////
// Contour function
////////////////////////////////////////////////////////////////////////////
void CPlotCUI::Contour(std::ostream &out, double Level, int W, int H) const
{
 CContour contour(plot.GetSurface());
 contour.SetBounds(plot.GetXMin(),
                   plot.GetXMax(),
                   plot.GetYMin(),
                   plot.GetYMax());
 contour.SetGridSize(W, H);
 contour.SetLevel(Level);
 contour.PlotLines(out, DMax, plot.GetDist());
}

////////////////////////////////////////////////////////////////////////////
// Function to process commands
////////////////////////////////////////////////////////////////////////////
int CPlotCUI::ProcessCommand(const char *pszCommand,
                             const char *pszParameters,
                             std::istream &in,
                             std::ostream &out)
{
 out << std::setprecision(7);
 out << std::setiosflags(std::ios::fixed);

 switch (ArrayLookup(pszCommand, tszCommands))
 {
  case IDC_Help: ///////////////////////////////////////////////////////////
   CConsoleUI::ProcessCommand(pszCommand, pszParameters, in, out);
   out << "CPlot commands\n";
   out << "~~~~~~~~~~~~~~\n";
   out << "obs [x y z] ..... get [set] observer position\n";
   out << "target [x y z] .. get [set] target position\n";
   out << "ranges [x X y Y]  get [set] ranges\n"; 
   out << "dist [d] ........ get [set] distance to projection plane\n";
   out << "grid [n] ........ get [set] grid samples\n";
   out << "line [n] ........ get [set] line samples\n";
   out << "dmax [x] ........ get [set] DMax\n";
   out << '\n';
   out << "visible x y z ... test whether point at (x, y, z) is visible\n";
   out << "exclude [e] ..... exclude lines where i % e == 0 (if e > 0)\n";
   out << "smooth [f] ...... get [set] smoothness flag\n";
   out << "dots [f] ........ get [set] dot flag\n";
   out << "steps [n] ....... get [set] steps (non-smooth visibility)\n";
   out << "plotx ........... plot lines along the x axis\n";
   out << "ploty ........... plot lines along the y axis\n";
   out << "gnuplot ......... write data that can be plotted by gnuplot\n";
   out << "bbox [s] ........ get s * the 2D bounding box for this 3D box\n";
   out << "axis c0 c1 ...... draw axis from corner c0 to corner c1\n";
   out << "                   corner syntax: 000, 010, 011, etc.\n";
   out << "lscale [s] ...... get [set] label scale\n";
   out << "label c0 c1 t d l plot label\n";
   out << "                   c0, c1: corners (like for axis)\n";
   out << "                   t: value along this axis\n";
   out << "                   d: distance from the axis\n";
   out << "                   l: label (must be enclosed in {braces})\n";
   out << "project x y z ... get 2D projection of one 3D point\n";
   out << "write params .... echo without newline\n"; // ??? dans CConsoleUI
   out << '\n';
   out << "contour z w h ... plot contour level z with wxh resolution\n";
   out << "contours m M s n  plot contours from m to M by s with n resolution\n";
   out << '\n';
  break;

  case IDC_Obs: ////////////////////////////////////////////////////////////
  {
   const double *pObs = plot.GetObserver();
   double x = pObs[0];
   double y = pObs[1];
   double z = pObs[2];
   std::istringstream(pszParameters) >> x >> y >> z;
   plot.SetObserver(x, y, z);
   out << pObs[0] << ' ' << pObs[1] << ' ' << pObs[2] << '\n';
  }
  break;

  case IDC_Target: /////////////////////////////////////////////////////////
  {
   const double *pTarget = plot.GetTarget();
   double x = pTarget[0];
   double y = pTarget[1];
   double z = pTarget[2];
   std::istringstream(pszParameters) >> x >> y >> z;
   plot.SetTarget(x, y, z);
   out << pTarget[0] << ' ' << pTarget[1] << ' ' << pTarget[2] << '\n';
  }
  break;

  case IDC_Ranges: /////////////////////////////////////////////////////////
  {
   double xMin = plot.GetXMin();
   double xMax = plot.GetXMax();
   double yMin = plot.GetYMin();
   double yMax = plot.GetYMax();
   double zMin = plot.GetZMin();
   double zMax = plot.GetZMax();

   std::istringstream(pszParameters) >>
    xMin >> xMax >> yMin >> yMax >> zMin >> zMax;
   plot.SetRanges(xMin, xMax, yMin, yMax, zMin, zMax);
   
   out << plot.GetXMin() << ' ';
   out << plot.GetXMax() << ' ';
   out << plot.GetYMin() << ' ';
   out << plot.GetYMax() << ' ';
   out << plot.GetZMin() << ' ';
   out << plot.GetZMax() << '\n';
  }
  break;

  case IDC_Dist: ///////////////////////////////////////////////////////////
  {
   double Dist = plot.GetDist();
   std::istringstream(pszParameters) >> Dist;
   plot.SetDist(Dist);
   out << plot.GetDist() << '\n';
  }
  break;

  case IDC_Grid: ///////////////////////////////////////////////////////////
   std::istringstream(pszParameters) >> GridSamples;
   out << GridSamples << '\n';
  break;

  case IDC_Line: ///////////////////////////////////////////////////////////
   std::istringstream(pszParameters) >> LineSamples;
   out << LineSamples << '\n';
  break;

  case IDC_DMax: ///////////////////////////////////////////////////////////
   std::istringstream(pszParameters) >> DMax;
   out << DMax << '\n';
  break;

  case IDC_Visible: ////////////////////////////////////////////////////////
  {
   double x = 0.0;
   double y = 0.0;
   double z = 0.0;
   std::istringstream(pszParameters) >> x >> y >> z;
   if (plot.Visible3(x, y, z))
    out << "yes\n";
   else
    out << "no\n";
  }
  break;

  case IDC_Exclude: ////////////////////////////////////////////////////////
   std::istringstream(pszParameters) >> Exclude;
   out << Exclude << '\n';
  break;

  case IDC_Smooth: /////////////////////////////////////////////////////////
   std::istringstream(pszParameters) >> fSmooth;
   out << fSmooth << '\n';
  break;

  case IDC_Dots: ///////////////////////////////////////////////////////////
   std::istringstream(pszParameters) >> fDots;
   out << fDots << '\n';
  break;

  case IDC_Steps: //////////////////////////////////////////////////////////
   std::istringstream(pszParameters) >> Steps;
   out << Steps << '\n';
  break;

  case IDC_PlotX: //////////////////////////////////////////////////////////
  {
   for (int i = 0; i <= GridSamples; i++)
    if (Exclude <= 0 || i % Exclude)
    {
     double x = plot.GetXMin() + (plot.GetXMax() - plot.GetXMin()) *
                (double(i) / double (GridSamples));
     PlotIsoLine(x, plot.GetYMin(), x, plot.GetYMax(), out);
    }
  }
  break;

  case IDC_PlotY: //////////////////////////////////////////////////////////
   for (int i = 0; i <= GridSamples; i++)
    if (Exclude <= 0 || i % Exclude)
    {
     double y = plot.GetYMin() + (plot.GetYMax() - plot.GetYMin()) *
               (double(i) / double (GridSamples));
     PlotIsoLine(plot.GetXMin(), y, plot.GetXMax(), y, out);
    }
  break;

  case IDC_Gnuplot: ////////////////////////////////////////////////////////
  {
   for (int i = 0; i <= GridSamples; i++)
    if (Exclude <= 0 || i % Exclude)
    {
     double x = plot.GetXMin() + (plot.GetXMax() - plot.GetXMin()) *
                (double(i) / double (GridSamples));
     for (int j = 0; j <= GridSamples; j++)
      if (Exclude <= 0 || j % Exclude)
      {
       double y = plot.GetYMin() + (plot.GetYMax() - plot.GetYMin()) *
                 (double(j) / double (GridSamples));
       out << x << ' ' << y << ' ' << plot.GetSurface().GetValue(x, y) << '\n';
      }
     out << '\n';
    }
  }
  break;

  case IDC_BBox: ///////////////////////////////////////////////////////////
  {
   double s = 1.0;
   std::istringstream(pszParameters) >> s;

   double x0, y0, x1, y1;
   plot.BoundingBox(x0, y0, x1, y1);

   double dx = 0.5 * (s - 1.0) * (x1 - x0);
   double dy = 0.5 * (s - 1.0) * (y1 - y0);

   out << '(' << x0 - dx << ',' << y0 - dy << ')';
   out << '(' << x1 + dx << ',' << y1 + dy << ')';
  }
  break;

  case IDC_Axis: ///////////////////////////////////////////////////////////
  {
   if (std::strlen(pszParameters) >= 7)
   {
    double x0, y0, z0, x1, y1, z1;
    ParseCorner(pszParameters, x0, y0, z0);
    ParseCorner(pszParameters + 4, x1, y1, z1);
    PlotLine(x0, y0, z0, x1, y1, z1, out);
   }
   else
    out << "error: bad syntax in parameters\n";
  }
  break;

  case IDC_LScale: /////////////////////////////////////////////////////////
   std::istringstream(pszParameters) >> LabelScale;
   out << LabelScale << '\n';
  break;

  case IDC_Label: //////////////////////////////////////////////////////////
  {
   if (strlen(pszParameters) >= 7)
   {
    //
    // Parse parameters
    //
    double x, y, z, x0, y0, z0, x1, y1, z1;
    ParseCorner(pszParameters, x0, y0, z0);
    ParseCorner(pszParameters + 4, x1, y1, z1);
    double t = 0;
    double d = 0;
    std::istringstream(pszParameters + 7) >> t >> d;

    if (x0 == x1)
     x = x0;
    else
     x = t;

    if (y0 == y1)
     y = y0;
    else
     y = t;

    if (z0 == z1)
     z = z0;
    else
     z = plot.GetZMin() + 0.5 * (t + 1) * (plot.GetZMax() - plot.GetZMin());

    //
    // Move some distance away from the axis
    //
    double vx = plot.GetObserver()[0] - x0;
    double vy = plot.GetObserver()[1] - y0;
    double vz = plot.GetObserver()[2] - z0;
 
    double dx = (y1 - y0) * vz - (z1 - z0) * vy;
    double dy = (z1 - z0) * vx - (x1 - x0) * vz;
    double dz = (x1 - x0) * vy - (y1 - y0) * vx;
 
    double n2 = dx * dx + dy * dy + dz * dz;
    if (n2 > 0)
    {
     double mul = d / sqrt(n2);
     x += dx * mul;
     y += dy * mul;
     z += dz * mul;
    }
 
    //
    // Place the label only if it is visible
    //
    if (plot.Visible3(x, y, z))
    {
     double x2;
     double y2;
     plot.Transform(x, y, z, x2, y2);
     const char *pLabel = pszParameters;
     while (*pLabel && *pLabel != '{')
      pLabel++;

     out << "\\node at (";

     out << x2 << ',' << y2 << ')';

     out << "{\\scalebox{"; 

     out << LabelScale * plot.Scale(x, y, z) << '}';
     out << pLabel << '}';
     out << ';';
     out << '\n';
    }
   }
  }
  break;

  case IDC_Project: ////////////////////////////////////////////////////////
  {
   double x = 0;
   double y = 0;
   double z = 0;
   std::istringstream(pszParameters) >> x >> y >> z;

   double x2;
   double y2;
   plot.Transform(x, y, z, x2, y2);
   out << '(' << x2 << ',' << y2 << ')';
  }
  break;

  case IDC_Write: //////////////////////////////////////////////////////////
   out << pszParameters;
  break;

  case IDC_Contour: ////////////////////////////////////////////////////////
  {
   double Level = -1;
   int W = 20;
   int H = 20;
   std::istringstream(pszParameters) >> Level >> W >> H;

   Contour(out, Level, W, H);
  }
  break;

  case IDC_Contours: ///////////////////////////////////////////////////////
  {
   double Min = plot.GetZMin();
   double Max = plot.GetZMax();
   double Steps = 10;
   int N = 100;
   std::istringstream(pszParameters) >> Min >> Max >> Steps >> N;

   for (int i = 0; i <= Steps; i++)
   {
    double Level = Min + i * (Max - Min) / Steps;
    Contour(out, Level, N, N);
    double x = float(i) / Steps;
    int w = int(440 + 210 * x);
    out << "\\xdefinecolor{col}{wave}{" << w << "}\n";
    out << "\\pgfsetstrokecolor{col}\n";
    out << "\\pgfusepathqstroke\n";
   }
  }
  break;

  default: /////////////////////////////////////////////////////////////////
   return CConsoleUI::ProcessCommand(pszCommand, pszParameters, in, out);
 }

 return PC_Continue;
}
