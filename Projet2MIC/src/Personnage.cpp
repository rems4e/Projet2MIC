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
#include <cstring>
#include "Inventaire.h"
#include "ObjetInventaire.h"

int Personnage::Competences::operator[](competences_t c) const {
	return _valeurs[c];
}

int &Personnage::Competences::operator[](competences_t c) {
	return _valeurs[c];
}

bool Personnage::Competences::operator==(Competences const &c) const {
	for(competences_t i = premiereCompetence; i != nbCompetences; ++i) {
		if(_valeurs[i] != c._valeurs[i])
			return false;
	}
	
	return true;
}

bool Personnage::Competences::operator<(Competences const &c) const {
	for(competences_t i = premiereCompetence; i != nbCompetences; ++i) {
		if(_valeurs[i] >= c._valeurs[i])
			return false;
	}
	
	return true;
}

bool Personnage::Competences::operator<=(Competences const &c) const {
	return !(*this > c);
}

bool Personnage::Competences::operator>(Competences const &c) const {
	for(competences_t i = premiereCompetence; i != nbCompetences; ++i) {
		if(_valeurs[i] <= c._valeurs[i])
			return false;
	}
	
	return true;
}

bool Personnage::Competences::operator>=(Competences const &c) const {
	return !(*this < c);
}

Personnage::competences_t &operator++(Personnage::competences_t &p) {
	p = static_cast<Personnage::competences_t>(static_cast<int>(p) + 1);
	return p;
}

Personnage::positionTenue_t &operator++(Personnage::positionTenue_t &p) {
	p = static_cast<Personnage::positionTenue_t>(static_cast<int>(p) + 1);
	return p;
}

Personnage::Personnage(Niveau *n, uindex_t index, ElementNiveau::elementNiveau_t cat, Inventaire *inventaire) : EntiteMobile(n, index, cat), _vitesse(10), _vieActuelle(0), _vieTotale(200), _delaisAction(), _inventaire(inventaire), _competences() {
	TiXmlElement *e = ElementNiveau::description(index, cat);
	int t;
	if(e->Attribute("vitesse"))
		e->Attribute("vitesse", &_vitesse);
	if(e->Attribute("vie")) {
		e->Attribute("vie", &t);
		_vieTotale = t; 
	}
	_vieActuelle = _vieTotale / 2;
	
	
	for(EntiteMobile::action_t a = EntiteMobile::premiereAction; a != EntiteMobile::nbActions; ++a) {
		_delaisAction[a]._cdAbsolu = 0;
		_delaisAction[a]._cooldown = 0;
	}
	_delaisAction[a_attaquer]._cooldown = .5;
	
	std::memset(_tenue, 0, nbPositionsTenue * sizeof(ObjetInventaire *));
	
	for(competences_t i = premiereCompetence; i != nbCompetences; ++i) {
		_competences[i] = 1;
	}
}

Personnage::~Personnage() {
	delete _inventaire;
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
			_delaisAction[a]._cdAbsolu = horloge() + _delaisAction[a]._cooldown;
			return true;
		}
	}
	
	return false;
}

Niveau::couche_t Personnage::couche() const {
	return Niveau::cn_objet;
}

ObjetInventaire const *Personnage::tenue(positionTenue_t p) const {
	return _tenue[p];
}

ObjetInventaire *Personnage::tenue(positionTenue_t p) {
	return _tenue[p];
}

void Personnage::definirTenue(positionTenue_t p, ObjetInventaire *o) {
	_tenue[p] = o;
}

Inventaire const *Personnage::inventaire() const {
	return _inventaire;
}

Inventaire *Personnage::inventaire() {
	return _inventaire;
}

bool Personnage::peutEquiperObjet(ObjetInventaire *objet) {
	return this->competences() >= objet->competencesRequises();
}

bool Personnage::peutEquiperObjet(ObjetInventaire *objet, Personnage::positionTenue_t pos) {
	if(!this->peutEquiperObjet(objet) || !objet->tenue())
		return false;

	switch(pos) {
		case casque:
			return objet->categorieObjet() == ObjetInventaire::casque;
		case brasD:
		case brasG:
			return objet->categorieObjet() == ObjetInventaire::arme || objet->categorieObjet() == ObjetInventaire::bouclier;
		case armure:
			return objet->categorieObjet() == ObjetInventaire::armure;
		case gants:
			return objet->categorieObjet() == ObjetInventaire::gants;
		case bottes:
			return objet->categorieObjet() == ObjetInventaire::bottes;
		case nbPositionsTenue:
			return false;
	}
}

Personnage::Competences const &Personnage::competences() const {
	return _competences;
}
