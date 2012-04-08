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
#include "TableauDeBord.h"
#include "Inventaire.h"

Partie::Partie() : _niveau(0), _joueur(0), _tableauDeBord(0) {
	
}

Partie::~Partie() {
	
}

void Partie::commencer() {
	_joueur = ElementNiveau::elementNiveau<Joueur>(0, 0);
	_niveau = new Niveau(_joueur, "niveau1.xml");

	_joueur->definirNiveau(_niveau);
	_joueur->definirPosition(Coordonnees(200, 200));
	_tableauDeBord = new TableauDeBord(_joueur);

	while(Session::boucle(VITESSE_RAFRAICHISSEMENT, !Session::evenement(Session::QUITTER))) {
		Ecran::definirPointeurAffiche(false);
		Ecran::effacer();
		if(Session::evenement(Session::T_i)) {
			_joueur->definirInventaireAffiche(!_joueur->inventaireAffiche());
			Session::reinitialiser(Session::T_i);
		}
		if(Session::evenement(Session::T_r)) {
			this->reinitialiser();
			Session::reinitialiser(Session::T_r);
		}
		if(Session::evenement(Session::T_e)) {
			Editeur *e = Editeur::editeur();
			
			e->editerNiveau("niveau1.xml");
			
			delete Editeur::editeur();
		}
				
		_niveau->animer(1 / Ecran::frequenceInstantanee());
		this->afficher();
		
		Ecran::maj();
	}
	
	delete _niveau;
	delete _tableauDeBord;
	delete _joueur;
	_niveau = 0;
	_joueur = 0;
	_tableauDeBord = 0;
}

void Partie::afficher() {
	_niveau->afficher();
	if(_joueur->inventaireAffiche()) {
		_joueur->inventaire().afficher();
	}
	_tableauDeBord->afficher();
}

void Partie::reinitialiser() {
	delete _niveau;
	_niveau = new Niveau(_joueur, "niveau1.xml");
	_joueur->definirNiveau(_niveau);
	_joueur->definirPosition(Coordonnees());
}
