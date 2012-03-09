//
//  Personnage.cpp
//  Projet2MIC
//
//  Created by Rémi Saurel on 09/02/12.
//  Copyright (c) 2012 Rémi Saurel. All rights reserved.
//

#include "Personnage.h"
#include "Session.h"
#include "tinyxml.h"

Personnage::Personnage(Niveau *n, uindex_t index, ElementNiveau::elementNiveau_t cat) : EntiteMobile(n, index, cat), _vitesse(10) {
	TiXmlElement *e = ElementNiveau::description(index, cat);
	if(e->Attribute("vitesse"))
		e->Attribute("vitesse", &_vitesse);
}

void Personnage::animer(horloge_t tempsEcoule) {
	EntiteMobile::animer(tempsEcoule);
}

bool Personnage::grille() const {
	return false;
}

bool Personnage::centrage() const {
	return true;
}

Coordonnees Personnage::origine() const {
	return Coordonnees(1 * this->dimensions().x / 2, 23 * this->dimensions().y / 32);
}

double Personnage::vitesse() const {
	return _vitesse;
}
