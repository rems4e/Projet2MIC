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
#include "Niveau.h"

EntiteStatique::EntiteStatique(bool decoupagePerspective, Niveau *n, uindex_t index, ElementNiveau::elementNiveau_t cat) : ElementNiveau(decoupagePerspective, n, index, cat), _image(), _cadres(0) {
	TiXmlElement *e = ElementNiveau::description(index, cat);
	char const *img = e->Attribute("image");
	if(!img)
		throw ElementNiveau::Exc_DefinitionEntiteIncomplete();
	
	_image = Image(Session::cheminRessources() + img);
	
	if(this->decoupagePerspective()) {
		size_t nb = this->dimY();
		_cadres = new Rectangle[nb];
		_cadres[0] = Rectangle(Coordonnees::zero, Coordonnees(LARGEUR_CASE, _image.dimensions().y));
		for(index_t i = 1; i < nb; ++i) {
			_cadres[i].definirDimensions(_cadres[0].dimensions());
			_cadres[i].definirOrigine(Coordonnees((_cadres[i - 1].origine() + _cadres[i - 1].dimensions()).x, 0));
		}
		_cadres[nb - 1].largeur = _image.dimensions().x - _cadres[nb - 1].gauche;
	}
}

EntiteStatique::~EntiteStatique() {
	delete[] _cadres;
}

void EntiteStatique::afficher(index_t deltaX, index_t deltaY, Coordonnees const &decalage, double zoom) const {
	_image.redimensionner(zoom);
	_image.afficher(this->positionAffichage() * zoom - decalage + Coordonnees(deltaY * 64, 0), this->decoupagePerspective() ? _cadres[deltaY] : Rectangle(Coordonnees::zero, _image.dimensionsReelles()));
	
}

void EntiteStatique::animer(horloge_t tempsEcoule) {
	
}

Image const &EntiteStatique::image() const {
	return _image;
}
