/////////////////////////////////////////////////////////////////////////////
//
// pgftest.cpp
//
// Test plotting 2D functions with pgf
//
/////////////////////////////////////////////////////////////////////////////
#include "CSplineFit.h"
#include "CDiscretizedLine.h"

#include <iostream>
#include <cmath>

static const int n = 1000;
static const double Scale = 5.0;
static const double D = 0.01;

/////////////////////////////////////////////////////////////////////////////
// draw a sigmoid of given steepness
/////////////////////////////////////////////////////////////////////////////
void Sigmoid(double Steepness)
{
 CDiscretizedLine dl;
 dl.Resize(n);

 for (int i = n; --i >= 0;)
 {
  double x = (2.0 * i) / (n - 1) - 1;
  double y = 1.0 / (1.0 + std::exp(-Steepness*x));

  dl.SetX(i) = x * Scale;
  dl.SetY(i) = y * Scale;
 }

 CSplineFit sfit(dl);
 sfit.Fit(D);
 sfit.TikZ();
}

/////////////////////////////////////////////////////////////////////////////
// main function
/////////////////////////////////////////////////////////////////////////////
int main()
{
 std::cout << "\\begin{tikzpicture}\n";
 for (int i = 0; i < 20; i+=2)
  Sigmoid(i);
 std::cout << "\\draw (-" << Scale << "," << Scale << ") rectangle (" << Scale << ",0);\n";
 std::cout << "\\pgfusepath{stroke}\n";
 std::cout << "\\end{tikzpicture}\n";

 return 0;
}
