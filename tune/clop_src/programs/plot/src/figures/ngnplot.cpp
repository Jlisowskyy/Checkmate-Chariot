/////////////////////////////////////////////////////////////////////////////
//
// ngnplot.cpp
//
/////////////////////////////////////////////////////////////////////////////
#include "CSplineFit.h"
#include "random.h"

#include <iostream>
#include <cmath>

static const int n = 10000;
static const double xMin = -2.0;
static const double xMax = 7.0;
static const double xMinLearn = 1.0;
static const double xMaxLearn = 4.0;
static const double s = 0.25;

double tx[n];
double ty[n];
double tTarget[n];

static const int nGaussian = 4;
double txGaussian[nGaussian] = {1, 2, 3, 4};
double tw[nGaussian]         = {0, 0, 0, 0};
double tg[nGaussian];
double Sum;

/////////////////////////////////////////////////////////////////////////////
// Gaussian value
/////////////////////////////////////////////////////////////////////////////
double Gaussian(int i, double x)
{
 double d = x - txGaussian[i];
 return std::exp(-s * d * d);
}

/////////////////////////////////////////////////////////////////////////////
// NGN output
/////////////////////////////////////////////////////////////////////////////
double GetV(double x)
{
 Sum = 0;
 double V = 0;
 for (int j = nGaussian; --j >= 0;)
 {
  tg[j] = Gaussian(j, x);
  Sum += tg[j];
  V += tg[j] * tw[j];
 }
 return V / Sum;
}

/////////////////////////////////////////////////////////////////////////////
// main function
/////////////////////////////////////////////////////////////////////////////
int main()
{
 for (int i = 0; i < n; i++)
 {
  double x = xMin + (xMax - xMin) * double(i) / double(n-1);
  tx[i] = x;
  tTarget[i] = std::sin((x - 1) * 3.141592653589 / 2);
 }

 CRandom<unsigned> rnd;
 const double eta = 0.01;
 int iMin = int(n * (xMinLearn - xMin) / (xMax - xMin));
 int iMax = int(n * (xMaxLearn - xMin) / (xMax - xMin));
 for (int i = 3000000; --i >= 0;)
 {
  if (i % 100000 == 0)
   (std::cerr << '.').flush();
  int j = iMin + rnd.NewValue() % (iMax - iMin + 1);
  double Delta = GetV(tx[j]) - tTarget[j];

  for (int k = nGaussian; --k >= 0;)
   tw[k] -= eta * Delta * tg[k] / Sum;
 }
 (std::cerr << '\n').flush();

 for (int i = 0; i < n; i++)
  ty[i] = GetV(tx[i]);

 //
 // Write plot
 //
 std::cout << "\\begin{pspicture}(" << xMin << ",-1.2)(" << xMax << ",1.2)\n";
 std::cout << "\\psset{linewidth=0.4pt}\n";
 std::cout << "\\psline{->}(" << xMin << ",0)(" << xMax << ",0)\n";
 std::cout << "\\rput(1,-0.3){1}\n";
 std::cout << "\\rput(2,-0.3){2}\n";
 std::cout << "\\rput(3,-0.3){3}\n";
 std::cout << "\\rput(4,-0.3){4}\n";
 std::cout << "\\rput[tl](" << xMax << ",-0.1){$x$}\n";
 std::cout << "\\psline(1,-0.1)(1,0.1)\n";
 std::cout << "\\psline(2,-0.1)(2,0.1)\n";
 std::cout << "\\psline(3,-0.1)(3,0.1)\n";
 std::cout << "\\psline(4,-0.1)(4,0.1)\n";
 std::cout << "\\psset{linewidth=0.4pt,linestyle=dashed}\n";
 {
  //CSplineFit sfit(iMax - iMin + 1, tx + iMin, tTarget + iMin);
  //sfit.Fit(0.02);
  //sfit.PSTricks(std::cout);
 }
 std::cout << "\\psset{linewidth=1.2pt,linestyle=solid}\n";
 {
  //CSplineFit sfit(n, tx, ty);
  //sfit.Fit(0.01);
  //sfit.PSTricks(std::cout);
 }
 std::cout << "\\end{pspicture}\n";
}
