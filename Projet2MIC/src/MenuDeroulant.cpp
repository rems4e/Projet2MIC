/*
 *  MenuDeroulant.cpp
 *  Jeu C++
 *
 *  Created by Rémi on 01/01/11.
 *  Copyright 2011 Rémi Saurel. All rights reserved.
 *
 */

#include "MenuDeroulant.h"
#include "Image.h"
#include <cassert>
#include "Affichage.h"

bool MenuDeroulant::_imagesChargees(false);
Image MenuDeroulant::_g;
Image MenuDeroulant::_d;
Image MenuDeroulant::_m;

bool MenuInterface::_imagesChargees(false);
Image MenuInterface::_fH;
Image MenuInterface::_fB;
double MenuInterface::_hF;
horloge_t MenuInterface::_horlogeDefilement(0.0f);

MenuInterface::MenuInterface(VueInterface *parent, std::vector<Unichar> const &elements, int selection) : VueInterface(Rectangle(), Couleur(220, 240)), _elements(elements.begin(), elements.end()) {	
	this->init(parent, selection);
}

MenuInterface::MenuInterface(VueInterface *parent, std::vector<std::pair<Unichar, index_t>> const &elements, int selection) : VueInterface(Rectangle(), Couleur(220, 240)), _elements(elements.begin(), elements.end()) {	
	this->init(parent, selection);
}

void MenuInterface::init(VueInterface *parent, index_t selection) {
	
	this->definirParent(parent);
	this->definirSousMenuSelectionne(selection);
	
	glm::vec2 dim(0, 5);
	for(std::vector<sous_menu>::iterator i = _elements.begin(); i != _elements.end(); ++i) {
		i->_texte.definir(POLICE_NORMALE, 14);
		i->_texte.definir(Couleur::noir);
		dim.x = std::max(dim.x, i->_texte.dimensions().x);
	}
	
	dim.x += 5 + Texte::dimensions("x", POLICE_NORMALE, 14).x + 9;
	
	this->definirDimensions(dim);
	
	if(!_imagesChargees) {
		_imagesChargees = true;
		_fH = Image(Session::cheminRessources() + "fleche menu h.png");
		_fB = Image(Session::cheminRessources() + "fleche menu b.png");
		_hF = Texte::hauteur(POLICE_NORMALE, 14);
		assert(_hF >= _fH.dimensions().y);
	}
}

void MenuInterface::definirSousMenuSelectionne(index_t selection) {
	this->definirValeurIndeterminee(false);
	_sousMenuSelectionne = selection;
	
	this->definirPosition(glm::vec2(0, _centrage.y - this->sousMenuSelectionne() * (_elements[0]._texte.dimensions().y + 5) - 5));
}

void MenuInterface::definirSousMenuSelectionneAvecTag(index_t tag) {
	for(std::vector<sous_menu>::iterator i = _elements.begin(); i != _elements.end(); ++i, assert(i != _elements.end())) {
		if(i->_tag == tag) {
			this->definirSousMenuSelectionne(i - _elements.begin());
			break;
		}
	}
}

void MenuInterface::dessiner() {
	Affichage::afficherRectangle(Rectangle(0, 0, Ecran::largeur(), Ecran::hauteur()), _couleurFond);
	
	Affichage::afficherRectangle(Rectangle(0, 0, this->dimensions().x, 1), Couleur::noir);
	Affichage::afficherRectangle(Rectangle(0, 0, 1, this->dimensions().y), Couleur::noir);
	Affichage::afficherRectangle(Rectangle(0, this->dimensions().y - 1, this->dimensions().x, 1), Couleur::noir);
	Affichage::afficherRectangle(Rectangle(this->dimensions().x - 1, 0, 1, this->dimensions().x), Couleur::noir);
	
	glm::vec2 pos(20, 5);
	
	if(_premierAffiche != 0) {
		_fH.afficher(glm::vec2((this->dimensions().x - _fH.dimensions().x) / 2, pos.y + (_hF - _fH.dimensions().y) / 2));
		pos.y += _hF + 5;
	}
	
	for(index_t i = _premierAffiche; i != _dernierAffiche; ++i) {
		_elements[i]._pos = pos;
		if(i == _sousMenuSurvole) {
			Affichage::afficherRectangle(Rectangle(0, pos.y - 2, this->dimensions().x, _elements[i]._texte.dimensions().y + 4), Couleur(60, 85, 250));
			_elements[i]._texte.definir(Couleur::blanc);
		}
		_elements[i]._texte.afficher(_elements[i]._pos);
		if(i == _sousMenuSurvole)
			_elements[i]._texte.definir(Couleur::noir);
		
		if(i + this->valeurIndeterminee() == _sousMenuSelectionne) {
			Texte::afficher("x", POLICE_NORMALE, 14, i == _sousMenuSurvole ? Couleur::blanc : Couleur::noir, glm::vec2(5, pos.y));
		}
		
		pos.y += _elements[i]._texte.dimensions().y + 5;
	}
	
	if(_dernierAffiche != _elements.size()) {
		_fB.afficher(glm::vec2((this->dimensions().x - _fH.dimensions().x) / 2, pos.y + (_hF - _fB.dimensions().y) / 2));
		pos.y += _hF + 5;
	}
}

