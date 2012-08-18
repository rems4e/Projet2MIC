/*
 *  VueDefilante.cpp
 *  Jeu C++
 *
 *  Created by Rémi on 26/01/11.
 *  Copyright 2011 Rémi Saurel. All rights reserved.
 *
 */

#include "VueDefilante.h"
#include "Image.h"
#include "Session.h"
#include <algorithm>
#include "Affichage.h"

static bool imagesChargees = false;
Image VueDefilante::_curseurH;
Image VueDefilante::_curseurB;
Image VueDefilante::_curseurM;
Image VueDefilante::_barreH;
Image VueDefilante::_barreB;
Image VueDefilante::_barreBB;
Image VueDefilante::_barreBH;
Image VueDefilante::_barreM;
Image VueDefilante::_desact;

VueDefilante::VueDefilante(Rectangle const &cadre, VueInterface *vue) : VueInterface(cadre, Couleur(0, 96)), _decalage(), _barresMasquees(false), _vue(vue), _fH(false), _fB(false), _c(false), _clic2(false), _fAff(false), _cAff(false), _hCurseur(0.0), _pClic(0.0), _pCurseur(0.0), _depC(false) {
	if(!imagesChargees) {
		imagesChargees = true;
		_curseurH = Image(Session::cheminRessources() + "barre curseur haut.png");
		_curseurB = Image(Session::cheminRessources() + "barre curseur bas.png");
		_curseurM = Image(Session::cheminRessources() + "rallonge barre curseur.png");
		_barreH = Image(Session::cheminRessources() + "haut barre.png");
		_barreB = Image(Session::cheminRessources() + "flèches barre.png");
		_barreBB = Image(Session::cheminRessources() + "flèches barres bas.png");
		_barreBH = Image(Session::cheminRessources() + "flèches barres haut.png");
		_barreM = Image(Session::cheminRessources() + "rallonge barre.png");
		_desact = Image(Session::cheminRessources() + "barre désactivée.png");
	}
	
	VueInterface *lim = new VueInterface(Rectangle(0, 0, cadre.largeur - _barreM.dimensions().x, cadre.hauteur));
	
	this->VueInterface::ajouterEnfant(*lim);
	lim->ajouterEnfant(*_vue);
	_vue->definirPosition(glm::vec2(0));
}

VueDefilante::~VueDefilante() {
	_objets.front()->supprimerEnfant(*_vue);
	VueInterface *lim = _objets.front();
	this->VueInterface::supprimerEnfant(*lim);
	delete lim;
}

void VueDefilante::definirCadre(Rectangle const &c) {
	this->VueInterface::definirCadre(c);
	_objets.front()->definirCadre(Rectangle(0, 0, this->cadre().largeur - _barreM.dimensions().x, this->cadre().hauteur));

}

void VueDefilante::definirDecalage(glm::vec2 const &d) {
	_decalage.x = std::min(d.x, this->dimensionsTotales().x - _objets.front()->cadre().largeur);
	_decalage.y = std::min(d.y, this->dimensionsTotales().y - _objets.front()->cadre().hauteur);
	_decalage.x = std::max<coordonnee_t>(_decalage.x, 0.0);
	_decalage.y = std::max<coordonnee_t>(_decalage.y, 0.0);

	_vue->definirPosition(_decalage * -1.0f);
}

void VueDefilante::definirDimensionsTotales(glm::vec2 const &dim) {
	Rectangle r = _vue->cadre();
	r.largeur = std::max(dim.x, _objets.front()->cadre().largeur);
	r.hauteur = std::max(dim.y, _objets.front()->cadre().hauteur);
	_vue->definirCadre(r);
	
	this->definirDecalage(_decalage);
}

void VueDefilante::gestionClavier() {
}

