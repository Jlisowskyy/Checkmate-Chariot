/////////////////////////////////////////////////////////////////////////////
//
// CScopedPointer.h
//
// Rémi Coulom
//
// March, 2011
//
/////////////////////////////////////////////////////////////////////////////
#ifndef CScopedPointer_Declared
#define CScopedPointer_Declared

template<typename T> class CScopedPointer // p
{
 private:
  T* p;

 public:
  CScopedPointer(T *p): p(p) {}

  T &operator*() const {return *p;}
  T *operator->() const {return p;}

  ~CScopedPointer() {delete p;}
};

#endif
