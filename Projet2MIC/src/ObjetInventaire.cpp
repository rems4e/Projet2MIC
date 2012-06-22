//
//  ObjetInvenaire.cpp
//  Projet2MIC
//
//  Créé par Marc Promé et Rémi Saurel.
//  Ce fichier et son contenu sont librement distribuables, modifiables et utilisables pour toute œuvre non commerciale, à condition d'en citer les auteurs.
//

#include "ObjetInventaire.h"
#include "Joueur.h"
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
	else if(typ == "cle") {
		_categorie = cle;
	}

	this->equilibrerAvecJoueur();
}

ObjetInventaire::~ObjetInventaire() {

}

void ObjetInventaire::afficher(index_t deltaY, Coordonnees const &decalage) const {
	this->image().redimensionner(Ecran::echelle());
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
		case cle:
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
	if(_categorie == potion) {
		for(Personnage::competences_t c = Personnage::premiereCompetence; c != Personnage::nbCompetences; ++c) {
			_competencesRequises[c] = 0;
		}
	}
	else {
		Personnage::Competences comp = this->niveau()->joueur()->competences();
		_competencesRequises[Personnage::force] = std::max<int>(1, comp[Personnage::force] * (1 + (nombreAleatoire(31) - 20) / 100.0));
		_competencesRequises[Personnage::endurance] = std::max<int>(1, comp[Personnage::endurance] * (1 + (nombreAleatoire(31) - 20) / 100.0));
		_competencesRequises[Personnage::agilite] = std::max<int>(1, comp[Personnage::agilite] * (1 + (nombreAleatoire(31) - 20) / 100.0));
	}

	switch(_categorie) {
		case arme:
			_attaque = _competencesRequises[Personnage::force] / 4;
			break;
		case bouclier:
			_defense = _competencesRequises[Personnage::agilite] / 8;
			break;
		case armure:
			_defense = _competencesRequises[Personnage::agilite] / 5;
			break;
		case bottes:
			_defense = _competencesRequises[Personnage::agilite] / 12;
			break;
		case gants:
			_defense = _competencesRequises[Personnage::agilite] / 12;
			break;
		case casque:
			_defense = _competencesRequises[Personnage::agilite] / 10;
			break;
		case potion: {
			_vie = this->niveau()->joueur()->vieTotale() / 2;
			_vie = std::max(1, _vie);
			_attaque = _defense = 0;
			break;
		}
		case cle: {
			_attaque = _defense = _vie = 0;
		}
	}
	
	if(this->categorieObjet() != potion && this->categorieObjet() != cle) {
		_attaque = std::max(1, _attaque);
		_defense = std::max(1, _defense);
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
		case cle:
			return "Cle";
	}
}