void VueDefilante::gestionSouris(bool /*vueActuelle*/, glm::vec2 const &souris, bool gauche, bool /*droit*/) {
	if(!_clic) {
		_clic2 = false;
		return;
	}

	if(gauche && !_clic2 && _fAff) {
		_clic2 = true;
		_fB = _fH = _c = false;
		if(souris.x >= this->dimensions().x - _barreM.dimensions().x) {
			if(souris.y >= 6 + (22 - 6) * !_cAff) {
				if(souris.y <= this->dimensions().y - 32 && _cAff) {
					_c = true;
					_depC = souris.y >= _pCurseur && souris.y <= _pCurseur + _hCurseur;
					
					_pClic = souris.y - _pCurseur;
				}
				else if(souris.y <= this->dimensions().y - 16)
					_fH = true;
				else
					_fB = true;
			}
		}
	}
	else if(!gauche) {
		_clic2 = false;
		_fB = _fH = _c = false;
	}
	
	if(gauche && (_fH || _fB || _c)) {
		if(_fH) {
			this->definirDecalage(glm::vec2(0, _decalage.y - 2));
		}
		else if(_fB) {
			this->definirDecalage(glm::vec2(0, _decalage.y + 2));
		}
		else if(_c) {
			if(_depC) {
				this->definirDecalage(glm::vec2(0, (souris.y - _pClic - 6) / _hCurseur * this->dimensions().y));
			}
			else {
				this->definirDecalage(glm::vec2(0, (souris.y - _hCurseur / 2 - 6) / _hCurseur * this->dimensions().y));
			}
		}
		
	}
}

void VueDefilante::dessiner() {
	Affichage::afficherRectangle(Rectangle(0, 0, Ecran::largeur(), Ecran::hauteur()), _couleurFond);
	//afficherBarres();
	if(this->dimensionsTotales().y > this->cadre().hauteur && this->dimensions().y >= _barreH.dimensions().y + _barreB.dimensions().y) {
		_barreH.afficher(glm::vec2(this->dimensions().x - _barreH.dimensions().x, 0));
		for(index_t i = _barreH.dimensions().y; i < this->dimensions().y - _barreB.dimensions().y; ++i) {
			_barreM.afficher(glm::vec2(this->dimensions().x - _barreM.dimensions().x, i));
		}
		_barreB.afficher(glm::vec2(this->dimensions().x - _barreB.dimensions().x, this->dimensions().y - _barreB.dimensions().y));
		
		_fAff = true;
		
		if(this->dimensions().y >= _barreH.dimensions().y + _barreB.dimensions().y + _curseurH.dimensions().y + _curseurB.dimensions().y) {
			_cAff = true;
			_hCurseur = (this->dimensions().y - 6 - 32) * this->dimensions().y / this->dimensionsTotales().y;
			double h = _hCurseur - _curseurH.dimensions().y - _curseurB.dimensions().y;
			_pCurseur = this->decalage().y / this->dimensions().y * _hCurseur + 6;

			_curseurH.afficher(glm::vec2(this->dimensions().x - _curseurH.dimensions().x - 1, _pCurseur));
			
			size_t nb = std::floor(h / _curseurM.dimensions().y), part = std::fmod(h, _curseurM.dimensions().y), i;
			for(i = 0; i < nb; ++i) {
				_curseurM.afficher(glm::vec2(this->dimensions().x - _curseurH.dimensions().x - 1, _pCurseur + _curseurH.dimensions().y + i * _curseurM.dimensions().y));
			}
			if(part > 0.0) {
				_curseurM.afficher(glm::vec2(this->dimensions().x - _curseurH.dimensions().x - 1, _pCurseur + _curseurH.dimensions().y + i * _curseurM.dimensions().y), Rectangle(0, 0, _curseurM.dimensions().x, part));
			}
			
			_curseurB.afficher(glm::vec2(this->dimensions().x - _curseurB.dimensions().x - 1, _pCurseur + _hCurseur - _curseurB.dimensions().y));
		}
	}
	else {
		for(index_t i = 0; i < this->dimensions().y; ++i) {
			_desact.afficher(glm::vec2(this->dimensions().x - _desact.dimensions().x, i));
		}
		_fAff = _cAff = false;
		_fH = _fB = _c = false;
	}
}
