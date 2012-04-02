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
	index_t const pX = this->pX(), pY = this->pY();

	_cible = Coordonnees::aucun;
	for(index_t x = pX - this->porteeVision(); x != pX + this->porteeVision(); ++x) {
		for(index_t y = pY - this->porteeVision(); y != pY + this->porteeVision(); ++y) {
			ElementNiveau const *el = this->niveau()->element(x, y, this->couche());
			if(el && el->joueur()) {
				_cible = el->position();
				goto finRecherche;
			}
		}
	}
	
finRecherche:
	if(_cible != Coordonnees::aucun) {
		Coordonnees dep = _cible - this->position();
		if(this->definirAction(EntiteMobile::a_deplacer)) {
			dep.normaliser();
			dep *= this->vitesse();
			this->deplacerPosition(dep);
		}
	}
	else {
		this->definirAction(EntiteMobile::a_immobile);
	}
}

void Ennemi::interagir(Personnage *p) {
	
}

index_t Ennemi::porteeVision() const {
	return 8;
}

double Ennemi::vitesse() const {
	return Personnage::vitesse() * 0.75;
}

