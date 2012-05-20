//
//  ObjetInvenaire.cpp
//  Projet2MIC
//
//  Created by Rémi Saurel on 08/04/12.
//  Copyright (c) 2012 Rémi Saurel. All rights reserved.
//

#include "ObjetInventaire.h"
#include "tinyxml.h"

ObjetInventaire::ObjetInventaire(bool decoupagePerspective, Niveau *n, uindex_t index, ElementNiveau::elementNiveau_t cat) : EntiteStatique(decoupagePerspective, n, index, cat), _dimInventaire(1, 1), _competencesRequises(), _attaque(0), _defense(0), _vie(0) {
	TiXmlElement *e = ElementNiveau::description(index, cat);
	if(e->Attribute("iX"))
		e->Attribute("iX", &_dimInventaire.x);
	if(e->Attribute("iY"))
		e->Attribute("iY", &_dimInventaire.y);
	
	std::string typ = e->Attribute("type");
	if(typ == "arme") {
		_categorie = arme;
		_attaque = 1;
	}
	else if(typ == "armure") {
		_categorie = armure;
		_defense = 1;
	}
	else if(typ == "potion") {
		_categorie = potion;
		_vie = 1;
	}
	else if(typ == "bouclier") {
		_categorie = bouclier;
		_defense = 1;
	}
	else if(typ == "gants") {
		_categorie = gants;
		_defense = 1;
	}
	else if(typ == "bottes") {
		_categorie = bottes;
		_defense = 1;
	}
	else if(typ == "casque") {
		_categorie = casque;
		_defense = 1;
	}

	this->equilibrerAvecJoueur();
}

ObjetInventaire::~ObjetInventaire() {

}

void ObjetInventaire::afficher(index_t deltaY, Coordonnees const &decalage) const {
	this->image().redimensionner(Ecran::echelle().x, Ecran::echelle().y);
	this->image().afficher(this->positionAffichage() - decalage);
}

void ObjetInventaire::animer() {

}

Coordonnees ObjetInventaire::dimensions() const {
	return Coordonnees(1, 1);
}

Coordonnees ObjetInventaire::dimensionsInventaire() const {
	return _dimInventaire;
}

Personnage::Competences const &ObjetInventaire::competencesRequises() const {
	return _competencesRequises;
}

void ObjetInventaire::definirCompetencesRequises(Personnage::Competences const &c) {
	_competencesRequises = c;
}

ObjetInventaire::categorie_t ObjetInventaire::categorieObjet() const {
	return _categorie;
}

bool ObjetInventaire::tenue() const {
	switch(_categorie) {
		case arme:
		case bouclier:
		case armure:
		case bottes:
		case gants:
		case casque:
			return true;
		case potion:
			return false;
	}
}

int ObjetInventaire::defense() const {
	return _defense;
}

int ObjetInventaire::attaque() const {
	return _attaque;
}

int ObjetInventaire::vie() const {
	return _vie;
}

void ObjetInventaire::definirDefense(int d) {
	_defense = d;
}

void ObjetInventaire::definirAttaque(int a) {
	_attaque = a;
}

void ObjetInventaire::definirVie(int v) {
	_vie = v;
}

void ObjetInventaire::supprimerVie(int delta) {
	if(delta >= 0) {
		_vie = std::max(0, _vie - delta);
	}
}

void ObjetInventaire::equilibrerAvecJoueur() {
	_attaque = _defense = _vie = 0;
	switch(_categorie) {
		case arme:
			_attaque = std::max(1, nombreAleatoire(20));
			break;
		case bouclier:
			_defense = std::max(1, nombreAleatoire(15));
			break;
		case armure:
			_defense = std::max(1, nombreAleatoire(20));
			break;
		case bottes:
			_defense = std::max(1, nombreAleatoire(5));
			break;
		case gants:
			_defense = std::max(1, nombreAleatoire(5));
			break;
		case casque:
			_defense = std::max(1, nombreAleatoire(8));
			break;
		case potion:
			_vie = std::max(1, nombreAleatoire(1000));
	}
	
	if(_categorie == potion) {
		for(Personnage::competences_t c = Personnage::premiereCompetence; c != Personnage::nbCompetences; ++c) {
			_competencesRequises[c] = 0;
		}
	}
	else {
		_competencesRequises[Personnage::force] = 3 * _attaque + 1 * _defense;
		_competencesRequises[Personnage::endurance] = 3 * _defense + 1 * _attaque;
		_competencesRequises[Personnage::endurance] = 2 * _defense;

		for(Personnage::competences_t c = Personnage::premiereCompetence; c != Personnage::nbCompetences; ++c) {
			_competencesRequises[c] += (nombreAleatoire(60) - 30) * _competencesRequises[c] / 100;
			_competencesRequises[c] = std::max(1, _competencesRequises[c]);
		}
	}
}

char const *ObjetInventaire::nomCategorieObjet() const {
	switch(_categorie) {
		case arme:
			return "Arme";
		case bouclier:
			return "Bouclier";
		case armure:
			return "Armure";
		case bottes:
			return "Bottes";
		case gants:
			return "Gants";
		case casque:
			return "Casque";
		case potion:
			return "Potion";
	}
}

