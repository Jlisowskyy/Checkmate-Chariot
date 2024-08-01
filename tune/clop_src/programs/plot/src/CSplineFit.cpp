/////////////////////////////////////////////////////////////////////////////
//
// CSplineFit.cpp
//
// Rémi Coulom
// December, 2001
//
/////////////////////////////////////////////////////////////////////////////
#include "CSplineFit.h"
#include "CPGF.h"
#include "random.h"
#include "CBezier.h"
#include "CDiscretizedLine.h"

#include <iostream>
#include <math.h>
#include <iomanip>

/////////////////////////////////////////////////////////////////////////////
// Measure error on interval number i (i in 1...Points-1)
/////////////////////////////////////////////////////////////////////////////
double CSplineFit::GetError(int i, const CBezier &bez) const
{
 jMax = vIndex[i - 1];
 tMax = 0;

 double t = 0;
 double Error = 0;

 //
 // Loop over all points of this interval
 //
 for (int j = vIndex[i - 1]; j < vIndex[i]; j++)
 {
  double x, y;
  bez.GetValue(t, x, y);
  double ex = x - dl.X(j);
  double ey = y - dl.Y(j);
  double e2 = ex * ex + ey * ey;

  if (e2 > Error)
  {
   Error = e2;
   jMax = j;
   tMax = t;
  }

  double dx, dy;
  bez.GetDerivative(t, dx, dy);
  double n2 = dx * dx + dy * dy;
  t += ((dl.X(j + 1) - x) * dx + (dl.Y(j + 1) - y) * dy) / n2;
 }

 return std::sqrt(Error);
}

/////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////
CSplineFit::CSplineFit(const CDiscretizedLine &dlInit):
 dl(dlInit),
 Points(0),
 vIndex(dl.Size())
{
}

/////////////////////////////////////////////////////////////////////////////
// Fit with as few points as possible
/////////////////////////////////////////////////////////////////////////////
void CSplineFit::Fit(double DMax)
{
 Points = 1;
 vIndex[0] = 0;
 while (vIndex[Points - 1] < dl.Size() - 1)
 {
  int Min = vIndex[Points - 1] + 1;
  int Max = dl.Size() - 1;

  while (Min < Max)
  {
   int i = (Min + Max + 1) / 2;
   vIndex[Points] = i;
   CBezier bez;
   double D = OptimizeUV(Points, bez, DMax);
   if (D > DMax)
    Max = i - 1;
   else
    Min = i;
  }

  vIndex[Points] = Min;

  Points++;
 }
}

/////////////////////////////////////////////////////////////////////////////
// Compute Bezier approximation (i > 0)
/////////////////////////////////////////////////////////////////////////////
void CSplineFit::GetBezier(int i, CBezier &bez, double u, double v) const
{
 //
 // Find directions of tangents at each point
 //
 for (int j = 2; --j >= 0;)
 {
  int i0 = vIndex[i - 1 + j];
  if (i0 == 0 || i0 == dl.Size() - 1)
   dl.CircularTangent(i0, tdx[j], tdy[j]);
  else
   dl.SimpleTangent(i0, tdx[j], tdy[j]);
 }

 //
 // Scale according to the distance between two points
 //
 double dx = dl.X(vIndex[i]) - dl.X(vIndex[i - 1]);
 double dy = dl.Y(vIndex[i]) - dl.Y(vIndex[i - 1]);
 double n2 = dx * dx + dy * dy;
 double n = std::sqrt(n2);
 tdx[0] *= n;
 tdx[1] *= n;
 tdy[0] *= n;
 tdy[1] *= n;

 //
 // CBezier info
 //
 bez.tx[0] = dl.X(vIndex[i - 1]);
 bez.tx[1] = dl.X(vIndex[i - 1]) + u * tdx[0];
 bez.tx[2] = dl.X(vIndex[i])     - v * tdx[1];
 bez.tx[3] = dl.X(vIndex[i]);
 bez.ty[0] = dl.Y(vIndex[i - 1]);
 bez.ty[1] = dl.Y(vIndex[i - 1]) + u * tdy[0];
 bez.ty[2] = dl.Y(vIndex[i])     - v * tdy[1];
 bez.ty[3] = dl.Y(vIndex[i]);
}

