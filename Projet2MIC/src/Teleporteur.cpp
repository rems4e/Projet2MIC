//
//  Teleporteur.cpp
//  Projet2MIC
//
//  Créé par Marc Promé et Rémi Saurel.
//  Ce fichier et son contenu sont librement distribuables, modifiables et utilisables pour toute œuvre non commerciale, à condition d'en citer les auteurs.
//

#include "Teleporteur.h"

Teleporteur::Teleporteur(bool decoupagePerspective, Niveau *n, uindex_t index, ElementNiveau::elementNiveau_t cat) : EntiteStatiqueAnimee(decoupagePerspective, n, index, cat), _destination(Coordonnees::aucun), _actif(false) {
	
}

Teleporteur::~Teleporteur() {
	
}
