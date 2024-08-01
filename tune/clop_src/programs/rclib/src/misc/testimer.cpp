#include <iostream>
#include "clktimer.h"
#include "CTimeIO.h"

int main(void)
{
 CClockTimer timer;
 CTime time = 0;

 while(time < 60000)
 {
  CTime timePrevious = time;
  time += timer.GetInterval();
  if (time != timePrevious)
  {
   std::cout << time << '\r';
   std::cout.flush();
  }
 }
 std::cout << time << '\n';

 return 0;
}
