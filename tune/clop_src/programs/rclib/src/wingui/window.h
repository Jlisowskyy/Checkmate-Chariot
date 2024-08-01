///////////////////////////////////////////////////////////////////////////
//                                                                       
// window.h
//
// CWindow
//
// Rémi Coulom
//
// August, 1998
//
///////////////////////////////////////////////////////////////////////////
#ifndef WINDOW_H
#define WINDOW_H

class CWindow // win
{
 public: //////////////////////////////////////////////////////////////////
  virtual ~CWindow() {}
  virtual HWND GetHandle() const {return 0;}

  //
  // Gets
  //
  virtual int GetVisibility() const;
  virtual int GetX() const;
  virtual int GetY() const;

  //
  // Sets
  //
  virtual void SetVisibility(int f);
  virtual void SetX(int i);
  virtual void SetY(int i);
};

#endif
