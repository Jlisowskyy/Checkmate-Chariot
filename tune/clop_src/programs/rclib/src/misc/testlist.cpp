////////////////////////////////////////////////////////////////////////////
//
// testlist.cpp
//
// Rémi Coulom
//
// avril 1996
//
// test de la template CListe<T>
//
////////////////////////////////////////////////////////////////////////////
#include <iostream>
#include <ctime>

#include "list.h"       // Template pour la liste
#include "listi.h"      // CListIterator
#if 0
////////////////////////////////////////////////////////////////////////////
// Test des performances
////////////////////////////////////////////////////////////////////////////
void    TestPerformances(void)
{
 clock_t        Temps = clock();

 for (int TailleTest = 0; TailleTest < 5000; TailleTest += 100)
 {
  std::cout << TailleTest << " ";

  //
  // Création d'une liste des TailleTest premiers entiers
  //
  CListe<int>   l(100);
  {
   int  i;

   for (i = TailleTest; --i >= 0;)
   {
    l.Ajouter();
    l.Tete() = i;
   }
  }

  //
  // Bouclage sur toute la liste et conservation des multiples de 4
  //
  {
   CPositionListe<int>  pos(l);
   while (!pos.EstALaFin())
   {
    if (pos.Contenu() & 3)
     pos.Supprimer();
    else
     pos.Incrementer();
   }
  }

#if 0
  //
  // Affichage du contenu de la liste
  //
  {
   CPositionListe<int>  pos(l);
   while (!pos.EstALaFin())
   {
    std::cout << pos.Contenu() << " ";
    pos.Incrementer();
   }
  }
#endif

  //
  // Appel au destructeur
  //
 }

 //
 // Affichage du temps d'exécution
 //
 Temps = clock() - Temps;
 std::cout << "Temps d'exécution : " << Temps << "\n";
}
#endif

////////////////////////////////////////////////////////////////////////////
// Test de qualité
////////////////////////////////////////////////////////////////////////////
void    TestQualite(void)
{
 CList<int>             liste;
 CListIterator<int>    pos(liste);

 for(;;)
 {
  char  c;

  std::cin >> c;

  //
  // Exécution de la commande entrée au clavier
  //
  switch(c)
  {
   case 's': // Suppression d'un entier
    pos.Remove();
   break;

   case '+': // Déplacement vers l'avant
    pos.Increment();
   break;

   case '0': // Retour au départ
    pos.Reset();
   break;

   case 'a': // Insertion d'un entier
   {
    int v;

    std::cin >> v;
    pos.Insert();
    pos.Value() = v;
   }
   break;

   case 'i': // Insertion d'un entier
   {
    int v;

    std::cin >> v;
    liste.Add();
    liste.Head() = v;
    pos.Reset();
   }
   break;

   case 'q': // Sortie de la fonction
    return;

   default:
    std::cout << "Commande inconnue\n";
    std::cout << " a : ajout d'en entier au curseur\n";
    std::cout << " i : Insertion d'un entier en tete\n";
    std::cout << " s : Suppression d'un entier\n";
    std::cout << " + : Déplacement dans la liste\n";
    std::cout << " 0 : Positionnement en début de liste\n";
    std::cout << " q : Sortie du test\n";
  }

  //
  // Affichage de l'état courant
  //
  {
   CListIterator<int> posl(liste);

   while (!posl.IsAtTheEnd())
   {
    std::cout << posl.Value();

    if (&posl.Value() == &pos.Value())
     std::cout << "*";
    else
     std::cout << " ";

    posl.Increment();
   }

   std::cout << "\n";
  }
 }
}

////////////////////////////////////////////////////////////////////////////
// Programme principal
////////////////////////////////////////////////////////////////////////////
int main(void)
{
 TestQualite();

 return 0;
}
