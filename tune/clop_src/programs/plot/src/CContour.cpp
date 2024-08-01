/////////////////////////////////////////////////////////////////////////////
//
// CContour.cpp
//
// Rémi Coulom
//
// January, 2002
//
/////////////////////////////////////////////////////////////////////////////
#include "CContour.h"
#include "CSurface.h"
#include "CSplineFit.h"
#include "CDiscretizedLine.h"

/////////////////////////////////////////////////////////////////////////////
// Constructor
/////////////////////////////////////////////////////////////////////////////
CContour::CContour(const CSurface &surfInit):
 surf(surfInit),
 xMin(-1),
 xMax(+1),
 yMin(-1),
 yMax(+1)
{
 SetGridSize(20, 20);
}

/////////////////////////////////////////////////////////////////////////////
// Plot lines
/////////////////////////////////////////////////////////////////////////////
void CContour::PlotLines(std::ostream &out, double dMax, double Scale)
{
 for (int i = BorderTypes; --i >= 0;)
  PlotComponent(i, out, 0, dMax, Scale);
}

/////////////////////////////////////////////////////////////////////////////
// False position method to find precise point between p0 and p1
/////////////////////////////////////////////////////////////////////////////
void CContour::RefinePoint(double x0, 
                           double y0, 
                           double z0, 
                           double x1, 
                           double y1, 
                           double z1, 
                           double &x,
                           double &y) const
{
 for (int j = 4;;)
 {
  x = (z0 * x1 - z1 * x0) / (z0 - z1);
  y = (z0 * y1 - z1 * y0) / (z0 - z1);

  if (--j < 0)
   break;

  double z = GetZ(x, y);

  if (z * z0 > 0)
  {
   x0 = x;
   y0 = y;
   z0 = z;
  }
  else
  {
   x1 = x;
   y1 = y;
   z1 = z;
  }
 }
}

/////////////////////////////////////////////////////////////////////////////
// Find point to plot
//  return 1 if point found
//  return 0 if on the border of the grid
/////////////////////////////////////////////////////////////////////////////
int CContour::PlotRight(int i, int Dir, double &x, double &y) const
{
 double x0 = GetX(i % (GridWidth + 3) - 1);
 double y0 = GetY(i / (GridWidth + 3) - 1);

 int iRight = i + tDirection[(Dir + 1) & 3];

 if (vGrid[iRight] < 0)
 {
  double x1 = GetX(iRight % (GridWidth + 3) - 1);
  double y1 = GetY(iRight / (GridWidth + 3) - 1);

  RefinePoint(x0, y0, GetZ(x0, y0), x1, y1, GetZ(x1, y1), x, y);
  return 1;
 }

 return 0;
}

/////////////////////////////////////////////////////////////////////////////
// Plot components
// ??? is it possible to have more than one line for a BorderType ???
// ??? should return a dl instead of SplineFitting it ???
/////////////////////////////////////////////////////////////////////////////
void CContour::PlotComponent(int BorderType,
                             std::ostream &out,
                             int fBorder,
                             double dMax,
                             double Scale)
{
 //
 // Find the starting point
 //
 int i0 = GridSize;
 while (vBorderType[--i0] != BorderType)
  if (i0 == 0)
   return;
 const int Component = vGrid[i0];

 //
 // Rotate so that non-component is to the right
 //
 int Dir0 = 4;
 while (--Dir0 >= 0)
  if (vGrid[i0 + tDirection[(Dir0 + 1) & 3]] != Component)
   break;

 //
 // Initialize for main loop
 //
 int i = i0;
 int Dir = Dir0;
 int Count = 0;
 int fJoin = 0;

 std::vector<CDiscretizedLine> vLine;

 int Line = 0;
 vLine.resize(Line + 1);
 
 //
 // Walk around this component, with the component to the left
 //
 while (1)
 {
  vBorderType[i] = BorderType;

  //
  // If point to the right is part of this component, then turn right
  //
  if (vGrid[i + tDirection[(Dir + 1) & 3]] == Component)
  {
   Dir = (Dir + 1) & 3;
   i += tDirection[Dir];
  }

  //
  // Otherwise, find the border point
  //
  else
  {
   //
   // Detect that we are back to the starting point
   //
   if (i == i0 && Dir == Dir0 && ++Count > 1)
    break;

   //
   // If the border point is found, add it to the line
   //
   double x, y;
   if (PlotRight(i, Dir, x, y))
   {
    vLine[Line].PushBack(x, y);
    if (i == i0 && Dir == Dir0)
     fJoin = 1;
   }

   //
   // If we were in a line, and we reach the border, then we create another line
   //
   else if (vLine[Line].Size() > 0)
   {
    Line++;
    vLine.resize(Line + 1);
   }

   //
   // Move forward or turn left
   //
   if (vGrid[i + tDirection[Dir]] == Component)
    i += tDirection[Dir];
   else
    Dir = (Dir + 3) & 3;
  }
 }

 //
 // Merge first and last line if they are connected
 //
 int FirstLine = 0;
 if (vLine[Line].Size())
  if (fJoin && Line > 0)
  {
   for (int j = 0; j < vLine[0].Size(); j++)
    vLine[Line].PushBack(vLine[0].X(j), vLine[0].Y(j));
   Line++;
   FirstLine = 1;
   fJoin = 0;
  }
  else
   Line++;
 else
  fJoin = 0;

 //
 // Loop over lines, to plot them
 //
 for (int j = FirstLine; j < Line; j++)
 {
  if (Line == 1 && fJoin)
   vLine[j].PushBack(vLine[j].X(0), vLine[j].Y(0));

  CDiscretizedLine dl = vLine[j].UniformSplit(vLine[j].Size() * 20);
  RefineLine(dl);
  dl.Scale(Scale);
  CSplineFit::PGF(dl, dMax, out);
 }
}

