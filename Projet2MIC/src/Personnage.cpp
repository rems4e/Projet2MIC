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

Personnage::Personnage(Niveau *n, uindex_t index, ElementNiveau::elementNiveau_t cat) : EntiteMobile(n, index, cat), _vitesse(10), _vieActuelle(0), _vieTotale(200), _delaisAction() {
	TiXmlElement *e = ElementNiveau::description(index, cat);
	int t;
	if(e->Attribute("vitesse"))
		e->Attribute("vitesse", &_vitesse);
	if(e->Attribute("vie")) {
		e->Attribute("vie", &t);
		_vieTotale = t; 
	}
	_vieActuelle = _vieTotale;		
	
	
	for(EntiteMobile::action_t a = EntiteMobile::premiereAction; a != EntiteMobile::nbActions; ++a) {
		_delaisAction[a]._cdAbsolu = 0;
		_delaisAction[a]._cooldown = 0;
	}
	_delaisAction[a_attaquer]._cooldown = 3    ;
}

void Personnage::animer(horloge_t tempsEcoule) {
	EntiteMobile::animer(tempsEcoule);
}

bool Personnage::centrage() const {
	return true;
}

Coordonnees Personnage::origine() const {
	return Coordonnees(this->cadre().largeur / 2, 3 * this->cadre().hauteur / 4);
}

double Personnage::vitesse() const {
	return _vitesse;
}

unsigned int Personnage::vieActuelle() const {
	return _vieActuelle;
}

unsigned int Personnage::vieTotale() const {
	return _vieTotale;
}

bool Personnage::definirAction(action_t a) {
	if(horloge() > _delaisAction[a]._cdAbsolu) {
		if(EntiteMobile::definirAction(a)) {			
			_delaisAction[a]._cdAbsolu = horloge()+_delaisAction[a]._cooldown;
			return true;
		}
	}
	
	return false;
}
