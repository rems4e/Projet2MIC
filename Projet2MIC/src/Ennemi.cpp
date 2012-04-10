//
//  Ennemi.cpp
//  Projet2MIC
//
//  Created by Rémi Saurel on 02/03/12.
//  Copyright (c) 2012 Rémi Saurel. All rights reserved.
//

#include "Ennemi.h"
#include "Joueur.h"

Ennemi::Ennemi(Niveau *n, uindex_t index, ElementNiveau::elementNiveau_t cat) : Personnage(n, index, cat, new InventaireEnnemi(*this, CAPACITE_ENNEMI)), _recherche(false) {
	
}

Ennemi::~Ennemi() {
	
}

void Ennemi::animer(horloge_t tempsEcoule) {
	Personnage::animer(tempsEcoule);
		
	Joueur *j = this->niveau()->joueur();
	if((j->position() - this->position()).norme() < this->porteeVision() * LARGEUR_CASE) {
		_cible = j->position();
		_recherche = true;

		bool suivre;
		Coordonnees dep = _cible - this->position();
		if(dep.vecteurNul()) {
			suivre = false;
		}
		else {
			dep.normaliser();
			dep *= this->vitesse();
			index_t pX = this->pX(this->position().x + dep.x), pY = this->pY(this->position().y + dep.y);
			suivre = j->pX() != pX || j->pY() != pY;
			Niveau::const_listeElements_t liste = this->niveau()->elements(pX, pY, this->couche());
			for(Niveau::elements_t::const_iterator el = liste.first; el != liste.second; ++el) {
				if(*el != this && (*el)->mobile() && static_cast<EntiteMobile *>(*el)->type() == EntiteMobile::ennemi) {
					Ennemi *e = static_cast<Ennemi *>(*el);
					
					if(e->_recherche) {
						suivre = false;
						break;
					}
				}
			}
		}
		if(suivre && this->definirAction(EntiteMobile::a_deplacer) && this->deplacerPosition(dep)) {

		}
		else {
			this->definirAction(EntiteMobile::a_immobile);
		}
	}
	else {
		this->definirAction(EntiteMobile::a_immobile);
		_cible = Coordonnees::aucun;
		_recherche = false;
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

EntiteMobile::categorie_t Ennemi::type() const {
	return EntiteMobile::ennemi;
}
