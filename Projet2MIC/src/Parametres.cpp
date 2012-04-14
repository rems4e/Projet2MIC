//
//  Parametres.cpp
//  Projet2MIC
//
//  Created by Rémi Saurel on 13/04/12.
//  Copyright (c) 2012 Rémi Saurel. All rights reserved.
//

#include "Parametres.h"

namespace Parametres {
	void charger();
	void enregistrer();
	void nettoyer();
	
	Session::evenement_t _evenementsAction[nbActions];
}

void Parametres::charger() {
	for(action_t a = premiereAction; a != nbActions; ++a)
		_evenementsAction[a] = Session::aucunEvenement;
	
	_evenementsAction[depBas] = Session::T_BAS;
	_evenementsAction[depDroite] = Session::T_DROITE;
	_evenementsAction[depGauche] = Session::T_GAUCHE;
	_evenementsAction[depHaut] = Session::T_HAUT;

	// ZQSD
	/*_evenementsAction[depBas] = Session::T_s;
	_evenementsAction[depDroite] = Session::T_d;
	_evenementsAction[depGauche] = Session::T_q;
	_evenementsAction[depHaut] = Session::T_z;*/
}

void Parametres::enregistrer() {
	
}

void Parametres::nettoyer() {
	Parametres::enregistrer();
}

Session::evenement_t Parametres::evenementAction(action_t action) {
	return _evenementsAction[action];
}

void Parametres::definirEvenementAction(action_t action, Session::evenement_t e) {
	for(action_t a = premiereAction; a != nbActions; ++a) {
		if(a == action)
			continue;
		if(_evenementsAction[a] == e)
			_evenementsAction[a] = Session::aucunEvenement;
	}
	
	_evenementsAction[action] = e;
}
