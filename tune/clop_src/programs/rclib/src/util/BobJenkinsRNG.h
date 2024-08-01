// adapted from public domain code by Bob Jenkins:
// http://www.burtleburtle.net/bob/rand/smallprng.html
#ifndef BobJenkinsRNG_declared
#define BobJenkinsRNG_declared

#include <stdint.h>

class BobJenkinsRNG
{
 private:
  uint32_t a;
  uint32_t b;
  uint32_t c;
  uint32_t d;

 public:
  #define rot(x,k) (((x)<<(k))|((x)>>(32-(k))))
  uint32_t NewValue()
  {
   uint32_t e = a - rot(b, 27);
   a = b ^ rot(c, 17);
   b = c + d;
   c = d + e;
   d = e + a;
   return d;
  }
  #undef rot

  BobJenkinsRNG(uint32_t seed = 0):
   a(0xf1ea5eed),
   b(seed),
   c(seed),
   d(seed)
  {
   for (int i = 20; --i >= 0;)
    NewValue();
  }
};

#endif
