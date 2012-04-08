//
//  EntiteStatique.cpp
//  Projet2MIC
//
//  Created by Rémi Saurel on 06/02/12.
//  Copyright (c) 2012 Rémi Saurel. All rights reserved.
//

#include "EntiteStatique.h"
#include "Ecran.h"
#include "tinyxml.h"

EntiteStatique::EntiteStatique(Niveau *n, uindex_t index, ElementNiveau::elementNiveau_t cat) : ElementNiveau(n, index, cat), _image() {
	TiXmlElement *e = ElementNiveau::description(index, cat);
	char const *img = e->Attribute("image");
	if(!img)
		throw ElementNiveau::Exc_DefinitionEntiteIncomplete();
	
	_image = Image(Session::cheminRessources() + img);
}

EntiteStatique::~EntiteStatique() {
	
}

void EntiteStatique::afficher(Coordonnees const &decalage, double zoom) const {
	_image.redimensionner(zoom);
	_image.afficher(this->positionAffichage() * zoom - decalage);
}

bool EntiteStatique::collision() const {
	return true;
}

void EntiteStatique::animer(horloge_t tempsEcoule) {
	
}

Coordonnees EntiteStatique::dimensions() const {
	return _image.dimensionsReelles();
}

Image const &EntiteStatique::image() const {
	return _image;
}


