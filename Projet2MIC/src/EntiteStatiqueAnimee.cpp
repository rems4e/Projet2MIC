//
//  EntiteStatiqueAnimee.cpp
//  Projet2MIC
//
//  Created by Rémi Saurel on 11/03/12.
//  Copyright (c) 2012 Rémi Saurel. All rights reserved.
//

#include "EntiteStatiqueAnimee.h"
#include "tinyxml.h"

EntiteStatiqueAnimee::EntiteStatiqueAnimee(bool decoupagePerspective, Niveau *n, uindex_t index, ElementNiveau::elementNiveau_t cat) : EntiteStatique(decoupagePerspective, n, index, cat), _imageActuelle(0), _cadres(0), _tempsAffichage(0), _tempsPrecedent(0) {
	TiXmlElement *e = ElementNiveau::description(index, cat);
	
	_nbImages = 1;
	if(e->Attribute("nbPoses"))
		e->Attribute("nbPoses", &_nbImages);
	// Position aléatoire, en cas de contiguïté, pour éviter d'avoir 50 entités qui bougent en même temps…
	_imageActuelle = nombreAleatoire(_nbImages);
	
	_tempsAffichage = 1;
	if(e->Attribute("tempsAttente")) {
		int temps;
		e->Attribute("tempsAttente", &temps);
		_tempsAffichage = temps / 1000.0f;
	}
	
	_cadres = new Rectangle[_nbImages];
	
	size_t largeur = this->image().dimensionsReelles().x / _nbImages;
	size_t hauteur = this->image().dimensionsReelles().y;
	index_t x = 0;
	for(int p = 0; p < _nbImages; ++p) {
		_cadres[p] = Rectangle(x, 0, largeur, hauteur);
		x += largeur;
	}
}

EntiteStatiqueAnimee::~EntiteStatiqueAnimee() {
	delete[] _cadres;
}

void EntiteStatiqueAnimee::afficher(index_t deltaY, Coordonnees const &decalage) const {
	Rectangle const &cadre = this->cadre();
	this->image().afficher(this->positionAffichage() - decalage, cadre);
}

void EntiteStatiqueAnimee::animer() {
	if(horloge() - _tempsPrecedent >= _tempsAffichage) {
		_tempsPrecedent = horloge();
		_imageActuelle = (_imageActuelle + 1) % _nbImages;
	}
}

Rectangle const &EntiteStatiqueAnimee::cadre() const {
	return _cadres[_imageActuelle];
}
