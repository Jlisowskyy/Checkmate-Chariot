/////////////////////////////////////////////////////////////////////////////
//
// Dummy.cpp
//
/////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <sstream>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <boost/thread/thread.hpp>

/////////////////////////////////////////////////////////////////////////////
// random float between 0 and 1
/////////////////////////////////////////////////////////////////////////////
static float rand_float()
{
 return (float)std::rand() / (float)RAND_MAX;
}

/////////////////////////////////////////////////////////////////////////////
// main function
/////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
 //
 // Print doc if not enough parameters
 //
 if (argc < 5)
 {
  std::cerr <<
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
";
  std::cout << "Press ENTER to quit...";
  std::cin.ignore( std::numeric_limits <std::streamsize> ::max(), '\n' );

  return 1;
 }

 //
 // Parse parameter values
 //
 std::vector<double> vParam;
 for (int i = 4; i < argc; i += 2)
 {
  double x;
  std::istringstream(argv[i]) >> x;
  vParam.push_back(x);
 }

 //
 // Compute winning probability
 //
 double d2 = 0.0;
 for (int i = vParam.size(); --i >= 0;)
 {
  double delta = vParam[i] - 0.3456789;
  d2 += 10 * delta * delta;
 }
 double p = 1.0 / (1.0 + std::exp(d2));

 //
 // Sleep for a random amount of time
 //
 int seed = 0;
 std::istringstream(argv[2]) >> seed;
 std::srand(seed);
 for (int i = 5; --i >= 0;)
  rand_float(); // improves quality of randomness
 boost::this_thread::sleep(boost::posix_time::milliseconds(rand_float() * 2000));

 //
 // Draw a random game outcome
 //
 if (rand_float() < p)
  std::cout << "W\n";
 else
  std::cout << "L\n";

 return 0;
}
