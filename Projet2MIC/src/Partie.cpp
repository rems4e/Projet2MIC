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
#include "Menu.h"

Partie *Partie::_partie = 0;

Partie *Partie::creerPartie() throw(Partie::Exc_PartieDejaCreee) {
	if(_partie != 0) {
		delete _partie;
		throw Exc_PartieDejaCreee();
	}
	
	_partie = new Partie;
	
	return Partie::partie();
}

/*Partie *Partie::creerPartie(std::string const &sauvegarde) throw(Partie::Exc_PartieDejaCreee) {
	if(_partie != 0) {
		delete _partie;
		throw Exc_PartieDejaCreee();
	}
	
	_partie = new Partie(sauvegarde);

	return Partie::partie();
}

Partie *Partie::creerPartie(int numeroSecteur) throw(Partie::Exc_PartieDejaCreee) {
	if(_partie != 0) {
		delete _partie;
		throw Exc_PartieDejaCreee();
	}

	_partie = new Partie(numeroSecteur);
	
	return Partie::partie();
}*/

Partie *Partie::partie() {
	return _partie;
}

Partie::Partie() : _niveau(0), _joueur(0), _tableauDeBord(0) {
	
}

Partie::~Partie() {
	if(_partie != this)
		throw Exc_PartieDejaCreee();
	_partie = 0;
}

void Partie::commencer() {
	_joueur = ElementNiveau::elementNiveau<Joueur>(0, 0);
	_niveau = new Niveau(_joueur, "niveau1.xml");

	_joueur->definirNiveau(_niveau);
	_tableauDeBord = new TableauDeBord(_joueur);
		
	Menu *menu = 0;
	{
		std::vector<Unichar> elem;
		elem.push_back("Recharger le niveau");
		elem.push_back("Ouvrir l'éditeur");
		elem.push_back("Quitter");
		
		menu = new Menu("Menu principal", elem);
	}
	
	bool continuer = true;
	while(Session::boucle(VITESSE_RAFRAICHISSEMENT, continuer)) {
		Ecran::definirPointeurAffiche(_joueur->inventaireAffiche());
		Ecran::definirPointeur(0);
		Ecran::effacer();
		_niveau->animer(1 / Ecran::frequenceInstantanee());
		this->afficher();
		Ecran::finaliser();
		
		if(Session::evenement(Session::T_i)) {
			_joueur->definirInventaireAffiche(!_joueur->inventaireAffiche());
			Session::reinitialiser(Session::T_i);
		}
		if(Session::evenement(Session::T_ESC)) {
			index_t retour = menu->afficher();
			if(retour == 0) {
				this->reinitialiser();
				Session::reinitialiser(Session::T_r);
			}
			else if(retour == 1) {
				Editeur *e = Editeur::editeur();
				
				e->editerNiveau("niveau1.xml");
				
				delete Editeur::editeur();
			}
			else if(retour == 2) {
				continuer = false;
			}
		}
		if(_joueur->inventaireAffiche()) {
			_joueur->inventaire()->gestionEvenements();
		}
		
		if(Session::evenement(Session::QUITTER)) {
			continuer = false;
		}

		Ecran::maj();
	}
	
	delete _niveau;
	delete _tableauDeBord;
	delete _joueur;
	delete menu;
	_niveau = 0;
	_joueur = 0;
	_tableauDeBord = 0;
}

void Partie::afficher() {
	_niveau->afficher();
	if(_joueur->inventaireAffiche()) {
		_joueur->inventaire()->afficher();
	}
	_tableauDeBord->afficher();
}

void Partie::reinitialiser() {
	delete _niveau;
	_niveau = new Niveau(_joueur, "niveau1.xml");
	_joueur->definirNiveau(_niveau);
	_joueur->definirInventaireAffiche(false);
	_joueur->inventaire()->vider();
}

Rectangle Partie::zoneJeu() const {
	if(!(_joueur->inventaireAffiche())) {
		return Rectangle(Coordonnees(), Coordonnees(Ecran::largeur(), Ecran::hauteur() - _tableauDeBord->hauteur()));
	}
	else {
		return Rectangle(Coordonnees(Ecran::largeur() / 2, 0), Coordonnees(Ecran::largeur(), Ecran::hauteur() - _tableauDeBord->hauteur()));
	}
}

