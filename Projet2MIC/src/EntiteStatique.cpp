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
#include <cmath>

EntiteStatique::EntiteStatique(bool decoupagePerspective, Niveau *n, uindex_t index, ElementNiveau::elementNiveau_t cat) : ElementNiveau(decoupagePerspective, n, index, cat), _image(), _cadres(0), _nb(1) {
	TiXmlElement *e = ElementNiveau::description(index, cat);
	char const *img = e->Attribute("image");
	if(!img)
		throw ElementNiveau::Exc_DefinitionEntiteIncomplete();
	
	_image = Image(Session::cheminRessources() + img);
	
	if(this->decoupagePerspective()) {
		_nb = this->dimY();std::ceil((_image.dimensions().x - this->origine().x) / LARGEUR_CASE);
		_cadres = new Rectangle[_nb];
		_cadres[0] = Rectangle(0*Coordonnees(-this->origine().x, 0), Coordonnees(LARGEUR_CASE, _image.dimensions().y));
		for(index_t i = 1; i < _nb; ++i) {
			_cadres[i].definirDimensions(_cadres[0].dimensions());
			_cadres[i].definirOrigine(Coordonnees((_cadres[i - 1].origine() + _cadres[i - 1].dimensions()).x, 0));
			if(_cadres[i].origine().x + _cadres[i].largeur > _image.dimensions().x)
				_nb = i + 1;
		}
		_cadres[_nb - 1].largeur = _image.dimensions().x - _cadres[_nb - 1].gauche;
	}
}

EntiteStatique::~EntiteStatique() {
	delete[] _cadres;
}

void EntiteStatique::afficher(index_t deltaY, Coordonnees const &decalage) const {
	if(deltaY >= _nb)
		return;
	_image.afficher(this->positionAffichage() - decalage + Coordonnees(deltaY * 64, 0), this->decoupagePerspective() ? _cadres[deltaY] : Rectangle(Coordonnees::zero, _image.dimensionsReelles()));
}

void EntiteStatique::animer() {
	
}

Image const &EntiteStatique::image() const {
	return _image;
}
