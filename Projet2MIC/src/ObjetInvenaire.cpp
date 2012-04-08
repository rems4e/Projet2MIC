//
//  ObjetInvenaire.cpp
//  Projet2MIC
//
//  Created by Rémi Saurel on 08/04/12.
//  Copyright (c) 2012 Rémi Saurel. All rights reserved.
//

#include "ObjetInventaire.h"

ObjetInventaire::ObjetInventaire(Niveau *n, uindex_t index, ElementNiveau::elementNiveau_t cat) : EntiteStatique(n, index, cat) {	
	//TiXmlElement *e = ElementNiveau::description(index, cat);
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

