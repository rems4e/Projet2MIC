//
//  ElementNiveau.cpp
//  Projet2MIC
//
//  Created by Rémi Saurel on 03/02/12.
//  Copyright (c) 2012 Rémi Saurel. All rights reserved.
//

#include "ElementNiveau.h"
#include "EntiteStatique.h"
#include "EntiteMobile.h"
#include "EntiteStatiqueAnimee.h"
#include "Teleporteur.h"
#include "Ennemi.h"
#include "Joueur.h"
#include "ObjetInventaire.h"
#include "Niveau.h"
#include "tinyxml.h"
#include <cmath>
#include "UtilitaireNiveau.h"
#include <cstring>

TiXmlDocument *ElementNiveau::_description = 0;

ElementNiveau::elementNiveau_t &operator++(ElementNiveau::elementNiveau_t &e) { return e = static_cast<ElementNiveau::elementNiveau_t>(static_cast<int>(e) + 1); }
ElementNiveau::elementNiveau_t operator+(ElementNiveau::elementNiveau_t e, int i) { return static_cast<ElementNiveau::elementNiveau_t>(static_cast<int>(e) + 1); }

ElementNiveau *ElementNiveau::elementNiveau(Niveau *n, uindex_t index, elementNiveau_t cat) throw(ElementNiveau::Exc_EntiteIndefinie, ElementNiveau::Exc_DefinitionEntiteIncomplete) {
	switch(cat) {
		case entiteStatique:
			return ElementNiveau::elementNiveau<EntiteStatique>(n, index);
		case ennemi:
			return ElementNiveau::elementNiveau<Ennemi>(n, index);
		case arbre:
			return ElementNiveau::elementNiveau<EntiteStatique>(n, index, arbre);
		case entiteStatiqueAnimee:
			return ElementNiveau::elementNiveau<EntiteStatiqueAnimee>(n, index);
		case objetInventaire:
			return ElementNiveau::elementNiveau<ObjetInventaire>(n, index);
		case teleporteur:
			return ElementNiveau::elementNiveau<Teleporteur>(n, index);
		case maison:
			return ElementNiveau::elementNiveau<EntiteStatique>(n, index, maison);
		case ndef8:
		case ndef9:case ndef10:case ndef11:case ndef12:case ndef13:case ndef14:case ndef15:case ndef16:
		case ndef17:case ndef18:case ndef19:case ndef20:case ndef21:case ndef22:case ndef23:case ndef24:
		case ndef25:case ndef26:case ndef27:case ndef28:case ndef29:case ndef30:case ndef31:case ndef32:
		case ndef33:case ndef34:case ndef35:case ndef36:case ndef37:case ndef38:case ndef39:case ndef40:
		case ndef41:case ndef42:case ndef43:case ndef44:case ndef45:case ndef46:case ndef47:case ndef48:
		case ndef49:case ndef50:case ndef51:case ndef52:case ndef53:case ndef54:case ndef55:case ndef56:
		case ndef57:case ndef58:case ndef59:case ndef60:case ndef61:case ndef62:case ndef63:case ndef64:
		case nbTypesElement:
			throw Exc_EntiteIndefinie();
	}
}

ElementNiveau::ElementNiveau(Niveau *n, uindex_t index, elementNiveau_t cat) throw(ElementNiveau::Exc_DefinitionEntiteIncomplete) : _niveau(n), _position(), _origine(), _centrage(false), _categorie(cat), _dimX(1), _dimY(1) {
	TiXmlElement *e = ElementNiveau::description(index, cat);
	if(e->Attribute("x"))
		e->Attribute("x", &_origine.x);
	if(e->Attribute("y"))
		e->Attribute("y", &_origine.y);
	
	if(e->Attribute("dimX"))
		e->Attribute("dimX", &_dimX);
	if(e->Attribute("dimY"))
		e->Attribute("dimY", &_dimY);
	
	int cc;
	if(e->Attribute("centrage")) {
		e->Attribute("centrage", &cc);
		_centrage = cc;
	}
	
	_collision = new bool*[_dimY];
	for(index_t y = 0; y < _dimY; ++y) {
		_collision[y] = new bool[_dimX];
		for(index_t x = 0; x < _dimX; ++x) {
			_collision[y][x] = true;
		}
	}
	char const *texte = e->GetText();
	if(texte) {
		size_t tailleAttendue = _dimY * (_dimX + 1) - 1;
		if(std::strlen(texte) != tailleAttendue) {
			std::cerr << "Erreur : dimensions de la couche de collision invalides (" << nombreVersTexte(std::strlen(texte)) << " au lieu de " << nombreVersTexte(tailleAttendue) <<  " attendus) !" << std::endl;
			throw Exc_DefinitionEntiteIncomplete();
		}
		for(index_t y = 0; y < _dimY; ++y) {
			for(index_t x = 0; x < _dimX; ++x, ++texte) {
				switch(*texte) {
					case '0':
						_collision[y][x] = false;
						break;
					case '1':
						break;
					default:
						std::cerr << "Erreur : couche de collision invalide." << std::endl;
						throw Exc_DefinitionEntiteIncomplete();
				}
			}
			++texte;
		}
	}
}

ElementNiveau::~ElementNiveau() {
	for(index_t y = 0; y < _dimY; ++y) {
		delete[] _collision[y];
	}
	
	delete[] _collision;
}

Coordonnees ElementNiveau::position() const {
	return _position;
}

void ElementNiveau::definirPosition(Coordonnees const &p) {
	_position = p;
}

Coordonnees ElementNiveau::positionAffichage() const {
	return referentielNiveauVersEcran(this->position()) - this->origine() + this->centrage() * Coordonnees(LARGEUR_CASE, 0) / 2;
}

