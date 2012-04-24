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
#include "Marchand.h"
#include <cmath>

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

Partie::Partie() : _niveau(0), _joueur(0), _tableauDeBord(0), _marchand(0) {
	
}

Partie::~Partie() {
	if(_partie != this)
		throw Exc_PartieDejaCreee();
	_partie = 0;
}

void Partie::commencer() {
	_joueur = ElementNiveau::elementNiveau<Joueur>(false, 0, 0);
	_niveau = new Niveau(_joueur, "niveau1.xml");

	_joueur->definirNiveau(_niveau);
	_tableauDeBord = new TableauDeBord(_joueur);
		
	Menu *menu = 0;
	{
		std::vector<Unichar> elem;
		elem.push_back("Réglages");
		elem.push_back("Recharger le niveau");
		elem.push_back("Ouvrir l'éditeur");
		elem.push_back("Quitter");
		
		menu = new Menu("Menu principal", elem);
	}
	
	bool continuer = true;
	while(Session::boucle(FREQUENCE_RAFRAICHISSEMENT, continuer)) {
		Ecran::definirPointeurAffiche(_joueur->inventaireAffiche());
		Ecran::definirPointeur(0);
		Ecran::effacer();
		_niveau->animer();
		this->afficher();
		Ecran::finaliser();
		
		if(_joueur->mort()) {
			this->mortJoueur();
		}
		
		if(Session::evenement(Parametres::evenementAction(Parametres::afficherInventaire))) {
			if(_joueur->inventaireAffiche()) {
				_joueur->inventaire()->masquer();
			}
			else {
				_joueur->inventaire()->preparationAffichage();
			}
			_joueur->definirInventaireAffiche(!_joueur->inventaireAffiche());
			Session::reinitialiser(Parametres::evenementAction(Parametres::afficherInventaire));
		}
		if(Session::evenement(Session::T_ESC)) {
			Image *apercu = Ecran::apercu();
			index_t selection = 0;
			do {
				selection = menu->afficher(0, *apercu);
				if(selection == 0) {
					Parametres::editerParametres(*apercu);
				}
				else if(selection == 1) {
					this->reinitialiser();
				}
				else if(selection == 2) {
					Editeur *e = Editeur::editeur();
					
					e->editerNiveau("niveau1.xml");
					
					delete Editeur::editeur();
				}
				else if(selection == 3) {
					continuer = false;
				}
			} while(selection == 0);
			delete apercu;
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
/*	static Shader *s = 0;
	static horloge_t temps;
	if(s == 0) {
		temps = horloge();
		s = new Shader(Session::cheminRessources() + "aucun.vert", Session::cheminRessources() + "test.frag");
	}*/
	
	_niveau->afficher();
	/*s->activer();
	s->definirParametre("_temps", horloge() - temps);*/
	Ecran::afficherRectangle(Ecran::ecran(), Couleur(0, 0, 0, 1.0));
	//s->desactiver();
	
	if(_joueur->inventaireAffiche()) {
		_joueur->inventaire()->afficher();
	}

	if(!_joueur->mort())
		_tableauDeBord->afficher();
}

void Partie::reinitialiser() {
	_joueur->renaitre();

	delete _niveau;
	_niveau = new Niveau(_joueur, "niveau1.xml");
	_joueur->definirNiveau(_niveau);
	_joueur->definirInventaireAffiche(false);
	_joueur->inventaire()->vider();
}

Rectangle Partie::zoneJeu() const {
	if(_marchand) {
		return Rectangle::aucun;
	}
	else if(!(_joueur->inventaireAffiche())) {
		return Rectangle(Coordonnees(), Coordonnees(Ecran::largeur(), Ecran::hauteur() - _tableauDeBord->hauteur()));
	}
	else {
		return Rectangle(Coordonnees(Ecran::largeur() / 2, 0), Coordonnees(Ecran::largeur(), Ecran::hauteur() - _tableauDeBord->hauteur()));
	}
}

Joueur *Partie::joueur() {
	return _joueur;
}

Marchand *Partie::marchand() {
	return _marchand;
}

void Partie::definirMarchand(Marchand *m) {
	_marchand = m;
	if(_marchand) {
		_marchand->inventaire()->preparationAffichage();
		_joueur->inventaire()->preparationAffichage();
		bool continuer = true;
		while(Session::boucle(FREQUENCE_RAFRAICHISSEMENT, continuer)) {
			Ecran::definirPointeurAffiche(true);
			Ecran::definirPointeur(0);
			Ecran::effacer();
			
			_joueur->inventaire()->afficher();
			_marchand->inventaire()->afficher();
			_tableauDeBord->afficher();
			
			Ecran::finaliser();
			
			_joueur->inventaire()->gestionEvenements();
			_marchand->inventaire()->gestionEvenements();
			if(Session::evenement(Session::T_ESC)) {
				continuer = false;
			}
			
			Ecran::maj();
		}
		_marchand->inventaire()->masquer();
		_joueur->inventaire()->masquer();
		_marchand = 0;
	}
}

void Partie::mortJoueur() {
	bool continuer = true;
	Image *apercu = Ecran::apercu();
	float const dureeTransition = 3.0f;
	char const * const t = "temps";
	char const * const angle = "angle";
	char const * const position = "position";
	Shader sMort(Session::cheminRessources() + "aucun.vert", Session::cheminRessources() + "mort.frag");
	sMort.definirParametre("duree", dureeTransition);
	
	Shader sTexte(Session::cheminRessources() + "rotation.vert", Session::cheminRessources() + "aucun.frag");
	sTexte.definirParametre("axe", 1.0, 0, 0.0);
	Texte titre("T'es mort !", POLICE_DECO, 42, Couleur::blanc);
	Texte sousTitre("(Esc pour recommencer)", POLICE_DECO, 26, Couleur::blanc);
	
	horloge_t tempsInitial = horloge();
	
	while(Session::boucle(FREQUENCE_RAFRAICHISSEMENT, continuer)) {
		Ecran::definirPointeurAffiche(true);
		Ecran::definirPointeur(0);
		Ecran::effacer();
		
		Shader::flou(1.0).activer();
		apercu->afficher(Coordonnees::zero);
		
		sMort.activer();
		sMort.definirParametre(t, horloge() - tempsInitial);
		Ecran::afficherRectangle(Ecran::ecran(), Couleur::blanc);
		
		Shader::aucun().activer();
		
		sTexte.activer();
		sTexte.definirParametre(angle, std::fmod(horloge() - tempsInitial, float(M_PI * 2)));
		Coordonnees pTitre = Ecran::dimensions() / 2 - titre.dimensions() / 2 - Coordonnees(0, 80);
		sTexte.definirParametre(position, pTitre.x, pTitre.y, 0);
		titre.afficher(pTitre);

		pTitre.x = Ecran::dimensions().x / 2 - sousTitre.dimensions().x / 2;
		sousTitre.afficher(pTitre + Coordonnees(0, titre.dimensions().y + 40));
		Shader::desactiver();
		
		
		
		Ecran::finaliser();
		
		if(Session::evenement(Session::T_ESC)) {
			continuer = false;
		}
		
		Ecran::maj();
	}
	
	Shader::desactiver();

	delete apercu;
	this->reinitialiser();
}
