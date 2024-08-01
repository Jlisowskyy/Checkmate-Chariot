#include <iostream>

#include "justtext.h"

int main(void)
{
 CJustifiedText jt(cout, 10);

 cout << "\n\nTest:\n01234567890123456789\n";
 jt << "x x x x x.";
 jt << " coucou, ceci est un petit test pour la sortie de texte justifié. ";
 jt << 123 << " " << 456 << "(" << 789 << " !)";

 return 0;
}
