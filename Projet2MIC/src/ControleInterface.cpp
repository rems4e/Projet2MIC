/*
 *  ControleInterface.cpp
 *  Jeu C++
 *
 *  Created by Rémi on 12/01/11.
 *  Copyright 2011 Rémi Saurel. All rights reserved.
 *
 */

#include "ControleInterface.h"

ControleInterface::ControleInterface(Rectangle const &cadre, Couleur const &couleurFond) : VueInterface(cadre, couleurFond), _valeurIndeterminee(false), _action(0), _actionP(0), _parametreAction(0), _raccourci(Session::aucunEvenement), _modificateurRaccourci(), _desc() {
	
}

ControleInterface::~ControleInterface() { }

VueInterface *ControleInterface::gestionClic() {
	bool clic = _clic;
	VueInterface *a = VueInterface::gestionClic();
	
	if(clic && !_clic && this->cadreAbsolu().contientPoint(Session::souris())) {
		this->executerAction();
	}
	
	return a;
}

void ControleInterface::executerAction() {
	if(_action) {
		_action();
	}
	else if(_actionP) {
		_actionP(this, _parametreAction);
	}
}

Unichar &ControleInterface::description() const {
	if(_raccourci == Session::aucunEvenement) {
		return _desc = this->VueInterface::description();
	}
	else if(_modificateurRaccourci == Session::modificateur_touche_t()) {
		if(this->VueInterface::description().empty())
			return _desc = Unichar("(") + Session::transcriptionEvenement(_raccourci, true) + ")";
		else
			return _desc = this->VueInterface::description() + " (" + Session::transcriptionEvenement(_raccourci, true) + ")";
	}
	else {
		if(this->VueInterface::description().empty())
			return _desc = Unichar("(") + Session::transcriptionModificateur(_modificateurRaccourci, true) + " + " + Session::transcriptionEvenement(_raccourci, true) + ")";
		else
			return _desc = this->VueInterface::description() + " (" + Session::transcriptionModificateur(_modificateurRaccourci, true) + " + " + Session::transcriptionEvenement(_raccourci, true) + ")";
	}
}

void ControleInterface::gestionClavier() {
	if(!_clic) {
		if(Session::modificateurTouches() == _modificateurRaccourci && _raccourci != Session::aucunEvenement && Session::evenement(_raccourci)) {
			this->executerAction();
		}
	}
}
