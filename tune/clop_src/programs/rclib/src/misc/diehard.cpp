#include "random.h"
#include "BobJenkinsRNG.h"

#include <fstream>
#include <stdint.h>

static const int size = 4000000;

int main()
{
 std::ofstream ofs("diehard.bin", std::ios::binary);
 //CRandom<uint32_t> rnd;
 BobJenkinsRNG rnd;
 for (int i = size; --i >= 0;)
 {
  uint32_t r = rnd.NewValue();
  ofs.write((const char *)&r, sizeof(r));
 }
 return 0;
}
