/////////////////////////////////////////////////////////////////////////////
//
// CPlot.cpp
//
// Rémi Coulom
//
// December, 2000
//
/////////////////////////////////////////////////////////////////////////////
#include <math.h>

#include "debug.h"
#include "CPlot.h"
#include "CSurface.h"

/////////////////////////////////////////////////////////////////////////////
// Normalize a 3D vector
/////////////////////////////////////////////////////////////////////////////
static void Normalize(double *pd)
{
 double N = sqrt(pd[0] * pd[0] + pd[1] * pd[1] + pd[2] * pd[2]);
 FATAL(N == 0);
 pd[0] /= N;
 pd[1] /= N;
 pd[2] /= N;
}

/////////////////////////////////////////////////////////////////////////////
// Update the transform matrix
/////////////////////////////////////////////////////////////////////////////
void CPlot::UpdateTransform()
{
 //
 // Normal vector in the observer's z direction
 //
 {
  tTransform[6] = tTarget[0] - tObserver[0];
  tTransform[7] = tTarget[1] - tObserver[1];
  tTransform[8] = tTarget[2] - tObserver[2];
  Normalize(tTransform + 6);
 }

 //
 // Normal horizontal vector to the right
 //
 {
  tTransform[0] = tTransform[7];
  tTransform[1] = -tTransform[6];
  tTransform[2] = 0;
  Normalize(tTransform);
 }

 //
 // Third vector orthogonal to the others
 //
 {
  tTransform[3] = tTransform[7] * tTransform[2] -
                  tTransform[8] * tTransform[1];
  tTransform[4] = tTransform[8] * tTransform[0] -
                  tTransform[6] * tTransform[2];
  tTransform[5] = tTransform[6] * tTransform[1] -
                  tTransform[7] * tTransform[0];
 }
}

/////////////////////////////////////////////////////////////////////////////
// Relative scale with the target
/////////////////////////////////////////////////////////////////////////////
double CPlot::Scale(double x, double y, double z) const
{
 double tx = tTarget[0] - tObserver[0];
 double ty = tTarget[1] - tObserver[1];
 double tz = tTarget[2] - tObserver[2];

 double rx = x - tObserver[0];
 double ry = y - tObserver[1];
 double rz = z - tObserver[2];

 double r2 = rx * rx + ry * ry + rz * rz;
 double t2 = tx * tx + ty * ty + tz * tz;

 if (r2 > 0)
  return sqrt (t2 / r2);
 else
  return 1.0;
}

/////////////////////////////////////////////////////////////////////////////
// Conic projection
/////////////////////////////////////////////////////////////////////////////
void CPlot::Transform(double x,
                      double y,
                      double z,
                      double &x2,
                      double &y2) const
{
 x -= tObserver[0];
 y -= tObserver[1];
 z -= tObserver[2];

 double xt = tTransform[0] * x + tTransform[1] * y + tTransform[2] * z;
 double yt = tTransform[3] * x + tTransform[4] * y + tTransform[5] * z;
 double zt = tTransform[6] * x + tTransform[7] * y + tTransform[8] * z;

 x2 = Dist * xt / zt;
 y2 = -Dist * yt / zt;
}

/////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////
CPlot::CPlot(const CSurface &surfInit):
 surf(surfInit)
{
 tObserver[0] = tObserver[1] = tObserver[2] = 10.0;
 tTarget[0] = tTarget[1] = tTarget[2] = 0.0;
 xMin = yMin = zMin = -1.0;
 xMax = yMax = zMax = 1.0;
 Dist = 1.0;
 UpdateTransform();
}

/////////////////////////////////////////////////////////////////////////////
// Set observer position
/////////////////////////////////////////////////////////////////////////////
void CPlot::SetObserver(double x, double y, double z)
{
 tObserver[0] = x;
 tObserver[1] = y;
 tObserver[2] = z;
 UpdateTransform();
}

/////////////////////////////////////////////////////////////////////////////
// Set target point
/////////////////////////////////////////////////////////////////////////////
void CPlot::SetTarget(double x, double y, double z)
{
 tTarget[0] = x;
 tTarget[1] = y;
 tTarget[2] = z;
 UpdateTransform();
}

/////////////////////////////////////////////////////////////////////////////
// Set Ranges
/////////////////////////////////////////////////////////////////////////////
void CPlot::SetRanges(double a, double b, double c, double d, double e, double f)
{
 xMin = a;
 xMax = b;
 yMin = c;
 yMax = d;
 zMin = e;
 zMax = f;
}

/////////////////////////////////////////////////////////////////////////////
// Compute projected position of a point on the surface
/////////////////////////////////////////////////////////////////////////////
void CPlot::Projection(double x, double y, double &x2, double &y2) const
{
 Transform(x, y, surf.GetValue(x, y), x2, y2);
}