/////////////////////////////////////////////////////////////////////////////
// u and v must be between 0 and 1
/////////////////////////////////////////////////////////////////////////////
double CSplineFit::Normalize(double x)
{
 if (x < 0.0001)
  return 0.0001;
 else if (x > 1)
  return 1;
 else
  return x;
}

/////////////////////////////////////////////////////////////////////////////
// Optimize u and v
/////////////////////////////////////////////////////////////////////////////
double CSplineFit::OptimizeUV(int i, CBezier &bez, double DMax) const
{
 //
 // Baseline: u, v = 1/3
 //
 double u = 1.0 / 3.0;
 double v = 1.0 / 3.0;
 GetBezier(i, bez, u, v);
 double Error = GetError(i, bez);

 //
 // Try to optimize it
 //
 const double Scale0 = 0.8;
 double Scale = Scale0;
 for (int k = 10; --k >= 0;)
 {
  double t = tMax;
  int j = jMax;

  double x, y;
  bez.GetValue(t, x, y);

  double dx, dy;
  dx = dl.X(j) - x;
  dy = dl.Y(j) - y;

  double r = 1 - t;
  double c1 = 3.0 * t * r * r;
  double c2 = -3.0 * t * t * r;

  double m11 = c1 * tdx[0];
  double m21 = c1 * tdy[0];
  double m12 = c2 * tdx[1];
  double m22 = c2 * tdy[1];

  double det = m11 * m22 - m12 * m21;

  if (det * det > 1e-6)
  {
   double du = ( m22 * dx - m12 * dy) / det;
   double dv = (-m21 * dx + m11 * dy) / det;

   double uTest = Normalize(u + Scale * du);
   double vTest = Normalize(v + Scale * dv);
   CBezier bezTest;
   GetBezier(i, bezTest, uTest, vTest);
   double ErrorTest = GetError(i, bezTest);

   if (ErrorTest < Error)
   {
    u = uTest;
    v = vTest;
    bez = bezTest;
    Error = ErrorTest;
    Scale = Scale0;
   }
   else
   {
    tMax = t;
    jMax = j; 
    Scale *= 0.7;
   }
  }
  else
   break;
 }

 return Error;
}

/////////////////////////////////////////////////////////////////////////////
// Write in tikz/pgf format
/////////////////////////////////////////////////////////////////////////////
void CSplineFit::TikZ(int Precision, bool fMoveTo, std::ostream &out) const
{
 if (Points >= 2)
 {
  for (int i = 0; i < Points; i++)
  {
   if (i > 0)
   {
    CBezier bez;
    OptimizeUV(i, bez, 0.0);
    out << "\\pgfpathqcurveto";
    CPGF::Point(out, bez.tx[1], bez.ty[1], Precision);
    CPGF::Point(out, bez.tx[2], bez.ty[2], Precision);
    CPGF::Point(out, dl.X(vIndex[i]), dl.Y(vIndex[i]), Precision);
   }
   else if (fMoveTo)
   {
    out << "\\pgfpathqmoveto";
    CPGF::Point(out, dl.X(vIndex[i]), dl.Y(vIndex[i]), Precision);
   }

   out << '\n';
  }

#if 0
  for (int i = 0; i < Points; i++)
   CPGF::Circle(out, 4, dl.X(vIndex[i]), dl.Y(vIndex[i]), 0.03);
//  for (int i = 0; i < dl.Size(); i++)
//   CPGF::Circle(out, 4, dl.X(i), dl.Y(i), 0.02);
  CPGF::Circle(out, 4, dl.X(0), dl.Y(0), 0.04);
#endif
 }
}

/////////////////////////////////////////////////////////////////////////////
// PGF shortcut
/////////////////////////////////////////////////////////////////////////////
void CSplineFit::PGF(const CDiscretizedLine &dl,
                     double DMax,
                     std::ostream &out)
{
 //
 // Split at angles
 //
 CSplineFit sfit(dl);

 int i = 0;
 while (1)
 {
  int j = i + 1;
  while (!dl.IsAngle(j) && j < dl.Size() - 1)
   j++;

  CDiscretizedLine dl2 = dl.Range(i, j + 1);
  CSplineFit sfit2(dl2);
  sfit2.Fit(DMax);
  sfit2.TikZ(CPGF::GetPrecision(DMax) + 1, i == 0, out);

  if (j == dl.Size() - 1)
   break;
  else
   i = j;
 }

 //
 // Close
 //
 if (dl.IsClosed())
  out << "\\pgfpathclose\n";
}