void MenuInterface::definirMasque(bool m) {
	if(!m) {
		if(this->valeurIndeterminee())
			this->insererBlanc();
		_premierAffiche = _sousMenuSelectionne;
		this->definirPosition(glm::vec2(-_centrage.x, -_centrage.y));
		double p = this->positionAbsolue().y + (_premierAffiche != 0) * (_hF + 5);
		if(_premierAffiche != 0) {
			for(index_t i = _premierAffiche;;) {
				_premierAffiche = i;
				p -= _elements[i]._texte.dimensions().y + 5;
				if(p < 0) {
					_premierAffiche = i + 2;
					p += _elements[i]._texte.dimensions().y + 5;
					break;
				}
				if(i == 0)
					break;
				--i;
			}
		}
		
		_dernierAffiche = 0;
		this->definirPosition(glm::vec2(-_centrage.x, p - this->positionAbsolue().y));
		p = this->positionAbsolue().y + (_premierAffiche != 0) * (_hF + 5);
		for(index_t i = _premierAffiche; i != _elements.size(); ++i) {
			p += _elements[i]._texte.dimensions().y + 5;
			if(p > Ecran::hauteur()) {
				_dernierAffiche = i - 1;
				p -= _elements[i]._texte.dimensions().y + 5;
				break;
			}
		}
		if(_dernierAffiche == 0)
			_dernierAffiche = _elements.size();
		this->definirDimensions(glm::vec2(this->dimensions().x, p - this->positionAbsolue().y + 5));
	}
	else if(this->valeurIndeterminee()) {
		this->supprimerBlanc();
	}
	
	this->VueInterface::definirMasque(m);
}

void MenuInterface::insererBlanc() {
	assert(this->valeurIndeterminee());
	_elements.insert(_elements.begin(), sous_menu(" "));
	_elements.front()._texte.definir(POLICE_NORMALE, 14);
	_elements.front()._texte.definir(Couleur::noir);
	
}

void MenuInterface::supprimerBlanc() {
	assert(this->valeurIndeterminee() && _elements.front()._texte.texte() == " ");
	_elements.erase(_elements.begin());
}

void MenuInterface::gestionSouris(bool vueActuelle, glm::vec2 const &souris, bool gauche, bool /*droit*/) {
	if(!vueActuelle)
		return;
	
	if(_premierAffiche != 0 && Rectangle(0, 0, this->dimensions().x, _hF + 5).contientPoint(souris)) {
		horloge_t dif = horloge() - _horlogeDefilement;
		if(dif >= interpolationQuadratique<float>(0.0, 0.02, _hF + 5, 0.2, souris.y)) {
			_horlogeDefilement = horloge();
			int nbF = (_premierAffiche != 0) + (_dernierAffiche != _elements.size());
			--_premierAffiche;
			if(this->positionAbsolue().y + this->dimensions().y + _hF + 5 < Ecran::hauteur()) {
				Rectangle cc = this->cadre();
				cc.hauteur += _hF + 5;
				this->definirCadre(cc);
			}
			else {
				--_dernierAffiche;
				_dernierAffiche += nbF - ((_premierAffiche != 0) + (_dernierAffiche != _elements.size()));
			}
		}
		return;
	}
	else if(_dernierAffiche != _elements.size() && Rectangle(0, this->dimensions().y - _hF - 5, this->dimensions().x, _hF + 5).contientPoint(souris)) {
		horloge_t dif = horloge() - _horlogeDefilement;
		if(dif >= interpolationQuadratique<float>(0.0, 0.02, _hF + 5, 0.2, souris.y - this->dimensions().y)) {
			_horlogeDefilement = horloge();
			int nbF = (_premierAffiche != 0) + (_dernierAffiche != _elements.size());
			++_dernierAffiche;
			if(this->positionAbsolue().y >= _hF + 5) {
				Rectangle cc = this->cadre();
				cc.haut -= _hF + 5;
				cc.hauteur += _hF + 5;
				this->definirCadre(cc);
			}
			else {
				++_premierAffiche;
				_premierAffiche -= nbF - ((_premierAffiche != 0) + (_dernierAffiche != _elements.size()));
			}
		}
		return;
	}
	
	_sousMenuSurvole = -1;
	for(index_t i = _premierAffiche; i != _dernierAffiche; ++i) {
		if(Rectangle(_elements[i]._pos - glm::vec2(0, 2.5), glm::vec2(this->dimensions().x, _elements[i]._texte.dimensions().y + 5)).contientPoint(souris)) {
			_sousMenuSurvole = i;
			break;
		}
	}
	
	if(gauche) {
		if(_sousMenuSurvole != -1) {
			if(this->valeurIndeterminee()) {
				if(_sousMenuSurvole != 0) {
					this->supprimerBlanc();
					this->definirSousMenuSelectionne(_sousMenuSurvole - 1);
				}
			}
			else
				this->definirSousMenuSelectionne(_sousMenuSurvole);
		}
		this->definirMasque(true);
		static_cast<MenuDeroulant *>(this->parent())->exec();
		Session::reinitialiser(Session::B_GAUCHE);
	}	
}

