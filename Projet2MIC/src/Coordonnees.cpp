/*
 *  Coordonnees.cpp
 *  Jeu C++
 *
 *  Created by Rémi on 28/07/11.
 *  Copyright 2011 Rémi Saurel. All rights reserved.
 *
 */

#include "Coordonnees.h"
#include "Rectangle.h"
#include <limits>
#include <cmath>

Coordonnees const Coordonnees::aucun = Coordonnees(-std::numeric_limits<coordonnee_t>::max(), -std::numeric_limits<coordonnee_t>::max());
Coordonnees const Coordonnees::zero = Coordonnees();

coordonnee_t const Coordonnees::coordMin = -std::numeric_limits<coordonnee_t>::max();
coordonnee_t const Coordonnees::coordMax = std::numeric_limits<coordonnee_t>::max();

dimension_t Coordonnees::norme() const {
	return std::sqrt(x * x + y * y);
}

Coordonnees::Coordonnees(Rectangle const &r) : x(r.gauche), y(r.haut) { }
Coordonnees::Coordonnees(Rectangle const &r, int) : x(r.largeur), y(r.hauteur) { }


bool Coordonnees::operator<(Rectangle const &r) const {
	return Rectangle(*this, Coordonnees()) < r;
}
