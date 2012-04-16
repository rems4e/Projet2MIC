//
//  Parametres.cpp
//  Projet2MIC
//
//  Created by Rémi Saurel on 13/04/12.
//  Copyright (c) 2012 Rémi Saurel. All rights reserved.
//

#include "Parametres.h"
#include "Image.h"
#include "Ecran.h"
#include "Menu.h"

namespace Parametres {
	void charger();
	void enregistrer();
	void nettoyer();
	
	void video(Image const &fond);
	void audio(Image const &fond);
	void controles(Image const &fond);
	void definirEvenementAction(action_t action, Session::evenement_t e);

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

void Parametres::editerParametres(Image const &fond) {
	std::vector<Unichar> elements;
	elements.push_back("Réglages vidéos");
	elements.push_back("Réglages audios");
	elements.push_back("Réglages contrôles");
	Menu menu("Réglages", elements);
	
	index_t selection = 0;
	do {
		selection = menu.afficher(selection, fond);
		switch(selection) {
			case 0:
				Parametres::video(fond);
				break;
			case 1:
				Parametres::audio(fond);
				break;
			case 2:
				Parametres::controles(fond);
				break;
		}
	} while(selection != elements.size());
}

void Parametres::video(Image const &fond) {
	std::cout << "vidéo" << std::endl;
}

void Parametres::audio(Image const &ond) {
	std::cout << "audio" << std::endl;
}

void Parametres::controles(Image const &fond) {
	std::cout << "contrôles" << std::endl;
}