void MenuInterface::gestionClavier() {
	if(Session::evenement(Session::T_ESC)) {
		VueInterface::definirVueActuelle(0);
		return;
	}	
}

void MenuInterface::prendreLaMain(VueInterface *) {
	_sousMenuSurvole = _sousMenuSelectionne;
}

void MenuInterface::rendreLaMain(VueInterface *) {
	if(!this->masque())
		this->definirMasque(true);
}

void MenuInterface::definirElements(std::vector<Unichar> const &elements) {
	_elements.assign(elements.begin(), elements.end());
	glm::vec2 dim(0, 5);
	for(std::vector<sous_menu>::iterator i = _elements.begin(); i != _elements.end(); ++i) {
		i->_texte.definir(POLICE_NORMALE, 14);
		i->_texte.definir(Couleur::noir);
		dim.x = std::max(dim.x, i->_texte.dimensions().x);
	}
	
	dim.x += 5 + Texte::dimensions("x", POLICE_NORMALE, 14).x + 9;
	
	this->definirDimensions(dim);
	this->definirSousMenuSelectionne(0);	
}

void MenuInterface::definirElements(std::vector<std::pair<Unichar, index_t> > const &elements) {
	_elements.assign(elements.begin(), elements.end());
	glm::vec2 dim(0, 5);
	for(std::vector<sous_menu>::iterator i = _elements.begin(); i != _elements.end(); ++i) {
		i->_texte.definir(POLICE_NORMALE, 14);
		i->_texte.definir(Couleur::noir);
		dim.x = std::max(dim.x, i->_texte.dimensions().x);
	}
	
	dim.x += 5 + Texte::dimensions("x", POLICE_NORMALE, 14).x + 9;
	
	this->definirDimensions(dim);
	this->definirSousMenuSelectionne(0);	
}

MenuDeroulant::MenuDeroulant(glm::vec2 const &pos, std::vector<Unichar> const &elements, int sousMenuSelectionne) : ControleInterface(Rectangle(pos, glm::vec2(0))), _menu(this, elements, sousMenuSelectionne) {
	this->definirDimensions(glm::vec2(180, 23));
	
	if(!_imagesChargees) {
		_imagesChargees = true;
		_g = Image(Session::cheminRessources() + "menuDG.png");
		_m = Image(Session::cheminRessources() + "menuDM.png");
		_d = Image(Session::cheminRessources() + "menuDD.png");
	}
	
	_menu.definirCouche(1);
	_menu.definirMasque(true);
	
	_menu.definirCentrage(glm::vec2(5, (this->dimensions().y - _menu.texteSelectionne().dimensions().y)) / 2.0f);
	_menu.definirDimensions(glm::vec2(std::max(_menu.dimensions().x, this->dimensions().x - 10), _menu.dimensions().y));
}

MenuDeroulant::~MenuDeroulant() {
	this->supprimerEnfant(_menu);
}

void MenuDeroulant::dessiner() {
	this->definirValeurTexte(this->menuSelectionne());
	
	_g.afficher(glm::vec2(0));
	for(int i = 0; i < this->dimensions().x - _g.dimensions().x - _d.dimensions().x; ++i) {
		_m.afficher(glm::vec2(_g.dimensions().x + i, 0)); 
	}
	_d.afficher(glm::vec2(this->dimensions().x - _d.dimensions().x, 0));
	
	if(_menu.masque()) {
		if(!_menu.valeurIndeterminee()) {
			Texte &t = _menu.texteSelectionne();
			t.afficher(_menu.centrage());
		}
	}
}

void MenuDeroulant::gestionSouris(bool, glm::vec2 const &/*souris*/, bool gauche, bool /*droit*/) {	
	if(gauche && !_menu.elements().empty()) {
		_menu.definirMasque(false);
		VueInterface::definirVueActuelle(&_menu);
		Session::reinitialiser(Session::B_GAUCHE);
	}
}
