/* Dummy.c */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>

/* random float between 0 and 1 */
static float rand_float()
{
 return (float)rand() / (float)RAND_MAX;
}

/* main function */
int main(int argc, char **argv)
{
 /* Print doc if not enough parameters */
 if (argc < 5)
 {
  printf(
"Dummy.exe\n\
\n\
 This is an example program for use with parallel optimization. In order to\n\
 apply clop algorithms to your own problem, you should write a program that\n\
 behaves like this one.\n\
\n\
 Arguments are:\n\
  #1: processor id (symbolic name, typically a machine name to ssh to)\n\
  #2: seed (integer)\n\
  #3: parameter id of first parameter (symbolic name)\n\
  #4: value of first parameter (float)\n\
  #5: parameter id of second parameter (optional)\n\
  #6: value of second parameter (optional)\n\
  ...\n\
\n\
 This program should write the game outcome to its output:\n\
  W = win\n\
  L = loss\n\
  D = draw\n\
\n\
 For instance:\n\
  $ ./Dummy node-01 4 param 0.2\n\
  W\n\n\
Press ENTER to quit...");

 fflush(stdout);
 fgetc(stdin);

  return 1;
 }

 /* Parse parameter values */
 {
  int N = ((argc - 3) / 2);
  double *vParam = malloc(sizeof(double) * N);

  int i;
  for (i = 4; i < argc; i += 2)
  {
   double x;
   sscanf(argv[i], "%Lf", vParam + (i - 4) / 2);
  }

  /* Compute winning probability */
  {
   double d2 = 0.0;
   {
    int i;
    for (i = N; --i >= 0;)
    {
     double delta = vParam[i] - 0.3456789;
     d2 += 10 * delta * delta;
    }
   }
   {
    double p = 1.0 / (1.0 + exp(d2));

    /* Seed */
    {
     int seed;
     sscanf(argv[2], "%d", &seed);    
     srand(seed);
     rand();
     rand();
    }

    /* Sleep for a random amount of time */
    Sleep(rand() % 2000);

    /* Draw a random game outcome */
    if (rand_float() < p)
     printf("W\n");
    else
     printf("L\n");
   }
  }

  free(vParam);
 }

 return 0;
}
