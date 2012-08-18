/*
 *  Bouton.cpp
 *  Jeu C++
 *
 *  Created by Rémi on 15/08/07.
 *  Copyright 2007 Rémi Saurel. All rights reserved.
 *
 */

#include "Bouton.h"
#include "Affichage.h"
#include "Session.h"

Bouton::Bouton(glm::vec2 const &position, std::string const &texte, double h) : ControleInterface(Rectangle()), _texte(texte, POLICE_NORMALE, 14, Couleur::noir), _image(), _estImage(false), _valeur(false), _comportement(Bouton::normal), _enfonce(false) {
	Rectangle dim = Rectangle(position, _texte.dimensions() + glm::vec2(4, 4));
	if(h > 0) {
		dim.hauteur = h;
	}
	this->definirCadre(dim);
}

Bouton::Bouton(glm::vec2 const &position, Image const &image) : ControleInterface(Rectangle(position, image.dimensions())), _texte(), _image(image), _estImage(true), _valeur(false), _comportement(Bouton::normal), _enfonce(false) {
}

Bouton::~Bouton() {}

Image const &Bouton::image() const {
	return _image;
}

void Bouton::definirImage(Image const &img) {
	_image = img;
	_texte = Texte();
	
	_estImage = true;
}

Unichar const &Bouton::texte() const {
	return _texte.texte();
}

void Bouton::definirTexte(Unichar const &txt) {
	_texte.definir(txt);
	_image = Image();
	
	_estImage = false;
}

void Bouton::dessiner() {
	if(estImage()) {
		Couleur t = Affichage::teinte();
		if(!_actif) {
			Affichage::definirTeinte(Couleur(0, 180));
		}
		else if(_enfonce || (_clic && this->cadreAbsolu().contientPoint(Session::souris()))) { // clic
			Affichage::definirTeinte(Couleur(0, 150));
		}
		else if(_valeur) { // enfoncé
			Affichage::definirTeinte(Couleur(0, 100));
		}
		else { // état normal
			Affichage::definirTeinte(Couleur::blanc);
		}
		
		_image.afficher(glm::vec2(0));
		Affichage::definirTeinte(t);
	}
	else if(estTexte()) {
		Affichage::afficherRectangle(Rectangle(0, 0, Ecran::largeur(), Ecran::hauteur()), Couleur::noir);
		Couleur fond, texte;
		if(!_actif) {
			fond = Couleur::grisClair;
			texte = Couleur::grisFonce;
		}
		else if(_clic && this->cadreAbsolu().contientPoint(Session::souris())) {
			fond = Couleur(50, 255);
			texte = Couleur::noir;
		}
		else if(_valeur) {
			fond = Couleur(90, 255);
			texte = Couleur::noir;
		}
		else {
			fond = Couleur::gris;
			texte = Couleur::noir;
		}
		Affichage::afficherRectangle(Rectangle(glm::vec2(1, 1), this->dimensions() - glm::vec2(2, 2)), fond);
		_texte.definir(texte);
		_texte.afficher(this->dimensions() / 2.0f - _texte.dimensions() / 2.0f);
	}
	
	_enfonce = false;
}

void Bouton::gestionClavier() {
	if(!_clic) {
		if(Session::modificateurTouches() == _modificateurRaccourci && _raccourci != Session::aucunEvenement && Session::evenementDiscret(_raccourci)) {
			if(this->comportement() == on_off)
				_valeur = !_valeur;

			this->executerAction();
			_enfonce = true;
		}
	}
	
}

VueInterface *Bouton::gestionClic() {
	bool clic = _clic;
	VueInterface *a = VueInterface::gestionClic();
	
	if(clic && !_clic && this->cadreAbsolu().contientPoint(Session::souris())) {
		if(_comportement == on_off)
			this->definirValeurBool(!_valeur);
		this->executerAction();
	}
	
	return a;
}

int Bouton::valeurInt() const {
	return this->valeurBool();
}

bool Bouton::valeurBool() const {
	return _comportement == on_off ? _valeur : _clic;
}