/////////////////////////////////////////////////////////////////////////////
// Make sure we are within the plotted box
/////////////////////////////////////////////////////////////////////////////
void CPlot::Bound(double &x, double &y) const
{
 if (x < xMin)
  x = xMin;
 if (x > xMax)
  x = xMax;
 if (y < yMin)
  y = yMin;
 if (y > yMax)
  y = yMax;
}

/////////////////////////////////////////////////////////////////////////////
// Check intersection of a ray with the surface
/////////////////////////////////////////////////////////////////////////////
int CPlot::Visible4(double x, double y, double z, double s) const
{
 double v[3];
 v[0] = tObserver[0] - x;
 v[1] = tObserver[1] - y;
 v[2] = tObserver[2] - z;

 //
 // Get intersection of the ray with the bounding box of the curve
 //
 double tInter[3];
 {
  double det[4];
  det[0] = (xMin - x) * v[1] - (yMin - y) * v[0];
  det[1] = (xMax - x) * v[1] - (yMin - y) * v[0];
  det[2] = (xMax - x) * v[1] - (yMax - y) * v[0];
  det[3] = (xMin - x) * v[1] - (yMax - y) * v[0];

  double alpha = 0;
  if (det[0] >= 0 && det[1] < 0)
   alpha = (y - yMin) / (y - tObserver[1]);
  else if (det[1] >= 0 && det[2] < 0)
   alpha = (x - xMax) / (x - tObserver[0]);
  else if (det[2] >= 0 && det[3] < 0)
   alpha = (y - yMax) / (y - tObserver[1]);
  else if (det[3] >= 0 && det[0] < 0)
   alpha = (x - xMin) / (x - tObserver[0]);

  tInter[0] = alpha * (tObserver[0] - x);
  tInter[1] = alpha * (tObserver[1] - y);
  tInter[2] = alpha * (tObserver[2] - z);
 }

 //
 // Early exit if point close to border
 //
 if (tInter[0] * tInter[0] + tInter[1] * tInter[1] <
     0.0001 * ((xMin - xMax) * (xMin - xMax) + (yMin - yMax) * (yMin - yMax)))
  return 1;

 //
 // Find side
 //
 if (s == 0.0)
 {
  double tRay[3];
  tRay[0] = x + 0.001 * tInter[0];
  tRay[1] = y + 0.001 * tInter[1];
  tRay[2] = z + 0.001 * tInter[2];

  double az = surf.GetValue(tRay[0], tRay[1]); 

  if (tRay[2] - az > 0.0)
   s = 1.0;
  else
   s = -1.0;
 }

 //
 // Find intersections of the ray with the curve
 //
 for (double alpha = 0.001; alpha <= 1;)
 {
  double tRay[3];
  tRay[0] = x + alpha * tInter[0];
  tRay[1] = y + alpha * tInter[1];
  tRay[2] = z + alpha * tInter[2];

  double d = tRay[2] - surf.GetValue(tRay[0], tRay[1]); 

  if (s * d < 0.0)
   return 0;

  double Step = fabs(d) / 20;
  if (Step < 0.001)
   Step = 0.001;
  alpha += Step;
 }

 return 1;
}

/////////////////////////////////////////////////////////////////////////////
// Test if a point on the surface is visible
/////////////////////////////////////////////////////////////////////////////
int CPlot::Visible(double x, double y) const
{
 Bound(x, y);
 
 if (surf.HasReliableGradient())
 {
  double z;
  double gx;
  double gy;
  surf.GetValueAndGradient(x, y, z, gx, gy);

  double v[3];
  v[0] = tObserver[0] - x;
  v[1] = tObserver[1] - y;
  v[2] = tObserver[2] - z;

  double s = -gx * v[0] - gy * v[1] + v[2];

  return Visible4(x, y, z, s);
 }
 else
  return Visible4(x, y, surf.GetValue(x, y), 0);
}

/////////////////////////////////////////////////////////////////////////////
// Test whether a point not on the surface is visible
/////////////////////////////////////////////////////////////////////////////
int CPlot::Visible3(double x, double y, double z)
{
 Bound(x, y);
 return Visible4(x, y, z, z - surf.GetValue(x, y));
}

/////////////////////////////////////////////////////////////////////////////
// 2D Bounding Box of a 3D Box
/////////////////////////////////////////////////////////////////////////////
void CPlot::BoundingBox(double &x0, double &y0, double &x1, double &y1) const
{
 Transform(xMin, yMin, zMin, x0, y0);
 x1 = x0;
 y1 = y0;

 for (int i = 2; --i >= 0;)
  for (int j = 2; --j >= 0;)
   for (int k = 2; --k >= 0;)
   {
    double x;
    double y;
    Transform(xMin + double(i) * (xMax - xMin),
              yMin + double(j) * (yMax - yMin),
              zMin + double(k) * (zMax - zMin),
              x,
              y);
    if (x < x0)
     x0 = x;
    if (y < y0)
     y0 = y;
    if (x > x1)
     x1 = x;
    if (y > y1)
     y1 = y;
   }
}
