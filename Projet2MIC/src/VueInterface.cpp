/*
 *  VueInterface.cpp
 *  Jeu C++
 *
 *  Created by Rémi on 18/09/10.
 *  Copyright 2010 Rémi Saurel. All rights reserved.
 *
 */

#include "VueInterface.h"
#include <limits>
#include "Session.h"
#include "ControleInterface.h"
#include "Affichage.h"

namespace Session {
	void reinitialiserEvenementsClavier();
}

namespace Ecran {
	void ajouterVerrouTransformations();
	// On ne doit pas supprimer la première case de la pile.
	void supprimerVerrouTransformations() throw(int);
	void retourVerrouTransformations();
}

VueInterface *VueInterface::_dernierSurvole = 0;
horloge_t *VueInterface::_tempsSurvol = 0;

VueInterface *VueInterface::_vueActuelle = 0;

VueInterface::VueInterface(Rectangle const &cadre, Couleur const &couleurFond) : _parent(0), _cadre(cadre), _couleurFond(couleurFond), _objets(), _description(), _actif(true), _clic(false), _tag(0L), _masque(false), _couche(0) {
	
}

VueInterface::~VueInterface() {
	for(std::list<VueInterface *>::iterator i = _objets.begin(); i != _objets.end(); ++i) {
		(*i)->_parent = 0;
	}
	if(_parent) {
		_parent->supprimerEnfant(*this);
	}
}

bool VueInterface::appartientDescendence(VueInterface *vue) const {
	return vue && (vue == this || (_parent && _parent->appartientDescendence(vue)));
}

VueInterface *VueInterface::premierParentFocus() {
	if(this->focusClavier())
		return this;
	else if(_parent)
		return _parent->premierParentFocus();
	else
		return 0;
}


glm::vec2 VueInterface::positionAbsolue() const {
	if(this->parent()) {
		return this->parent()->positionAbsolue() + this->position();
	}
	else {
		return this->position();
	}
}

Rectangle VueInterface::cadreAbsolu() const {
	if(this->parent()) {
		return this->couche() == this->parent()->couche() ? this->parent()->cadreAbsolu().intersection(this->cadre().cadreDecale(this->parent()->positionAbsolue())) : this->cadre().cadreDecale(this->parent()->positionAbsolue());
	}
	else {
		return this->cadre();
	}
}

void VueInterface::definirMasque(bool masque) {
	if(masque) {
		_masque = true;
		if(VueInterface::vueActuelle() == this) {
			VueInterface::definirVueActuelle(0);
		}
	}
	else {
		_masque = false;
	}
}

void VueInterface::dessiner() {
	Affichage::afficherRectangle(this->cadre(), _couleurFond);
}

void VueInterface::preparationDessin() {
	
}

void VueInterface::afficher(glm::vec2 const &posAbs, int couche, int &coucheSuivante) {
	if(this->masque()) {
		return;
	}

	if(this->couche() > couche) {
		if(coucheSuivante == couche)
			coucheSuivante = this->couche();
		else
			coucheSuivante = std::min(coucheSuivante, this->couche());
		return;
	}
	
	if(couche <= this->couche())
		this->dessiner();

	for(std::list<VueInterface *>::iterator i = _objets.begin(); i != _objets.end(); ++i) {
		(*i)->preparationDessin();
		
		if(couche <= (*i)->couche())
			Ecran::ajouterCadreAffichage((*i)->cadre().cadreDecale(posAbs + this->position()));
		Ecran::ajouterVerrouTransformations();
		
		(*i)->afficher(posAbs + this->position(), couche, coucheSuivante);
		glDisable(GL_DEPTH_TEST);
		
		Ecran::retourVerrouTransformations();
		Ecran::supprimerVerrouTransformations();
		if(couche <= (*i)->couche())
			Ecran::supprimerCadreAffichage();
	}
}

