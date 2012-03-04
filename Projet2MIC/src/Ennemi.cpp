//
//  Ennemi.cpp
//  Projet2MIC
//
//  Created by Rémi Saurel on 02/03/12.
//  Copyright (c) 2012 Rémi Saurel. All rights reserved.
//

#include "Ennemi.h"

Ennemi::Ennemi(Niveau *n, uindex_t index, ElementNiveau::elementNiveau_t cat) : Personnage(n, index, cat) {
	
}

Ennemi::~Ennemi() {
	
}

void Ennemi::animer(horloge_t tempsEcoule) {
	Personnage::animer(tempsEcoule);
	
}

void Ennemi::interagir(Personnage *p) {
	
}
