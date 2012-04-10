//
//  ObjetInvenaire.cpp
//  Projet2MIC
//
//  Created by Rémi Saurel on 08/04/12.
//  Copyright (c) 2012 Rémi Saurel. All rights reserved.
//

#include "ObjetInventaire.h"
#include "tinyxml.h"

ObjetInventaire::ObjetInventaire(Niveau *n, uindex_t index, ElementNiveau::elementNiveau_t cat) : EntiteStatique(n, index, cat), _dimInventaire(1, 1), _competencesRequises() {	
	TiXmlElement *e = ElementNiveau::description(index, cat);
	if(e->Attribute("iX"))
		e->Attribute("iX", &_dimInventaire.x);
	if(e->Attribute("iY"))
		e->Attribute("iY", &_dimInventaire.y);
	
	std::string typ = e->Attribute("type");
	if(typ == "arme")
		_categorie = arme;
	else if(typ == "armure")
		_categorie = armure;
	else if(typ == "potion")
		_categorie = potion;
	else if(typ == "bouclier")
		_categorie = bouclier;
	else if(typ == "gants")
		_categorie = gants;
	else if(typ == "bottes")
		_categorie = bottes;
	else if(typ == "casque")
		_categorie = casque;
	
	for(Personnage::competences_t i = Personnage::premiereCompetence; i != Personnage::nbCompetences; ++i) {
		_competencesRequises[i] = 1;
	}
}

ObjetInventaire::~ObjetInventaire() {

}

void ObjetInventaire::afficher(Coordonnees const &decalage, double zoom) const {
	this->image().redimensionner(zoom);
	this->image().afficher(this->positionAffichage() * zoom - decalage);
}

void ObjetInventaire::animer(horloge_t tempsEcoule) {

}

Coordonnees ObjetInventaire::dimensions() const {
	return this->image().dimensionsReelles();
}

Coordonnees ObjetInventaire::dimensionsInventaire() const {
	return _dimInventaire;
}

Personnage::Competences const &ObjetInventaire::competencesRequises() const {
	return _competencesRequises;
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
