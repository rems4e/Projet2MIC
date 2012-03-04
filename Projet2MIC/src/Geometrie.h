/*
 *  Geometrie.h
 *  Jeu C++
 *
 *  Created by Rémi on 07/06/08.
 *  Copyright 2008 Rémi Saurel. All rights reserved.
 *
 */

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
