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

glm::vec2 const vec2Aucun(-std::numeric_limits<coordonnee_t>::max(), -std::numeric_limits<coordonnee_t>::max());
/*Coordonnees const Coordonnees::aucun(-std::numeric_limits<coordonnee_t>::max(), -std::numeric_limits<coordonnee_t>::max());
Coordonnees const Coordonnees::zero;
Coordonnees const Coordonnees::un(1.0, 1.0, 1.0);
Coordonnees const Coordonnees::eX(1.0, 0.0, 0.0);
Coordonnees const Coordonnees::eY(0.0, 1.0, 0.0);
Coordonnees const Coordonnees::eZ(0.0, 0.0, 1.0);

coordonnee_t const Coordonnees::coordMin = -std::numeric_limits<coordonnee_t>::max();
coordonnee_t const Coordonnees::coordMax = std::numeric_limits<coordonnee_t>::max();

dimension_t Coordonnees::norme() const {
	return std::sqrt(this->norme2());
}

bool Coordonnees::operator<(Rectangle const &r) const {
	return Rectangle(*this, Coordonnees()) < r;
}*/