void Bouton::definirValeurBool(bool valeur) {
	this->ControleInterface::definirValeurInt(valeur);
	_valeur = valeur;
}

void Bouton::definirComportement(int c) {
	if(c == Bouton::normal) {
		this->definirValeurBool(false);
	}
	_comportement = c;
}

void GroupeBoutons::actionMembre(ControleInterface *bouton, void *groupe) {
	GroupeBoutons *gp = static_cast<GroupeBoutons *>(groupe);
	int el = 0;
	for(std::list<VueInterface *>::iterator i = gp->_objets.begin(); i != gp->_objets.end(); ++i, ++el) {
		if(*i == bouton) {
			break;
		}
	}
	if(!gp->obligationChoix() || el != gp->elementSelectionne())
		gp->definirSelection(el, true);
}

GroupeBoutons::GroupeBoutons(glm::vec2 const &pos, Bouton **elems, glm::vec2 const &dim, glm::vec2 const &ecart, int elemSelect, bool obligatChoix) : ControleInterface(Rectangle(pos, dim)), _dimensionsGroupe(dim), _ecartementGroupe(ecart), _obligationChoix(obligatChoix), _elementSelectionne(-1) {			
	for(int i = 0; i < dim.x; ++i) {
		for(int j = 0; j < dim.y; ++j) {
			this->ajouterEnfant(*elems[static_cast<int>(j * dim.x + i)]);
			elems[static_cast<int>(j * dim.x + i)]->definirAction(GroupeBoutons::actionMembre);
			elems[static_cast<int>(j * dim.x + i)]->definirParametreAction(this);
		}
	}
	this->definirCadre(Rectangle(pos, glm::vec2(0)));
	this->definirSelection(elemSelect, false);
}

GroupeBoutons::~GroupeBoutons() {
}

void GroupeBoutons::definirSelection(int elem, bool action) {
	if(_elementSelectionne != -1) {
		this->element(_elementSelectionne)->definirValeurBool(false);
	}
	_elementSelectionne = elem;
	if(_elementSelectionne != -1) {
		this->element(_elementSelectionne)->definirValeurBool(true);
	}
	
	if(action) {
		this->executerAction();
	}
}

void GroupeBoutons::definirCadre(Rectangle const &c) {
	glm::vec2 d(0);
	for(int i = 0; i < _dimensionsGroupe.y; ++i) {
		for(int j = 0; j < _dimensionsGroupe.x; ++j) {
			Rectangle rect = this->element(i * static_cast<int>(_dimensionsGroupe.x) + j)->cadre();
			rect.gauche = 0, 0;
			if(i != 0)
				rect.haut = this->element((i - 1) * static_cast<int>(_dimensionsGroupe.x) + j)->cadre().haut + this->element((i - 1) * static_cast<int>(_dimensionsGroupe.x) + j)->cadre().hauteur;
			if(j != 0)
				rect.gauche = this->element(i * static_cast<int>(_dimensionsGroupe.x) + j - 1)->cadre().gauche + this->element(i * static_cast<int>(_dimensionsGroupe.x) + j - 1)->cadre().largeur;
			
			this->element(i * static_cast<int>(_dimensionsGroupe.x) + j)->definirCadre(rect);

			if(i == _dimensionsGroupe.y - 1)
				d.y = std::max(d.y, this->element(i * static_cast<int>(_dimensionsGroupe.x) + j)->cadre().haut + this->element(i * static_cast<int>(_dimensionsGroupe.x) + j)->cadre().hauteur - this->element(j)->cadre().haut);
			if(j == _dimensionsGroupe.x - 1)
				d.x = std::max(d.x, this->element(i * static_cast<int>(_dimensionsGroupe.x) + j)->cadre().gauche + this->element(i * static_cast<int>(_dimensionsGroupe.x) + j)->cadre().largeur - this->element(i * static_cast<int>(_dimensionsGroupe.x))->cadre().gauche);
		}
	}
	this->VueInterface::definirCadre(Rectangle(glm::vec2(c.gauche, c.haut), d));
}