VueInterface *VueInterface::gestionClic() {	
	VueInterface *vue = 0;
	for(std::list<VueInterface *>::iterator i = _objets.begin(); i != _objets.end(); ++i) {
		if(!(*i)->actif() || (*i)->masque())
			continue;
		VueInterface *tmp = (*i)->gestionClic();
		if(!vue || (tmp && tmp->couche() > vue->couche()))
			vue = tmp;
	}

 	if(!Session::evenement(Session::B_GAUCHE)) {
		_clic = false;
		return 0;
	}
	else if(this->cadreAbsolu().contientPoint(Session::souris()) && !vue && (!VueInterface::vueActuelle() || !VueInterface::vueActuelle()->clic() || VueInterface::vueActuelle() == this)) {
		_clic = true;
		vue = this;
	}

	return vue;
}

VueInterface *VueInterface::gestionSurvol() {
	if(this->masque())
		return 0;
	
	VueInterface *vue = 0;
	for(std::list<VueInterface *>::const_iterator i = _objets.begin(); i != _objets.end(); ++i) {
		VueInterface *tmp = (*i)->gestionSurvol();
		if(!vue || (tmp && tmp->couche() > vue->couche()))
			vue = tmp;
	}
	
	if(this->cadreAbsolu().contientPoint(Session::souris()) && !vue) {
		vue = this;
	}
	
	return vue;
}

void VueInterface::gestionClavier() {
	
}

void VueInterface::gestionSouris(bool, glm::vec2 const &, bool, bool) {
	
}

void VueInterface::ajouterEnfant(VueInterface &v) {
	if(std::find(_objets.begin(), _objets.end(), &v) == _objets.end()) {
		_objets.push_back(&v);
		v._parent = 0;
		v.definirCouche(this->couche());
		v._parent = this;
	}
}

void VueInterface::supprimerEnfant(VueInterface &v) {
	std::list<VueInterface *>::iterator it = std::find(_objets.begin(), _objets.end(), &v);
	if(it != _objets.end()) {
		(*it)->_parent = 0;
		_objets.erase(it);
	}
}

void VueInterface::definirParent(VueInterface *v) {
	if(_parent) {
		_parent->supprimerEnfant(*this);
	}
	if(v) {
		v->ajouterEnfant(*this);
		this->definirCouche(this->couche());
	}
	_parent = v;
}


void VueInterface::rendreLaMain(VueInterface */*prochaine*/) {
	
}

void VueInterface::prendreLaMain(VueInterface */*ancienne*/) {
	
}

void VueInterface::definirVueActuelle(VueInterface *vue) {
	if(_vueActuelle == vue)
		return;
	if(_vueActuelle) {
		VueInterface *vA = _vueActuelle;
		_vueActuelle = 0;
		vA->rendreLaMain(vue);
	}
	if(vue) {
		vue->prendreLaMain(_vueActuelle);
	}
	_vueActuelle = vue;
	Session::reinitialiserEvenementsClavier();
}

VueInterface *VueInterface::sousVueAvecTag(tag_t t) {
	for(std::list<VueInterface *>::iterator i = _objets.begin(); i != _objets.end(); ++i) {
		if((*i)->tag() == t) {
			return *i;
		}
	}
	
	return 0;
}

ControleInterface *VueInterface::controleAvecTag(tag_t t) {
	return dynamic_cast<ControleInterface *>(this->sousVueAvecTag(t));
}

void VueInterface::gestionGestionClavier() {
	if(this->masque() || !this->actif())
		return;
	
	for(std::list<VueInterface *>::iterator i = _objets.begin(); i != _objets.end(); ++i) {
		(*i)->gestionGestionClavier();
	}
	bool focus = !VueInterface::vueActuelle() || !VueInterface::vueActuelle()->premierParentFocus() || this->appartientDescendence(VueInterface::vueActuelle()->premierParentFocus());
	
	if((!this->necessiteFocusClavier() || VueInterface::vueActuelle() == this) && (!VueInterface::vueActuelle() || VueInterface::vueActuelle() == this || focus))
		this->gestionClavier();
}

void VueInterface::definirCouche(int couche) {
	if(couche > this->couche()) {
		for(std::list<VueInterface *>::iterator i = _objets.begin(); i != _objets.end(); ++i) {
			if((*i)->couche() < couche)
				(*i)->definirCouche(couche);
		}
	}
	else if(couche < this->couche()) {
		if(this->parent()) {
			this->parent()->definirCouche(couche);
		}
	}
	_couche = couche;
}
