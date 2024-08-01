#ifdef BOUNDS_CHECK
#include "debug.h"
#endif

#include <cstddef>

template<typename T, std::size_t n> class rc_array
{
 private:
  T t[n];

 public:
  T& operator[](std::size_t i)
  {
#ifdef BOUNDS_CHECK
   FATAL(i >= n);
#endif
   return t[i];
  }

  const T& operator[](std::size_t i) const
  {
#ifdef BOUNDS_CHECK
   FATAL(i >= n);
#endif
   return t[i];
  }

  std::size_t size() const {return n;}
};
