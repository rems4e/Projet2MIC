//
//  EntiteMobile.cpp
//  Projet2MIC
//
//  Created by Rémi Saurel on 06/02/12.
//  Copyright (c) 2012 Rémi Saurel. All rights reserved.
//

#include "EntiteMobile.h"
#include <string>
#include <cstring>
#include "tinyxml.h"
#include "Niveau.h"

EntiteMobile::action_t &operator++(EntiteMobile::action_t &c) { return c = static_cast<EntiteMobile::action_t>(static_cast<int>(c + 1)); }

EntiteMobile::EntiteMobile(Niveau *n, uindex_t index, ElementNiveau::elementNiveau_t cat) : ElementNiveau(n, index, cat), _tempsPrecedent(0), _image(), _cadres(), _tempsAffichage(), _nbImages(), _imageActuelle(0), _action(a_immobile), _direction(gauche) {
	std::memset(_nbImages, 0, nbActions * sizeof(size_t));
	for(action_t a = premiereAction; a != nbActions; ++a) {
		std::memset(_cadres[a], 0, 8 * sizeof(Rectangle *));
	}
	
	TiXmlElement *e = ElementNiveau::description(index, cat);
	char const *img = e->Attribute("image");
	if(!img)
		throw ElementNiveau::Exc_DefinitionEntiteIncomplete();
	
	_image = Image(Session::cheminRessources() + img);
	
	size_t dimX = 128, dimY = 128;
	if(e->Attribute("dimX"))
		e->Attribute("dimX", &dimX);
	if(e->Attribute("dimY"))
		e->Attribute("dimY", &dimY);
	
	for(action_t a = premiereAction; a != nbActions; ++a) {
		TiXmlElement *action = e->FirstChildElement(EntiteMobile::transcriptionAction(a));
		if(action) {
			action->Attribute("nbPoses", &_nbImages[a]);
			int temps;
			action->Attribute("tempsAttente", &temps);
			_tempsAffichage[a] = temps / 1000.0;
		}
	}
	
	for(int direction = 0; direction < 8; ++direction) {
		int x = 0;
		for(action_t a = premiereAction; a != nbActions; ++a) {
			_cadres[a][direction] = new Rectangle[_nbImages[a]];
			for(int p = 0; p < _nbImages[a]; ++p) {
				_cadres[a][direction][p] = Rectangle(x, direction * dimY, dimX, dimY);
				x += dimX;
			}
		}
	}
	
	this->definirAction(EntiteMobile::a_immobile);
}

EntiteMobile::~EntiteMobile() {
	for(action_t a = premiereAction; a != nbActions; ++a) {
		for(int i = 0; i < 8; ++i) {
			delete[] _cadres[a][i];
		}
	}
}

void EntiteMobile::afficher(Coordonnees const &decalage, double zoom) const {
	_image.redimensionner(zoom);
	Rectangle const &cadre = this->cadre();
	_image.afficher(this->positionAffichage() * zoom - decalage, cadre);
}

bool EntiteMobile::collision() const {
	return true;
}

void EntiteMobile::animer(horloge_t tempsEcoule) {
	if(horloge() - _tempsPrecedent >= _tempsAffichage[_action]) {
		_tempsPrecedent = horloge();
		_imageActuelle = (_imageActuelle + 1) % _nbImages[_action];
	}
}

Coordonnees EntiteMobile::dimensions() const {
	return Coordonnees(LARGEUR_CASE, LARGEUR_CASE);
}

Rectangle const &EntiteMobile::cadre() const {
	return _cadres[_action][_direction][_imageActuelle];
}

bool EntiteMobile::actionDisponible(action_t a) const {
	return _nbImages[a];
}
					
char const *EntiteMobile::transcriptionAction(action_t a) {
	switch(a) {
		case a_immobile:
			return "Immobile";
		case a_deplacer:
			return "Deplacer";
		case a_attaquer:
			return "Attaque";
		case a_mourir:
			return "Mourir";
		case nbActions:
			return "nbActions";
	}
}

bool EntiteMobile::actionInterruptible(action_t a) {
	switch(a) {
		case a_immobile:
			return true;
		case a_deplacer:
			return true;
		case a_attaquer:
			return false;
		case a_mourir:
			return false;
		case nbActions:
			return false;
	}
}


void EntiteMobile::definirDirection(direction_t d) {
	_direction = d;
}

bool EntiteMobile::definirAction(action_t a) {
	if(this->actionDisponible(a)) {
		if(EntiteMobile::actionInterruptible(_action) || _imageActuelle == _nbImages[_action] - 1) {
			if(_action != a) {
				_action = a;
				_imageActuelle = 0;
			}
			
			return true;
		}
	}
	
	return false;
}

void EntiteMobile::deplacerPosition(Coordonnees const &dep) {
	if(dep.vecteurNul())
		return;

	direction_t dir = EntiteMobile::gauche;
	if(dep.x < 0) {
		if(dep.y < 0)
			dir = EntiteMobile::gauche;
		else if(dep.y == 0)
			dir = EntiteMobile::gaucheBas;
		else if(dep.y > 0)
			dir = EntiteMobile::bas;
	}
	else if(dep.x == 0) {
		if(dep.y < 0)
			dir = EntiteMobile::gaucheHaut;
		else if(dep.y > 0)
			dir = EntiteMobile::droiteBas;
	}
	else if(dep.x > 0) {
		if(dep.y < 0)
			dir = EntiteMobile::haut;
		else if(dep.y == 0)
			dir = EntiteMobile::droiteHaut;
		else if(dep.y > 0)
			dir = EntiteMobile::droite;
	}
	
	this->definirDirection(dir);
	
	ElementNiveau::deplacerPosition(dep);
}

EntiteMobile::action_t EntiteMobile::action() const {
	return _action;
}

EntiteMobile::direction_t EntiteMobile::direction() const {
	return _direction;
}




