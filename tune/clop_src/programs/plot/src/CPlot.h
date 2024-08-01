/////////////////////////////////////////////////////////////////////////////
//
// CPlot.h
//
// Rémi Coulom
//
// December, 2000
//
/////////////////////////////////////////////////////////////////////////////
#ifndef Plot_CPlot_Declared
#define Plot_CPlot_Declared

class CSurface;

class CPlot // plot
{
 private: /////////////////////////////////////////////////////////////////
  double tObserver[3];
  double tTarget[3];
  double Dist;

  double xMin;
  double xMax;
  double yMin;
  double yMax;
  double zMin;
  double zMax;

  const CSurface &surf;

  double tTransform[9];

  void UpdateTransform();
  void Bound(double &x, double &y) const;
  int Visible4(double x, double y, double z, double s) const;

 public: //////////////////////////////////////////////////////////////////
  explicit CPlot(const CSurface &surfInit);

  const CSurface &GetSurface() const {return surf;}

  const double *GetObserver() const {return tObserver;}
  const double *GetTarget() const {return tTarget;}
  double GetXMin() const {return xMin;}
  double GetXMax() const {return xMax;}
  double GetYMin() const {return yMin;}
  double GetYMax() const {return yMax;}
  double GetZMin() const {return zMin;}
  double GetZMax() const {return zMax;}
  double GetDist() const {return Dist;}

  void Transform(double x, double y, double z, double &x2, double &y2) const;
  void Projection(double x, double y, double &x2, double &y2) const;
  int Visible(double x, double y) const;
  int Visible3(double x, double y, double z);
  void BoundingBox(double &x0, double &y0, double &x1, double &y1) const;
  double Scale(double x, double y, double z) const;

  void SetRanges(double a, double b, double c, double d, double e, double f);

  void SetObserver(double x, double y, double z);
  void SetTarget(double x, double y, double z);
  void SetDist(double NewDist) {Dist = NewDist;}
};

#endif
