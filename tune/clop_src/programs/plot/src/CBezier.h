/////////////////////////////////////////////////////////////////////////////
//
// CBezier.h
//
// Rémi Coulom
//
// February, 2009
//
/////////////////////////////////////////////////////////////////////////////
#ifndef Plot_CBezier_Declared
#define Plot_CBezier_Declared

class CBezier
{
 public: ////////////////////////////////////////////////////////////////////
  double tx[4];
  double ty[4];

  void GetValue(double t, double &x, double &y) const
  {
   double u = 1 - t;
   x = tx[0] * u * u * u +
       3 * t * u * (tx[1] * u + tx[2] * t) +
       tx[3] * t * t * t;
   y = ty[0] * u * u * u +
       3 * t * u * (ty[1] * u + ty[2] * t) +
       ty[3] * t * t * t;
  }

  void GetDerivative(double t, double &x, double &y) const
  {
   double u = 1 - t;
   x = 3 * ((tx[1] - tx[0]) * u * u +
            (tx[2] - tx[1]) * u * t * 2 +
            (tx[3] - tx[2]) * t * t);
   y = 3 * ((ty[1] - ty[0]) * u * u +
            (ty[2] - ty[1]) * u * t * 2 +
            (ty[3] - ty[2]) * t * t);
  }
};

#endif
