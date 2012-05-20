//
//  Marchand.cpp
//  Projet2MIC
//
//  Created by Rémi Saurel on 18/04/12.
//  Copyright (c) 2012 Rémi Saurel. All rights reserved.
//

#include "Marchand.h"
#include "Session.h"

Marchand::Marchand(bool decoupagePerspective, Niveau *n, uindex_t index, ElementNiveau::elementNiveau_t cat) : Personnage(decoupagePerspective, n, index, cat, new InventaireMarchand(*this)) {
	for(int i = 0; i < 20; ++i) {
		ObjetInventaire *e = ElementNiveau::elementNiveau<ObjetInventaire>(false, this->niveau(), nombreAleatoire(6));
		this->inventaire()->ajouterObjet(e);
	}
}

Marchand::~Marchand() {
	
}

void Marchand::animer() {
	Personnage::animer();
}

bool Marchand::interagir(Personnage *p, bool test) {
	switch(p->categorieMobile()) {
		case EntiteMobile::em_joueur:
		case EntiteMobile::em_ennemi:
		case EntiteMobile::em_marchand:
			break;
	}
	
	return false;
}

void Marchand::jeterObjets() {
	
}

EntiteMobile::categorie_t Marchand::categorieMobile() const {
	return EntiteMobile::em_marchand;
}

bool Marchand::collision(index_t x, index_t y) const {
	return true;
}

ssize_t Marchand::prixAchat(ObjetInventaire *o) {
	return std::max<ssize_t>(this->prixVente(o) * 75 / 100, 1);
}

ssize_t Marchand::prixVente(ObjetInventaire *o) {	
	return 5 * (o->attaque() + o->defense() + o->vie());
}
