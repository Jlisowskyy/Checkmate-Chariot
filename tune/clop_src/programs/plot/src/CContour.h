/////////////////////////////////////////////////////////////////////////////
//
// CContour.h
//
// Rémi Coulom
//
// January, 2002
//
/////////////////////////////////////////////////////////////////////////////
#ifndef Plot_CContour_Declared
#define Plot_CContour_Declared

#include <iosfwd>
#include <vector>
#include "CSurface.h"

class CDiscretizedLine;

class CContour // contour
{
 private: /////////////////////////////////////////////////////////////////
  const CSurface &surf;

  double xMin;
  double xMax;
  double yMin;
  double yMax;

  int GridWidth;
  int GridHeight;
  int GridSize;
  int tDirection[4];

  std::vector<int> vGrid;
  std::vector<int> vBorderType;

  double Level;
  int Components;
  int NegativeComponents;
  int BorderTypes;
  std::vector<int> vInside;
  std::vector<int> vOutside;

  double GetX(int i) const
  {
   return xMin + (xMax - xMin) * double(i) / double(GridWidth);
  }

  double GetY(int i) const
  {
   return yMin + (yMax - yMin) * double(i) / double(GridHeight);
  }

  int Index(int i, int j) const {return (j + 1) * (GridWidth + 3) + i + 1;}

  void MarkComponent(int i, int v);
  int PlotRight(int i, int Dir, double &x, double &y) const;

  double GetZ(double x, double y) const {return surf.GetValue(x, y) - Level;}

  void RefinePoint(double x0,
                   double y0,
                   double z0,
                   double x1,
                   double y1,
                   double z1,
                   double &x,
                   double &y) const;

  void RefineLine(CDiscretizedLine &dl) const;

 public: //////////////////////////////////////////////////////////////////
  explicit CContour(const CSurface &surfInit);

  const CSurface &GetSurface() const {return surf;}

  double GetXMin() const {return xMin;}
  double GetXMax() const {return xMax;}
  double GetYMin() const {return yMin;}
  double GetYMax() const {return yMax;}

  int GetComponents() const;
  void PlotComponent(int Component,
                     std::ostream &out,
                     int fBorder,
                     double dMax,
                     double Scale);
  void PlotLines(std::ostream &out, double dMax, double Scale);

  void SetBounds(double x, double X, double y, double Y);
  void SetGridSize(int Width, int Height);
  void SetLevel(double NewLevel);
};

#endif
