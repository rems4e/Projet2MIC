//
//  Personnage.cpp
//  Projet2MIC
//
//  Created by Rémi Saurel on 09/02/12.
//  Copyright (c) 2012 Rémi Saurel. All rights reserved.
//

#include "Personnage.h"
#include "Session.h"
#include "tinyxml.h"
#include <cstring>
#include "Inventaire.h"
#include "ObjetInventaire.h"

int Personnage::Competences::operator[](competences_t c) const {
	return _valeurs[c];
}

int &Personnage::Competences::operator[](competences_t c) {
	return _valeurs[c];
}

bool Personnage::Competences::operator==(Competences const &c) const {
	for(competences_t i = premiereCompetence; i != nbCompetences; ++i) {
		if(_valeurs[i] != c._valeurs[i])
			return false;
	}
	
	return true;
}

bool Personnage::Competences::operator<(Competences const &c) const {
	for(competences_t i = premiereCompetence; i != nbCompetences; ++i) {
		if(_valeurs[i] >= c._valeurs[i])
			return false;
	}
	
	return true;
}

bool Personnage::Competences::operator<=(Competences const &c) const {
	return !(*this > c);
}

bool Personnage::Competences::operator>(Competences const &c) const {
	for(competences_t i = premiereCompetence; i != nbCompetences; ++i) {
		if(_valeurs[i] <= c._valeurs[i])
			return false;
	}
	
	return true;
}

bool Personnage::Competences::operator>=(Competences const &c) const {
	return !(*this < c);
}

TiXmlElement *Personnage::Competences::sauvegarde() const {
	TiXmlElement *el = new TiXmlElement("Competences");
	for(competences_t c = premiereCompetence; c != nbCompetences; ++c) {
		std::string val = "c" + nombreVersTexte(c);
		el->SetAttribute(val, (*this)[c]);
	}
	
	return el;
}

void Personnage::Competences::restaurer(TiXmlElement *e) {
	TiXmlElement *el = e->FirstChildElement("Competences");
	for(competences_t c = premiereCompetence; c != nbCompetences; ++c) {
		std::string val = "c" + nombreVersTexte(c);
		el->Attribute(val, &(*this)[c]);
	}
}

Personnage::competences_t &operator++(Personnage::competences_t &p) {
	p = static_cast<Personnage::competences_t>(static_cast<int>(p) + 1);
	return p;
}

Personnage::positionTenue_t &operator++(Personnage::positionTenue_t &p) {
	p = static_cast<Personnage::positionTenue_t>(static_cast<int>(p) + 1);
	return p;
}

Personnage::Personnage(bool decoupagePerspective, Niveau *n, uindex_t index, ElementNiveau::elementNiveau_t cat, Inventaire *inventaire) : EntiteMobile(decoupagePerspective, n, index, cat), _vitesse(10), _vieActuelle(0), _vieTotale(200), _delaisAction(), _inventaire(inventaire), _competences(), _cibleAttaque(0) {
	TiXmlElement *e = ElementNiveau::description(index, cat);
	int t;
	if(e->Attribute("vitesse"))
		e->Attribute("vitesse", &_vitesse);
	if(e->Attribute("vie")) {
		e->Attribute("vie", &t);
		_vieTotale = t; 
	}
	_vieActuelle = _vieTotale;
	
	
	for(EntiteMobile::action_t a = EntiteMobile::premiereAction; a != EntiteMobile::nbActions; ++a) {
		_delaisAction[a]._cdAbsolu = 0;
		_delaisAction[a]._cooldown = 0;
	}
	_delaisAction[a_attaquer]._cooldown = 0.5f;
	
	std::memset(_tenue, 0, nbPositionsTenue * sizeof(ObjetInventaire *));
	
	for(competences_t i = premiereCompetence; i != nbCompetences; ++i) {
		_competences[i] = 1;
	}
}

Personnage::~Personnage() {
	delete _inventaire;
}

void Personnage::animer() {
	this->EntiteMobile::animer();
	if(this->actionActuelle() == a_attaquer && this->imageActuelle() == this->imageAttaque()) {
		this->attaquer(0);
	}
	else if(this->categorieMobile() != em_joueur) {
		this->interagir();
	}
}

bool Personnage::centrage() const {
	return true;
}

