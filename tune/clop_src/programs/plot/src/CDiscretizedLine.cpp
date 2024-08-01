/////////////////////////////////////////////////////////////////////////////
//
// CDiscretizedLine
//
// Rémi Coulom
//
// May, 2009
//
/////////////////////////////////////////////////////////////////////////////
#include "CDiscretizedLine.h"
#include <cmath>

/////////////////////////////////////////////////////////////////////////////
// Range constructor
/////////////////////////////////////////////////////////////////////////////
CDiscretizedLine CDiscretizedLine::Range(int Min, int Max) const
{
 CDiscretizedLine dl;

 for (int i = Min; i < Max; i++)
  dl.PushBack(vx[i], vy[i]);

 return dl;
}

/////////////////////////////////////////////////////////////////////////////
// Approximation by n points (almost equidistant)
/////////////////////////////////////////////////////////////////////////////
CDiscretizedLine CDiscretizedLine::UniformSplit(int n) const
{
 //
 // Build return value
 //
 CDiscretizedLine dl;
 dl.Resize(n);

 //
 // Same starting and ending points
 //
 dl.SetX(0) = X(0);
 dl.SetY(0) = Y(0);
 dl.SetX(n - 1) = X(Size() - 1);
 dl.SetY(n - 1) = Y(Size() - 1);

 //
 // Intermediate points
 //
 double TotalLength = Length();

 int j = 0;
 double Lj = 0.0;
 int k = NextIndex(j);
 double Lk = DistanceToNext(j);

 for (int i = 1; i < n - 1; i++)
 {
  double L = i * TotalLength / (n - 1);

  while (L > Lk && k < Size() - 1)
  {
   j = k;
   Lj = Lk;
   k = NextIndex(j);
   Lk = Lj + DistanceToNext(j);
  }

  double t = (L - Lj) / (Lk - Lj);
  double u = 1.0 - t;

  dl.SetX(i) = X(j) * u + X(k) * t;
  dl.SetY(i) = Y(j) * u + Y(k) * t;
 }

 return dl;
}

/////////////////////////////////////////////////////////////////////////////
// Test for angle
/////////////////////////////////////////////////////////////////////////////
bool CDiscretizedLine::IsAngle(int i) const
{
 if ((i == 0 || i == Size() - 1) && !IsClosed())
  return true;

 int Next = NextIndex(i);
 int Prev = PreviousIndex(i);

 double x1 = vx[Next] - vx[i];
 double y1 = vy[Next] - vy[i];
 double x2 = vx[Prev] - vx[i];
 double y2 = vy[Prev] - vy[i];

 double det = x1 * y2 - x2 * y1;

 double n1 = x1 * x1 + y1 * y1;
 double n2 = x2 * x2 + y2 * y2;

 return std::fabs(det / std::sqrt(n1 * n2)) > 0.4; // ???
}

/////////////////////////////////////////////////////////////////////////////
// Compute curvature
/////////////////////////////////////////////////////////////////////////////
double CDiscretizedLine::Curvature(int i) const
{
 double x1 = vx[i + 1] - vx[i];
 double y1 = vy[i + 1] - vy[i];
 double x2 = vx[i - 1] - vx[i];
 double y2 = vy[i - 1] - vy[i];
 double x3 = vx[i + 1] - vx[i - 1];
 double y3 = vy[i + 1] - vy[i - 1];

 double det = x1 * y2 - x2 * y1;

 double n1 = x1 * x1 + y1 * y1;
 double n2 = x2 * x2 + y2 * y2;
 double n3 = x3 * x3 + y3 * y3;

 return 2.0 * det / sqrt(n1 * n2 * n3);
}

/////////////////////////////////////////////////////////////////////////////
// Distance to next point
/////////////////////////////////////////////////////////////////////////////
double CDiscretizedLine::DistanceToNext(int i) const
{
 int Next = NextIndex(i);
 double dx = vx[Next] - vx[i];
 double dy = vy[Next] - vy[i];
 return std::sqrt(dx * dx + dy * dy);
}

/////////////////////////////////////////////////////////////////////////////
// Length
/////////////////////////////////////////////////////////////////////////////
double CDiscretizedLine::Length() const
{
 double Result = 0.0;
 for (int i = Size() - 1; --i >= 0;)
  Result += DistanceToNext(i);
 return Result;
}

/////////////////////////////////////////////////////////////////////////////
// Previous index
/////////////////////////////////////////////////////////////////////////////
int CDiscretizedLine::PreviousIndex(int i) const
{
 if (i == 0)
 {
  if (IsClosed())
   return Size() - 2;
  else
   return 0;
 }
 else
  return i - 1;
}

/////////////////////////////////////////////////////////////////////////////
// Next index
/////////////////////////////////////////////////////////////////////////////
int CDiscretizedLine::NextIndex(int i) const
{
 if (i == Size() - 1)
 {
  if (IsClosed())
   return 1;
  else
   return i;
 }
 else
  return i + 1;
}

/////////////////////////////////////////////////////////////////////////////
// Scale
/////////////////////////////////////////////////////////////////////////////
void CDiscretizedLine::Scale(double x)
{
 for (int i = int(vx.size()); --i >= 0;)
 {
  vx[i] *= x;
  vy[i] *= x;
 }
}

/////////////////////////////////////////////////////////////////////////////
// Simple tangent: parallel to the previous-next line
/////////////////////////////////////////////////////////////////////////////
void CDiscretizedLine::SimpleTangent(int i, double &dx, double &dy) const
{
 int prev = PreviousIndex(i);
 int next = NextIndex(i);

 dx = vx[next] - vx[prev];
 dy = vy[next] - vy[prev];

 double n = std::sqrt(dx * dx + dy * dy);
 if (n > 0.0)
 {
  dx /= n;
  dy /= n;
 }
}

/////////////////////////////////////////////////////////////////////////////
// Circular tangent: tangent to the circumscribed circle
/////////////////////////////////////////////////////////////////////////////
void CDiscretizedLine::CircularTangent(int i0, double &dx, double &dy) const
{
 if (Size() < 3)
 {
  SimpleTangent(i0, dx, dy);
  return;
 }

 int i1 = NextIndex(i0);
 int i2 = PreviousIndex(i0);

 if (i1 == i0)
  i1 = i2 - 1;

 if (i2 == i0)
  i2 = i1 + 1;

 double dx20 = vx[i1] - vx[i2];
 double dy20 = vy[i1] - vy[i2];
 double dx21 = vx[i1] - vx[i0];
 double dy21 = vy[i1] - vy[i0];
 double dx10 = vx[i0] - vx[i2];
 double dy10 = vy[i0] - vy[i2];

 double c = dx20 * dx10 + dy20 * dy10;
 double s = dx20 * dy10 - dy20 * dx10;

 dx = c * dx21 - s * dy21;
 dy = s * dx21 + c * dy21;

 double n = std::sqrt(dx * dx + dy * dy);
 if (n > 0.0)
 {
  dx /= n;
  dy /= n;
 }
}
