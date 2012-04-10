//
//  Teleporteur.cpp
//  Projet2MIC
//
//  Created by Rémi Saurel on 09/04/12.
//  Copyright (c) 2012 Rémi Saurel. All rights reserved.
//

#include "Teleporteur.h"

Teleporteur::Teleporteur(Niveau *n, uindex_t index, ElementNiveau::elementNiveau_t cat) : EntiteStatiqueAnimee(n, index, cat), _destination(Coordonnees::aucun), _actif(false) {
	
}

Teleporteur::~Teleporteur() {
	
}