Coordonnees Personnage::origine() const {
	return Coordonnees(this->cadre().largeur / 2, 3 * this->cadre().hauteur / 4);
}

double Personnage::vitesse() const {
	return _vitesse;
}

unsigned int Personnage::vieActuelle() const {
	return _vieActuelle;
}

void Personnage::modifierVieActuelle(int delta) {
	_vieActuelle = std::min<ssize_t>(_vieTotale, std::max<ssize_t>(0, _vieActuelle + delta));
	if(_vieActuelle == 0) {
		this->mourir();
	}
}

unsigned int Personnage::vieTotale() const {
	return _vieTotale;
}

void Personnage::definirVieTotale(int vie) {
	_vieTotale = vie;
	_vieActuelle = std::min(_vieActuelle, _vieTotale);
}

bool Personnage::definirAction(action_t a) {
	if(horloge() > _delaisAction[a]._cdAbsolu) {
		if(EntiteMobile::definirAction(a)) {
			_delaisAction[a]._cdAbsolu = horloge() + _delaisAction[a]._cooldown;

			return true;
		}
	}
	
	return false;
}

Niveau::couche_t Personnage::couche() const {
	return this->mort() ? Niveau::cn_sol2 : Niveau::cn_objet;
}

ObjetInventaire const *Personnage::tenue(positionTenue_t p) const {
	return _tenue[p];
}

ObjetInventaire *Personnage::tenue(positionTenue_t p) {
	return _tenue[p];
}

void Personnage::definirTenue(positionTenue_t p, ObjetInventaire *o) {
	_tenue[p] = o;
}

Inventaire const *Personnage::inventaire() const {
	return _inventaire;
}

Inventaire *Personnage::inventaire() {
	return _inventaire;
}

bool Personnage::peutEquiperObjet(ObjetInventaire *objet) {
	return this->competences() >= objet->competencesRequises();
}

bool Personnage::peutEquiperObjet(ObjetInventaire *objet, Personnage::positionTenue_t pos) {
	if(!this->peutEquiperObjet(objet) || !objet->tenue())
		return false;

	switch(pos) {
		case casque:
			return objet->categorieObjet() == ObjetInventaire::casque;
		case brasD:
		case brasG:
			return objet->categorieObjet() == ObjetInventaire::arme || objet->categorieObjet() == ObjetInventaire::bouclier;
		case armure:
			return objet->categorieObjet() == ObjetInventaire::armure;
		case gants:
			return objet->categorieObjet() == ObjetInventaire::gants;
		case bottes:
			return objet->categorieObjet() == ObjetInventaire::bottes;
		case nbPositionsTenue:
			return false;
	}
}

Personnage::Competences const &Personnage::competences() const {
	return _competences;
}

void Personnage::definirCompetences(Competences const &c) {
	_competences = c;
}

void Personnage::interagir() {
	index_t x = this->pX(), y = this->pY();
	
	for(int xx = -1; xx <= 1; ++xx) {
		for(int yy = -2; yy <= 1; ++yy) {
			Niveau::listeElements_t elements = this->niveau()->elements(x + xx, y + yy, this->couche());
			for(Niveau::elements_t::iterator i = elements.first; i != elements.second; ++i) {
				if(i->entite->mobile()) {
					if(static_cast<EntiteMobile *>(i->entite)->personnage() && !static_cast<EntiteMobile *>(i->entite)->mort()) {
						if(this->interagir(static_cast<Personnage *>(i->entite))) {
							break;
						}
					}
				}
			}
		}
	}
}

void Personnage::renaitre() {
	this->EntiteMobile::renaitre();
	_vieActuelle = _vieTotale;
}

void Personnage::mourir() {
	this->EntiteMobile::mourir();
	this->niveau()->modifierMonnaie(this->pX(), this->pY(), this->inventaire()->monnaie());
	this->inventaire()->modifierMonnaie(-this->inventaire()->monnaie());
	this->jeterObjets();
}

void Personnage::attaquer(Personnage *p) {
	if(_cibleAttaque && !p) {
		_cibleAttaque->modifierVieActuelle(-2);
		_cibleAttaque = 0;
	}
	else {
		this->definirAction(EntiteMobile::a_attaquer);
		_cibleAttaque = p;
	}
}
