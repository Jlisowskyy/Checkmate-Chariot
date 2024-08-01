/////////////////////////////////////////////////////////////////////////////
//
// CColor.h
//
// Rémi Coulom
//
// April, 2009
//
/////////////////////////////////////////////////////////////////////////////
#ifndef Plot_CColor_Declared
#define Plot_CColor_Declared

class CColor // col
{
 private: ///////////////////////////////////////////////////////////////////
  float R;
  float G;
  float B;

 public: ////////////////////////////////////////////////////////////////////
  CColor(float r, float g, float b): R(r), G(g), B(b) {}

  float GetR() const {return R;}
  float GetG() const {return G;}
  float GetB() const {return B;}

  CColor Grayscale() const
  {
   float Level = 0.30f * R + 0.59f * G + 0.11f * B;
   return CColor(Level, Level, Level);
  }
};

#endif
