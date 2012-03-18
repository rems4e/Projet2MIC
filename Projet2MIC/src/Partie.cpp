//
//  Partie.cpp
//  Projet2MIC
//
//  Created by Rémi Saurel on 07/02/12.
//  Copyright (c) 2012 Rémi Saurel. All rights reserved.
//

#include "Partie.h"
#include "Session.h"
#include "Ecran.h"
#include "Niveau.h"
#include "Personnage.h"
#include "Joueur.h"
#include "Editeur.h"

Partie::Partie() : _niveau(0), _joueur(0) {
	
}

Partie::~Partie() {
	
}

void Partie::commencer() {
	int tabId[] = {0, 1, 2, 3, 4};
	int idId = 0;
	_joueur = ElementNiveau::elementNiveau<Joueur>(0, tabId[idId]);
	_niveau = new Niveau(_joueur, "niveau1.xml");

	_joueur->definirNiveau(_niveau);
	_joueur->definirPosition(Coordonnees(200, 200));
	
	Ecran::definirPointeurAffiche(true);
	while(Session::boucle(60, !Session::evenement(Session::QUITTER))) {
		Ecran::effacer();
		if(Session::evenement(Session::T_r)) {
			this->reinitialiser();
			Session::reinitialiser(Session::T_r);
		}
		if(Session::evenement(Session::T_t)) {
			idId = (idId + 1) % (sizeof(tabId) / sizeof(*tabId));
			Joueur *j = ElementNiveau::elementNiveau<Joueur>(_niveau, tabId[idId]);
			_niveau->definirJoueur(j);
			delete _joueur;
			_joueur = j;
		}
		else if(Session::evenement(Session::T_e)) {
			Editeur *e = Editeur::editeur();
			
			e->editerNiveau("niveau1.xml");
			
			delete Editeur::editeur();
		}
				
		_niveau->animer(1 / Ecran::frequenceInstantanee());
		this->afficher();
		
		Ecran::maj();
	}
	
	delete _niveau;
	delete _joueur;
	_niveau = 0;
	_joueur = 0;
}

void Partie::afficher() {
	_niveau->afficher();
	_niveau->Niveau::afficher();
}

void Partie::reinitialiser() {
	delete _niveau;
	_niveau = new Niveau(_joueur, "niveau1.xml");
	_joueur->definirNiveau(_niveau);
	_joueur->definirPosition(Coordonnees());
}