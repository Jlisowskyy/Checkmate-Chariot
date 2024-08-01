#include <iostream>
#include <cstdlib>

#include "clktimer.h"   // CClockTimer
#include "chtime.h"     // CTime
#include "CTimeIO.h"
#include "random.h"     // CRandom
#include "BobJenkinsRNG.h"

#define ITERATIONS 1000000000

int main()
{
 std::cout << "Iterations = " << ITERATIONS << '\n';

 {
  std::cout << "Testing the CRandom class...\n";
  std::cout.flush();

  CRandom<unsigned> rnd;
  CClockTimer clkt;
  unsigned x = 0;
  for (long i = ITERATIONS; --i >= 0;)
   x += rnd.NewValue();
  std::cout << clkt.GetInterval() << '\n';
  std::cout << x << '\n';
 }

 {
  std::cout << "Bob Jenkins RNG...\n";
  std::cout.flush();

  BobJenkinsRNG bjrnd;
  CClockTimer clkt;
  unsigned x = 0;
  for (long i = ITERATIONS; --i >= 0;)
   x += bjrnd.NewValue();
  std::cout << clkt.GetInterval() << '\n';
  std::cout << x << '\n';
 }

 {
  std::cout << "Testing the rand() standard function...\n";
  std::cout.flush();

  CClockTimer clkt;
  unsigned x = 0;
  for (long i = ITERATIONS; --i >= 0;)
   x += rand();
  std::cout << clkt.GetInterval() << '\n';
  std::cout << x << '\n';
 }

 return 0;
}

