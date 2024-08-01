#include <iostream>
#include "random.h"

const int nAppels = 5;
const int nExperiences = 1000000;
const int nBatons = 24;
int nVals[nBatons];

////////////////////////////////////////////////////////////////////////////
// Programme principal
////////////////////////////////////////////////////////////////////////////
int main(void)
{
 CRandom<unsigned> rnd;

 double Total = 0;
 for (int i = nExperiences; --i >= 0;)
 {
  int Indice = 0;
  double x = 0;
#if 0
   x = rnd.NextDouble();
   Indice = int(nBatons * x);
#endif
#if 1
   x = rnd.NextGaussian();
   Indice = int(nBatons * (x + 3) / 6);
#endif
#if 0
   x = rnd.NextExponential();
   Indice = int(nBatons * x / 4);
#endif
  Total += x;

  if (Indice >= 0 && Indice < nBatons)
   nVals[Indice]++;
 }

 //
 // Affichage de l'histogramme
 //
 {
  for (int i = 0; i < nBatons; i++)
  {
   for (int j = (nVals[i] * nBatons * 20 + nExperiences - 1) / nExperiences; --j >= 0;)
    std::cout << '#';
   std::cout << '\n';
  }
 }
 std::cout << "Moyenne = " << Total / nExperiences << '\n';

 return 0;
}
