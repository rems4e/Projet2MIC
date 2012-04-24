//
//  Ennemi.cpp
//  Projet2MIC
//
//  Created by Rémi Saurel on 02/03/12.
//  Copyright (c) 2012 Rémi Saurel. All rights reserved.
//

#include "Ennemi.h"
#include "Joueur.h"

Ennemi::Ennemi(bool decoupagePerspective, Niveau *n, uindex_t index, ElementNiveau::elementNiveau_t cat) : Personnage(decoupagePerspective, n, index, cat, new InventaireEnnemi(*this, CAPACITE_ENNEMI)), _recherche(false) {
	this->definirVieTotale(10);
	this->inventaire()->modifierMonnaie(20);
}

Ennemi::~Ennemi() {
	
}

void Ennemi::animer() {
	this->Personnage::animer();
	if(this->mort()) {
		_recherche = false;
		return;
	}

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
			index_t pX = this->nPX(this->position().x + dep.x), pY = this->nPY(this->position().y + dep.y);
			suivre = j->pX() != pX || j->pY() != pY;
			Niveau::const_listeElements_t liste = this->niveau()->elements(pX, pY, this->couche());
			for(Niveau::elements_t::const_iterator el = liste.first; el != liste.second; ++el) {
				if(el->entite != this && el->entite->mobile() && static_cast<EntiteMobile *>(el->entite)->categorieMobile() == EntiteMobile::em_ennemi) {
					Ennemi *e = static_cast<Ennemi *>(el->entite);
					
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

bool Ennemi::interagir(Personnage *p) {
	switch(p->categorieMobile()) {
		case EntiteMobile::em_joueur:
			this->attaquer(p);
			return true;
		case EntiteMobile::em_ennemi:
		case EntiteMobile::em_marchand:
			break;
	}
	
	return false;
}

void Ennemi::jeterObjets() {
	InventaireEnnemi *e = static_cast<InventaireEnnemi *>(this->inventaire());
	std::list<ObjetInventaire *> liste;
	index_t pX = this->pX(), pY = this->pY();
	for(InventaireEnnemi::iterator i = e->debut(); i != e->fin(); ++i) {
		liste.push_back(*i);
		this->niveau()->ajouterElement(pX, pY, Niveau::cn_objetsInventaire, *i);
	}
	e->vider();	
}

index_t Ennemi::porteeVision() const {
	return 8;
}

double Ennemi::vitesse() const {
	return Personnage::vitesse() * 0.75;
}

EntiteMobile::categorie_t Ennemi::categorieMobile() const {
	return EntiteMobile::em_ennemi;
}
