/////////////////////////////////////////////////////////////////////////////
//
// illcond.cpp
//
/////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <cmath>

static const double LevelStep = 1.0;
static const double w1Min = -4.0;
static const double w1Max = 4.0;
static const double w2Min = -2.0;
static const double w2Max = 2.0;
static const double Margin = 0.2;

#if 1
static const double Lambda1 = 1.0;
static const double Lambda2 = 8.0;
static const double Eta = 0.7 / Lambda2;
static const int Steps = 16;
static const char *pszCaption = "$\\eta = 0.7/\\lambda_2$";
#elif 1
static const double Lambda1 = 1.0;
static const double Lambda2 = 8.0;
static const double Eta = 1.7 / Lambda2;
static const int Steps = 8;
static const char *pszCaption = "$\\eta = 1.7/\\lambda_2$";
#elif 1
static const double Lambda1 = 1.0;
static const double Lambda2 = 8.0;
static const double Eta = 2.1 / Lambda2;
static const int Steps = 4;
static const char *pszCaption = "$\\eta = 2.1/\\lambda_2$";
#endif

/////////////////////////////////////////////////////////////////////////////
// main function
/////////////////////////////////////////////////////////////////////////////
int main()
{
 std::cout.setf(std::ios::fixed, std::ios::floatfield);

 std::cout << "\\subfigure[" << pszCaption << "]{%\n";
 std::cout << "\\begin{pspicture}";
 std::cout << '(' << w1Min << "," << w2Min << ')';
 std::cout << '(' << w1Max << "," << w2Max + 0.5 << ')';
 std::cout << "\n";

 //
 // Axes
 //
 std::cout << "\\psset{linewidth=0.4pt}\n";
 std::cout << "\\psline{->}(" << w1Min << ",0)(" << w1Max << ",0)\n";
 std::cout << "\\psline{->}(0," << w2Min << ")(0," << w2Max << ")\n";
 std::cout << "\\rput[tl](" << w1Max << ",-0.1){$w_1$}\n";
 std::cout << "\\rput[r](-0.1," << w2Max << "){$w_2$}\n";

 //
 // Lines of constant E
 //
 std::cout << "\\psset{linewidth=0.4pt}\n";
 std::cout << "\\psclip{\\psframe[linestyle=none]";
 std::cout << '(' << w1Min + Margin << "," << w2Min + Margin << ')';
 std::cout << '(' << w1Max - Margin << "," << w2Max - Margin << ')';
 std::cout << "}\n";
 {
  double Level = 0;
  double M = Lambda1 * (w1Max - Margin) * (w1Max - Margin) +
             Lambda2 * (w2Max - Margin) * (w2Max - Margin);
  while((Level += LevelStep) < M)
  {
   double w1 = std::sqrt(2 * Level / Lambda1);
   double w2 = std::sqrt(2 * Level / Lambda2);
   std::cout << "\\psellipse(" << w1 << "," << w2 << ")\n";
  }
 }
 std::cout << "\\endpsclip\n";

 //
 // Learning trajectory
 //
 std::cout << "\\psset{linewidth=1.2pt,showpoints=true}\n";
 {
  double w1 = w1Min + Margin;
  double w2 = w2Max - Margin;
  for (int i = Steps; --i >= 0;)
  {
   std::cout << "\\psline";
   if (i == 0)
    std::cout << "{->}";
   std::cout << "(" << w1 << "," << w2 << ")(";
   w1 -= Eta * Lambda1 * w1;
   w2 -= Eta * Lambda2 * w2;
   std::cout << w1 << "," << w2 << ")\n";
  }
  std::cout << "\\psdot[dotstyle=o]";
  std::cout << "(" << w1Min + Margin << "," << w2Max - Margin << ")\n";
 }

 std::cout << "\\end{pspicture}}\n";
}
