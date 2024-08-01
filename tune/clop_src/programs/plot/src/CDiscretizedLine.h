/////////////////////////////////////////////////////////////////////////////
//
// CDiscretizedLine
//
// Rémi Coulom
//
// May, 2009
//
/////////////////////////////////////////////////////////////////////////////
#ifndef Plot_CDiscretizedLine_Declared
#define Plot_CDiscretizedLine_Declared

#include <vector>

class CDiscretizedLine
{
 private: ///////////////////////////////////////////////////////////////////
  std::vector<double> vx;
  std::vector<double> vy;

 public: ////////////////////////////////////////////////////////////////////

  //
  // Constructors
  //
  CDiscretizedLine() {}
  CDiscretizedLine Range(int Min, int Max) const;
  CDiscretizedLine UniformSplit(int n) const;

  //
  // const methods
  //
  int Size() const {return int(vx.size());}
  double X(int i) const {return vx[i];}
  double Y(int i) const {return vy[i];}
  const double *px() const {return &vx[0];} // ??? must disappear
  const double *py() const {return &vy[0];} // ???
  bool IsClosed() const
  {
   return vx[vx.size() - 1] == vx[0] &&
          vy[vy.size() - 1] == vy[0];
  }

  bool IsAngle(int i) const;
  double Curvature(int i) const;
  double DistanceToNext(int i) const;
  double Length() const;

  int PreviousIndex(int i) const;
  int NextIndex(int i) const;

  void SimpleTangent(int i, double &dx, double &dy) const;
  void CircularTangent(int i, double &dx, double &dy) const;

  //
  // non-const methods
  //
  void Resize(int n) {vx.resize(n); vy.resize(n);}
  void PushBack(double x, double y) {vx.push_back(x); vy.push_back(y);}
  double &SetX(int i) {return vx[i];}
  double &SetY(int i) {return vy[i];}

  void Scale(double x);
};

#endif
