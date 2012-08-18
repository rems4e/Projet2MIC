//
//  Personnage.cpp
//  Projet2MIC
//
//  Créé par Marc Promé et Rémi Saurel.
//  Ce fichier et son contenu sont librement distribuables, modifiables et utilisables pour toute œuvre non commerciale, à condition d'en citer les auteurs.
//

#include "Personnage.h"
#include "Session.h"
#include "tinyxml.h"
#include <cstring>
#include "Inventaire.h"
#include "ObjetInventaire.h"

Personnage::Competences::Competences() {
	for(competences_t i = premiereCompetence; i != nbCompetences; ++i) {
		_valeurs[i] = 1;
	}
}

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
	for(competences_t i = premiereCompetence; i != nbCompetences; ++i) {
		if(_valeurs[i] > c._valeurs[i])
			return false;
	}
	
	return true;
}

bool Personnage::Competences::operator>(Competences const &c) const {
	for(competences_t i = premiereCompetence; i != nbCompetences; ++i) {
		if(_valeurs[i] <= c._valeurs[i])
			return false;
	}
	
	return true;
}

bool Personnage::Competences::operator>=(Competences const &c) const {
	for(competences_t i = premiereCompetence; i != nbCompetences; ++i) {
		if(_valeurs[i] < c._valeurs[i])
			return false;
	}
	
	return true;
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
		if(el->Attribute(val))
			el->Attribute(val, &(*this)[c]);
		else
			(*this)[c] = 1;
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

Personnage::Personnage(bool decoupagePerspective, Niveau *n, uindex_t index, ElementNiveau::elementNiveau_t cat, Inventaire *inventaire) : EntiteMobile(decoupagePerspective, n, index, cat), _vitesse(10), _vieActuelle(0), _delaisAction(), _inventaire(inventaire), _competences(), _cibleAttaque(0) {
	TiXmlElement *e = ElementNiveau::description(index, cat);
	if(e->Attribute("vitesse"))
		e->Attribute("vitesse", &_vitesse);
	
	
	for(EntiteMobile::action_t a = EntiteMobile::premiereAction; a != EntiteMobile::nbActions; ++a) {
		_delaisAction[a]._cdAbsolu = 0;
		_delaisAction[a]._cooldown = 0;
	}
	_delaisAction[a_attaquer]._cooldown = 0.5f;
	
	std::memset(_tenue, 0, nbPositionsTenue * sizeof(ObjetInventaire *));
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
		this->interagir(false);
	}
}

bool Personnage::centrage() const {
	return true;
}

glm::vec2 Personnage::origine() const {
	return glm::vec2(this->cadre().largeur / 2, 3 * this->cadre().hauteur / 4);
}

double Personnage::vitesse() const {
	return _vitesse;
}

int Personnage::vieActuelle() const {
	return _vieActuelle;
}

void Personnage::modifierVieActuelle(int delta) {
	_vieActuelle = std::min<int>(this->vieTotale(), std::max<int>(0, _vieActuelle + delta));
	if(_vieActuelle == 0) {
		this->EntiteMobile::preparerMort();
	}
}

int Personnage::vieTotale() const {
	return this->competences()[endurance] * 10;
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

Personnage::Competences &Personnage::competences() {
	return _competences;
}

void Personnage::definirCompetences(Competences const &c) {
	_competences = c;
}

Personnage *Personnage::interagir(bool test) {
	index_t x = this->pX(), y = this->pY();
	
	for(int xx = -1; xx <= 1; ++xx) {
		for(int yy = -1; yy <= 1; ++yy) {
			Niveau::listeElements_t elements = this->niveau()->elements(x + xx, y + yy, this->couche());
			for(Niveau::elements_t::iterator i = elements.first; i != elements.second; ++i) {
				if(i->_entite->mobile()) {
					if(static_cast<EntiteMobile *>(i->_entite)->personnage() && !static_cast<EntiteMobile *>(i->_entite)->mort()) {
						if(this->interagir(static_cast<Personnage *>(i->_entite), test)) {
							return static_cast<Personnage *>(i->_entite);
						}
					}
				}
			}
		}
	}
	
	return 0;
}

void Personnage::mourir() {
	this->EntiteMobile::mourir();
	this->niveau()->modifierMonnaie(this->pX(), this->pY(), this->inventaire()->monnaie());
	this->inventaire()->modifierMonnaie(-this->inventaire()->monnaie());
	this->jeterObjets();
}

bool Personnage::attaquer(Personnage *p) {
	if(_cibleAttaque && !p && glm::length(this->position() - _cibleAttaque->position()) < 2 * LARGEUR_CASE) {
		int degats = this->competences()[force];
		int defense = _cibleAttaque->competences()[agilite];
		for(positionTenue_t p = premierePositionTenue; p != nbPositionsTenue; ++p) {
			if(this->tenue(p))
				degats += this->tenue(p)->attaque();
			if(_cibleAttaque->tenue(p))
				defense += _cibleAttaque->tenue(p)->defense();
		}
		
		_cibleAttaque->modifierVieActuelle(-degats * (1 - std::min(defense, 100) / 100.0));
		_cibleAttaque = 0;
		Audio::jouerSon(this->niveau()->attaque());
		
		return true;
	}
	else {
		this->definirAction(EntiteMobile::a_attaquer);
		_cibleAttaque = p;
	}
	
	return false;
}

Unichar Personnage::nomCompetence(competences_t c) {
	switch(c) {
		case force:
			return TRAD("compét Force");
		case agilite:
			return TRAD("compét Agilité");
		case endurance:
			return TRAD("compét Endurance");
		case nbCompetences:
			return Unichar::uninull;
	}
}

void Personnage::definirNiveau(Niveau *n) {
	this->EntiteMobile::definirNiveau(n);
	_inventaire->definirNiveau(n);
}


