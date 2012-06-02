//
//  Geometrie.h
//  Projet2MIC
//
//  Créé par Marc Promé et Rémi Saurel.
//  Ce fichier et son contenu sont librement distribuables, modifiables et utilisables pour toute œuvre non commerciale, à condition d'en citer les auteurs.
//


#ifndef EN_TETE_Geometrie
#define EN_TETE_Geometrie

#include <iostream>

#include "Coordonnees.h"
#include "Rectangle.h"

inline std::ostream &operator<<(std::ostream &s, Coordonnees const &c) {
	return s << "(" << c.x << ", " << c.y << ")";
}

inline std::ostream &operator<<(std::ostream &s, Rectangle const &r) {
	return s << "{(" << r.gauche << ", " << r.haut << "), (" << r.largeur << ", " << r.hauteur << ")}";
}


#endif
