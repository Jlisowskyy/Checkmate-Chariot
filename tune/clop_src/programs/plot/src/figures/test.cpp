/////////////////////////////////////////////////////////////////////////////
//
// test.cpp
//
// Test of surface plotting functions
//
// Rémi Coulom
// December, 2000
//
/////////////////////////////////////////////////////////////////////////////
#include <cmath>
#include <iostream>
#include <iomanip>

#include "CSurface.h"
#include "CPlot.h"
#include "CPlotCUI.h"

/////////////////////////////////////////////////////////////////////////////
// Function to plot
/////////////////////////////////////////////////////////////////////////////
class CTestSurface : public CSurface
{
 public:
#if 1
  double GetValue(double x, double y) const
  {
   double r2 = x * x + y * y;
   double s = std::sin(r2);
   double e = std::exp(-r2 * 0.25);
   return s * e + y * 0.2;
  }

  void GetValueAndGradient(double x, double y, double &z, double &gx, double &gy) const
  {
   double r2 = x * x + y * y;
   double s = std::sin(r2);
   double e = std::exp(-r2 * 0.25);
   z = s * e + y * 0.2;
   double c = std::cos(r2);
   double d = (c - 0.25 * s) * e;
   gx = 2 * x * d;
   gy = 2 * y * d + 0.2;
  }
#else
  double GetValue(double x, double y) const
  {
   const double Freq = 4.0;
   return std::sin(x * Freq) * std::sin(y * Freq);
  }
#endif
} surf;

/////////////////////////////////////////////////////////////////////////////
// Main function
/////////////////////////////////////////////////////////////////////////////
int main()
{
 CPlot plot(surf);
 plot.SetRanges(-2, 2, -2, 2, -1, 1);
 plot.SetObserver(2, -3, 2);
 plot.SetTarget(0, 0, -1);

 CPlotCUI plotcui(plot);
 plotcui.MainLoop(std::cin, std::cout);

 return 0;
}
