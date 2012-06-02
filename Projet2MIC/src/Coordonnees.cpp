//
//  Coordonnees.cpp
//  Projet2MIC
//
//  Créé par Marc Promé et Rémi Saurel.
//  Ce fichier et son contenu sont librement distribuables, modifiables et utilisables pour toute œuvre non commerciale, à condition d'en citer les auteurs.
//

#include "Coordonnees.h"
#include "Rectangle.h"
#include <limits>
#include <cmath>

Coordonnees const Coordonnees::aucun = Coordonnees(-std::numeric_limits<coordonnee_t>::max(), -std::numeric_limits<coordonnee_t>::max());
Coordonnees const Coordonnees::zero = Coordonnees();
Coordonnees const Coordonnees::un = Coordonnees(1.0, 1.0);
Coordonnees const Coordonnees::eX = Coordonnees(1.0, 0.0);
Coordonnees const Coordonnees::eY = Coordonnees(0.0, 1.0);

coordonnee_t const Coordonnees::coordMin = -std::numeric_limits<coordonnee_t>::max();
coordonnee_t const Coordonnees::coordMax = std::numeric_limits<coordonnee_t>::max();

dimension_t Coordonnees::norme() const {
	return std::sqrt(x * x + y * y);
}

bool Coordonnees::operator<(Rectangle const &r) const {
	return Rectangle(*this, Coordonnees()) < r;
}
