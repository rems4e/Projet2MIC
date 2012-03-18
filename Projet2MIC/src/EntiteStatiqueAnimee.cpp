//
//  EntiteStatiqueAnimee.cpp
//  Projet2MIC
//
//  Created by Rémi Saurel on 11/03/12.
//  Copyright (c) 2012 Rémi Saurel. All rights reserved.
//

#include "EntiteStatiqueAnimee.h"
#include "tinyxml.h"

EntiteStatiqueAnimee::EntiteStatiqueAnimee(Niveau *n, uindex_t index, ElementNiveau::elementNiveau_t cat) : EntiteStatique(n, index, cat), _imageActuelle(0), _cadres(0), _tempsAffichage(0), _tempsPrecedent(0) {	
	TiXmlElement *e = ElementNiveau::description(index, cat);
	char const *img = e->Attribute("image");
	if(!img)
		throw ElementNiveau::Exc_DefinitionEntiteIncomplete();
	
	_image = Image(Session::cheminRessources() + img);
	
	e->Attribute("nbPoses", &_nbImages);
	// Position aléatoire, en cas de contiguïté, pour éviter d'avoir 50 entités qui bougent en même temps…
	_imageActuelle = nombreAleatoire(_nbImages);
	
	int temps;
	e->Attribute("tempsAttente", &temps);
	_tempsAffichage = temps / 1000.0;

	_cadres = new Rectangle[_nbImages];
	
	size_t largeur = _image.dimensionsReelles().x / _nbImages;
	size_t hauteur = _image.dimensionsReelles().y;
	int x = 0;
	for(int p = 0; p < _nbImages; ++p) {
		_cadres[p] = Rectangle(x, 0, largeur, hauteur);
		x += largeur;
	}
}

EntiteStatiqueAnimee::~EntiteStatiqueAnimee() {
	delete[] _cadres;
}

void EntiteStatiqueAnimee::afficher(Coordonnees const &decalage, double zoom) const {
	_image.redimensionner(zoom);
	Rectangle const &cadre = this->cadre();
	_image.afficher(this->positionAffichage() * zoom - decalage, cadre);
}

void EntiteStatiqueAnimee::animer(horloge_t tempsEcoule) {
	if(horloge() - _tempsPrecedent >= _tempsAffichage) {
		_tempsPrecedent = horloge();
		_imageActuelle = (_imageActuelle + 1) % _nbImages;
	}
}

Coordonnees EntiteStatiqueAnimee::dimensions() const {
	return Coordonnees(_image.dimensionsReelles().x / _nbImages, _image.dimensionsReelles().y);
}

Rectangle const &EntiteStatiqueAnimee::cadre() const {
	return _cadres[_imageActuelle];
}

