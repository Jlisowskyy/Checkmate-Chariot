////////////////////////////////////////////////////////////////////////////
//
// CPlotCUI.h
//
// Remi Coulom
//
// May, 2000
//
////////////////////////////////////////////////////////////////////////////
#ifndef Plot_CPlotCUI_Declared
#define Plot_CPlotCUI_Declared

#include "consolui.h"   // CConsoleUI

class CPlot;

class CPlotCUI : public CConsoleUI // plotcui
{
 private: //////////////////////////////////////////////////////////////////
  static const char * const tszCommands[];
  CPlot &plot;
  int GridSamples;
  int LineSamples;
  int Exclude;
  int fSmooth;
  int fDots;
  int Steps;
  double DMax;
  double zMin;
  double zMax;
  double LabelScale;

  void psline(int n,
              const double *px,
              const double *py,
              const int *pfDot,
              std::ostream &out) const;

 void curve(int n,
            const double *px,
            const double *py,
            double DMax,
            std::ostream &out) const;

  double VisibilityChange(double x0, double y0, int f0,
                          double x1, double y1, int f1) const;

  void PlotIsoLine(double x0,
                   double y0,
                   double x1,
                   double y1,
                   std::ostream &out) const;

  void PlotLine(double x0, double y0, double z0,
                double x1, double y1, double z1,
                std::ostream &out) const;

  void ParseCorner(const char *psz, double &x, double &y, double &z) const;

  void Contour(std::ostream &out, double Level, int W, int H) const;

 protected: ////////////////////////////////////////////////////////////////
  virtual int ProcessCommand(const char *pszCommand,
                             const char *pszParameters,
                             std::istream &in,
                             std::ostream &out);

  virtual void PrintLocalPrompt(std::ostream &out);

 public: ///////////////////////////////////////////////////////////////////
  explicit CPlotCUI(CPlot &plotInit,
                    CConsoleUI *pcui = 0,
                    int openmode = OpenModal);
};

#endif
