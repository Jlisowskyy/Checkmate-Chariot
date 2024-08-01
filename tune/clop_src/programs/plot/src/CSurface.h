/////////////////////////////////////////////////////////////////////////////
//
// CSurface.h
//
// Rémi Coulom
//
// December, 2000
//
/////////////////////////////////////////////////////////////////////////////
#ifndef Plot_CSurface_Declared
#define Plot_CSurface_Declared

class CSurface // surf
{
 private: /////////////////////////////////////////////////////////////////
  const int fReliableGradient;

 public: //////////////////////////////////////////////////////////////////
  explicit CSurface(int f = 1) : fReliableGradient(f) {}

  int HasReliableGradient() const {return fReliableGradient;}
  virtual double GetValue(double x, double y) const = 0;
  virtual void GetValueAndGradient(double x,
                                   double y,
                                   double &z,
                                   double &gx,
                                   double &gy) const
  {
   z = GetValue(x, y);
   gx = gy = 0;
  }

  virtual ~CSurface() {}
};

#endif