/////////////////////////////////////////////////////////////////////////////
// Refine
/////////////////////////////////////////////////////////////////////////////
void CContour::RefineLine(CDiscretizedLine &dl) const
{
 CDiscretizedLine dl2 = dl;

 for (int i = dl.Size() - 1; --i > 0;)
 {
  int next = dl.NextIndex(i);
  int prev = dl.PreviousIndex(i);

  double x0 = dl.X(i);
  double y0 = dl.Y(i);
  double z0 = GetZ(x0, y0);

  double dx = dl.X(next) - dl.X(prev);
  double dy = dl.Y(next) - dl.Y(prev);

  double alpha = z0 > 0 ? 0.1 : -0.1;

  for (int j = 10; --j >= 0;) 
  {
   double x1 = x0 + alpha * dy;
   double y1 = y0 - alpha * dx;
   double z1 = GetZ(x1, y1);

   if (z1 * z0 < 0)
   {
    RefinePoint(x0, y0, z0, x1, y1, z1, dl2.SetX(i), dl2.SetY(i));
    break;
   }

   alpha *= 2.0;
  }
 }

 dl = dl2;
}

/////////////////////////////////////////////////////////////////////////////
// Set bounds
/////////////////////////////////////////////////////////////////////////////
void CContour::SetBounds(double x, double X, double y, double Y)
{
 xMin = x;
 xMax = X;
 yMin = y;
 yMax = Y;
}

/////////////////////////////////////////////////////////////////////////////
// Set grid size
/////////////////////////////////////////////////////////////////////////////
void CContour::SetGridSize(int Width, int Height)
{
 tDirection[0] = Width + 3;
 tDirection[1] = 1;
 tDirection[2] = -Width - 3;
 tDirection[3] = -1;

 GridSize = (Width + 3) * (Height + 3);
 vGrid.resize(GridSize);
 vBorderType.resize(GridSize);

 GridWidth = Width;
 GridHeight = Height;
}

/////////////////////////////////////////////////////////////////////////////
// Component floodfill
/////////////////////////////////////////////////////////////////////////////
void CContour::MarkComponent(int i, int v)
{
 int *tPoint = new int[GridSize];
 int Points = 1;
 tPoint[0] = i;

 while (Points)
 {
  Points--;
  int p = tPoint[Points];
  vGrid[p] = v;

  for (int j = 4; --j >= 0;)
  {
   int pp = p + tDirection[j];
   if (vGrid[pp] != v && vGrid[pp] * v > 0)
    tPoint[Points++] = pp;
  }
 }

 delete[] tPoint;
}

//#include <iomanip>
/////////////////////////////////////////////////////////////////////////////
// Set contour levels
/////////////////////////////////////////////////////////////////////////////
void CContour::SetLevel(double NewLevel)
{
 Level = NewLevel;

 for (int i = GridSize; --i >= 0;)
  vGrid[i] = 0;

 for (int j = 0; j <= GridHeight; j++)
  for (int i = 0; i <= GridWidth; i++)
  {
   double z = surf.GetValue(GetX(i), GetY(j));
   vGrid[Index(i, j)] = z > Level ? GridSize : -GridSize;
  }

 //
 // Create positive components
 //
 Components = 0;
 for (int i = GridSize; --i >= 0;)
  if (vGrid[i] > Components)
  {
   Components++;
   MarkComponent(i, Components);
  }

 //
 // Create negative components
 //
 NegativeComponents = 0;
 for (int i = GridSize; --i >= 0;)
  if (vGrid[i] < -NegativeComponents)
  {
   NegativeComponents++;
   MarkComponent(i, -NegativeComponents);
  }

 //
 // List all border types
 //
 BorderTypes = 0;
 for (int i = GridSize; --i >= 0;)
 {
  vBorderType[i] = -1;

  if (vGrid[i] > 0)
   for (int j = 4; --j >= 0;)
   {
    int x = vGrid[i + tDirection[j]];
    if (x < 0)
    {
     bool fFound = false;

     for (int b = BorderTypes; --b >= 0;)
      if (x == vOutside[b] && vGrid[i] == vInside[b])
      {
       vBorderType[i] = b;
       fFound = true;
       break;
      }

     if (!fFound)
     {
      vInside.push_back(vGrid[i]);
      vOutside.push_back(x);
      vBorderType[i] = BorderTypes;
      BorderTypes++;
     }

     break;
    }
   }
 }

 //
 // Debug output
 //
// std::cerr << '\n';
// for (int j = GridHeight; j >= 0; j--)
// {
//  for (int i = 0; i <= GridWidth; i++)
//   std::cerr << std::setw(4) << vBorderType[Index(i, j)];
//  std::cerr << '\n';
// }
}
