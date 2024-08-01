////////////////////////////////////////////////////////////////////////////
//
// listtest.h
//
// Remi Coulom
//
// april 1996
//
// list template test
//
////////////////////////////////////////////////////////////////////////////
#include <iostream>

#include "list.h"       // CList<T> template
#include "listi.h"      // CListIterator<T> template
#include "clisti.h"     // CConstListIterator<T> template
#include "lsort.h"      // ListSort<T> template

////////////////////////////////////////////////////////////////////////////
// Comparison function
////////////////////////////////////////////////////////////////////////////
int Compare(const int &x, const int &y)
{
 return x - y;
}

////////////////////////////////////////////////////////////////////////////
// Manual Test
////////////////////////////////////////////////////////////////////////////
void ManualTest(void)
{
 CList<int>             li;
 CListIterator<int>     listi(li);

 do
 {
  cout << '>';
  char c;
  cin >> c;

  switch(c)
  {
   case '?': ///////////////////////////////////////////////////////////////
    cout << "? to display this help\n";
    cout << "q to quit\n";
    cout << "\n";
    cout << "a to add an integer to the list\n";
    cout << "r to remove the head from the list\n";
    cout << "d to display the list\n";
    cout << "\n";
    cout << "m remove with cursor\n";
    cout << "s reset cursor\n";
    cout << "i increment cursor\n";
    cout << "\n";
    cout << "n to display the size of the list\n";
    cout << "o to sort the list in ascending order\n";
    cout << '\n';
    cout << "t to test copy and append operation\n";
   break;

   case 'o': ///////////////////////////////////////////////////////////////
   {
    CListSort<int> lsi(li, Compare);
   }
   break;

   case 'n': ///////////////////////////////////////////////////////////////
    cout << li.GetSize() << '\n';
   break;

   case 'm': ///////////////////////////////////////////////////////////////
    listi.Remove();
   break;

   case 's': ///////////////////////////////////////////////////////////////
    listi.Reset();
   break;

   case 'i': ///////////////////////////////////////////////////////////////
    listi.Increment();
   break;

   case 'a': ///////////////////////////////////////////////////////////////
    li.Add();
    cin >> li.Head();
   break;

   case 'r': ///////////////////////////////////////////////////////////////
    li.Remove();
   break;

   case 'd': ///////////////////////////////////////////////////////////////
   {
    CConstListIterator<int> listiLocal(li);

    cout << "li = (";
    while(!listiLocal.IsAtTheEnd())
    {
     cout << listiLocal.Value();
     if (!listi.IsAtTheEnd() && &listiLocal.Value() == &listi.Value())
      cout << '*';
     else
      cout << ' ';
     listiLocal.Increment();
    }
    cout << ")\n";
   }
   break;

   case 't': ///////////////////////////////////////////////////////////////
   {
    CList<int> liCopie = li;
    CList<int> liCopie2;
    liCopie2 = liCopie;
    li.Append(liCopie);
    li.Append(liCopie2);
   }
   break;

   case 'q': ///////////////////////////////////////////////////////////////
    return;

   default: ////////////////////////////////////////////////////////////////
    cout << "Type ? for help\n";
   break;
  }
 }
 while(1);
}

////////////////////////////////////////////////////////////////////////////
// Main function
////////////////////////////////////////////////////////////////////////////
int main(void)
{
 ManualTest();

 return 0;
}