Niveau *ElementNiveau::niveau() {
	return _niveau;
}

void ElementNiveau::definirNiveau(Niveau *n) {
	_niveau = n;
}

Coordonnees ElementNiveau::origine() const {
	return _origine;
}

bool ElementNiveau::centrage() const {
	return _centrage;
}

ElementNiveau::elementNiveau_t ElementNiveau::categorie() const {
	return _categorie;
}


void ElementNiveau::chargerDescription() {
	if(!_description) {
		_description = new TiXmlDocument(Session::cheminRessources() + "ElementsNiveaux.xml");
		if(!_description->LoadFile())
			std::cerr << "Erreur de l'ouverture du fichier de description d'entités (" << (Session::cheminRessources() + "ElementsNiveaux.xml") << "." << std::endl;
	}
}

TiXmlElement *ElementNiveau::description(uindex_t index, elementNiveau_t cc) {
	ElementNiveau::chargerDescription();
	
	TiXmlElement *element = _description->FirstChildElement("ElementsNiveau");
	if(!element) {
		std::cout << "Balise \"ElementsNiveau\" indisponible." << std::endl;
		return 0;
	}
	
	std::string const cat(ElementNiveau::nomBalise(cc));
	element = element->FirstChildElement(cat);
	if(!element) {
		std::cout << "Catégorie \"" << cat << "\" indisponible." << std::endl;
		return 0;
	}
	std::string balise("Element");
	balise += nombreVersTexte(index);
	element = element->FirstChildElement(balise);
		
	return element;
}

size_t ElementNiveau::nombreEntites(elementNiveau_t categorie) {
	static size_t nb[ElementNiveau::nbTypesElement] = {-1};
	if(nb[0] == -1) {
		ElementNiveau::chargerDescription();
		TiXmlElement *element = _description->FirstChildElement("ElementsNiveau");
		for(ElementNiveau::elementNiveau_t i = ElementNiveau::premierTypeElement; i != ElementNiveau::nbTypesElement; ++i) {
			nb[i] = 0;
			char const *nom = ElementNiveau::nomBalise(i);
			if(nom) {
				TiXmlElement *cat = element->FirstChildElement(nom);
				if(cat)
					for(TiXmlNode *n = cat->FirstChild(); n; n = n->NextSibling(), ++nb[i]);
			}
		}
	}
	
	return nb[categorie];
}

bool ElementNiveau::mobile() const {
	return false;
}

bool ElementNiveau::joueur() const {
	return false;
}

index_t ElementNiveau::pX() const {
	return std::floor(this->position().x / LARGEUR_CASE);
}

index_t ElementNiveau::pY() const {
	return std::floor(this->position().y / LARGEUR_CASE);
}

bool ElementNiveau::collision(index_t x, index_t y) const {
	return _collision[y][x];
}

size_t ElementNiveau::dimX() const {
	return _dimX;
}

size_t ElementNiveau::dimY() const {
	return _dimY;
}

char const *ElementNiveau::nomBalise(elementNiveau_t cat) {
	switch(cat) {
		case entiteStatique:
			return "EntiteStatique";
		case ennemi:
			return "Personnage";
		case arbre:
			return "Arbre";
		case entiteStatiqueAnimee:
			return "EntiteStatiqueAnimee";
		case objetInventaire:
			return "ObjetInventaire";
		case teleporteur:
			return "Teleporteur";
		case maison:
			return "Maison";
		case ndef8:
		case ndef9:case ndef10:case ndef11:case ndef12:case ndef13:case ndef14:case ndef15:case ndef16:
		case ndef17:case ndef18:case ndef19:case ndef20:case ndef21:case ndef22:case ndef23:case ndef24:
		case ndef25:case ndef26:case ndef27:case ndef28:case ndef29:case ndef30:case ndef31:case ndef32:
		case ndef33:case ndef34:case ndef35:case ndef36:case ndef37:case ndef38:case ndef39:case ndef40:
		case ndef41:case ndef42:case ndef43:case ndef44:case ndef45:case ndef46:case ndef47:case ndef48:
		case ndef49:case ndef50:case ndef51:case ndef52:case ndef53:case ndef54:case ndef55:case ndef56:
		case ndef57:case ndef58:case ndef59:case ndef60:case ndef61:case ndef62:case ndef63:case ndef64:
		case nbTypesElement:
			return 0;
	}
}

char const *ElementNiveau::nomCategorie(elementNiveau_t cat) {
	switch(cat) {
		case entiteStatique:
			return "Entité statique";
		case ennemi:
			return "Ennemi";
		case arbre:
			return "Arbre";
		case entiteStatiqueAnimee:
			return "Entite statique animée";
		case objetInventaire:
			return "Objet inventaire";
		case teleporteur:
			return "Téléporteur";
		case maison:
			return "Maison";
		case ndef8:
		case ndef9:case ndef10:case ndef11:case ndef12:case ndef13:case ndef14:case ndef15:case ndef16:
		case ndef17:case ndef18:case ndef19:case ndef20:case ndef21:case ndef22:case ndef23:case ndef24:
		case ndef25:case ndef26:case ndef27:case ndef28:case ndef29:case ndef30:case ndef31:case ndef32:
		case ndef33:case ndef34:case ndef35:case ndef36:case ndef37:case ndef38:case ndef39:case ndef40:
		case ndef41:case ndef42:case ndef43:case ndef44:case ndef45:case ndef46:case ndef47:case ndef48:
		case ndef49:case ndef50:case ndef51:case ndef52:case ndef53:case ndef54:case ndef55:case ndef56:
		case ndef57:case ndef58:case ndef59:case ndef60:case ndef61:case ndef62:case ndef63:case ndef64:
		case nbTypesElement:
			return 0;
	}
}

