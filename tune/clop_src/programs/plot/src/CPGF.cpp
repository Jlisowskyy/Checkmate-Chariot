/////////////////////////////////////////////////////////////////////////////
//
// CPGF.cpp
//
// Rémi Coulom
//
// February, 2009
//
/////////////////////////////////////////////////////////////////////////////
#include "CPGF.h"
#include "pi.h"
#include <iostream>
#include <cmath>

const double CPGF::cmtopt = 28.4527559;

/*
 for a compact pdf
 /usr/share/texmf/tex/generic/pgf/systemlayer/pgfsys.code.tex

 < \def\pgf@sys@bp@correct#1{#1=0.99627#1}
 ---
 > \def\pgf@sys@bp@correct#1{#1=#1}
 26c26
 <     \pgf@x=0.99627\pgf@x%
 ---
 >     \pgf@x=\pgf@x%
*/

/////////////////////////////////////////////////////////////////////////////
// Precision
/////////////////////////////////////////////////////////////////////////////
int CPGF::GetPrecision(double DMax)
{
 int Precision = 0;
 double Error = 0.5 / CPGF::cmtopt;

 while (Error > DMax)
 {
  Precision++;
  Error *= 0.1;
 }

 return Precision;
}

/////////////////////////////////////////////////////////////////////////////
// One point (in centimeters)
/////////////////////////////////////////////////////////////////////////////
void CPGF::Point(std::ostream &out, double x, double y, int Precision)
{
 std::ios_base::fmtflags F = out.setf(std::ios::fixed, std::ios::floatfield);
 std::streamsize OldPrecision = out.precision(Precision);

 out << "{" << cmtopt * x << "pt}{" << cmtopt * y << "pt}";

 out.precision(OldPrecision);
 out.setf(F);
}

/////////////////////////////////////////////////////////////////////////////
// Circle with n cubic splines
/////////////////////////////////////////////////////////////////////////////
void CPGF::Circle(std::ostream &out,
                  int n,
                  double x,
                  double y,
                  double r)
{
 //
 // Precision
 //
 const int Precision = GetPrecision(r / std::pow(10.0, n));

 //
 // A few useful constants
 //
 const double Theta = 2 * MY_PI / n;
 const double c = std::cos(Theta);
 const double s = std::sin(Theta);
 const double a = 4.0 * (2 * std::sin(Theta / 2) - s) / (3 * (1.0 - c));

 //
 // Starting point
 //
 double dx = r;
 double dy = 0;
 out << "\\pgfpathqmoveto";
 Point(out, x + dx, y + dy, Precision);
 out << '\n';

 //
 // Loop over n Bezier curves
 //
 for (int i = n; --i >= 0;)
 {
  out << "\\pgfpathqcurveto";
  Point(out, x + dx + a * dy, y + dy - a * dx, Precision);

  //
  // Rotate by theta to find next point
  //
  {
   double dxx = c * dx + s * dy;
   double dyy = c * dy - s * dx;
   dx = dxx;
   dy = dyy;
  }

  Point(out, x + dx - a * dy, y + dy + a * dx, Precision);
  Point(out, x + dx, y + dy, Precision);
  out << '\n';
 }
}

/////////////////////////////////////////////////////////////////////////////
// Cross (x)
/////////////////////////////////////////////////////////////////////////////
void CPGF::Cross(std::ostream &out, double x, double y, double r)
{
 const int Precision = GetPrecision(r / 100);

 out << "\\pgfpathqmoveto";
 Point(out, x - r, y - r, Precision);
 out << "\\pgfpathqlineto";
 Point(out, x + r, y + r, Precision);

 out << "\\pgfpathqmoveto";
 Point(out, x - r, y + r, Precision);
 out << "\\pgfpathqlineto";
 Point(out, x + r, y - r, Precision);
}
