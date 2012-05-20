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
#include "tinyxml.h"

Partie *Partie::_partie = 0;

Partie *Partie::creerPartie(TiXmlElement *sauve) throw(Partie::Exc_PartieDejaCreee) {
	if(_partie != 0) {
		delete _partie;
		throw Exc_PartieDejaCreee();
	}
	
	if(sauve)
		_partie = new Partie(sauve);
	else
		_partie = new Partie;
	
	return Partie::partie();
}

Partie *Partie::partie() {
	return _partie;
}

Partie::Partie() : _niveau(0), _joueur(0), _tableauDeBord(0), _marchand(0), _numeroNiveau(1) {
	_joueur = ElementNiveau::elementNiveau<Joueur>(false, 0, 0);
	_niveau = new Niveau(_joueur, "niveau" + nombreVersTexte(_numeroNiveau) + ".xml");
	
	_joueur->definirNiveau(_niveau);
	_tableauDeBord = new TableauDeBord(_joueur);
}

Partie::Partie(TiXmlElement *sauve) : _niveau(0), _joueur(0), _tableauDeBord(0), _marchand(0), _numeroNiveau(1) {
	sauve->FirstChildElement("Niveau")->Attribute("numero", &_numeroNiveau);
	
	_joueur = ElementNiveau::elementNiveau<Joueur>(false, 0, 0);
	
	_niveau = new Niveau(_joueur, "niveau" + nombreVersTexte(_numeroNiveau) + ".xml");
	
	_joueur->definirNiveau(_niveau);
	_joueur->restaurer(sauve);

	_tableauDeBord = new TableauDeBord(_joueur);
}

Partie::~Partie() {
	delete _niveau;
	delete _tableauDeBord;
	delete _joueur;
	
	if(_partie == this)
		_partie = 0;
}

void Partie::sauvegarder(Image *i) {
	TiXmlDocument sauves(Session::cheminRessources() + "parties.xml");
	std::vector<Unichar> elements;
	
	TiXmlElement *slots[4] = {0, 0, 0, 0};
	if(!sauves.LoadFile()) {
		char const *doc = 
		"<?xml version=\"1.0\" standalone='no' >\n"
		"<Slots>\n"
		"</Slots>";
		
		sauves.Parse(doc);
	}
	
	TiXmlElement *pp = sauves.FirstChildElement("Slots");
	for(index_t pos = 1; pos <= 4; ++pos) {
		slots[pos - 1] = pp->FirstChildElement("Slot" + nombreVersTexte(pos));
		if(slots[pos - 1])
			elements.push_back("Slot " + nombreVersTexte(pos) + " (occupé)");
		else
			elements.push_back("Slot " + nombreVersTexte(pos) + " (vide)");
	}
	
	index_t slot = 0;
	bool choix;
	do {
		choix = true;
		Menu m("Choisissez une sauvegarde :", elements);
		
		slot = m.afficher(0, *i);
		
		if(slot == 4)
			return;
		
		if(slots[slot]) {
			std::vector<Unichar> elements;
			elements.push_back("Remplacer");
			Menu m("Remplacer la sauvegarde ?", elements, "Annuler");
			
			index_t val = m.afficher(0, *i);
			if(val == 1) {
				choix = false;
			}
			else
				pp->RemoveChild(slots[slot]);
		}
	} while(!choix);
	
	TiXmlElement s("Slot" + nombreVersTexte(slot + 1));
	pp->InsertEndChild(s);
	slots[slot] = pp->FirstChildElement("Slot" + nombreVersTexte(slot + 1));
	
	TiXmlElement *sauve = this->sauvegarde();
	slots[slot]->InsertEndChild(*sauve);
	delete sauve;
	
	sauves.SaveFile();
}

TiXmlElement *Partie::sauvegarde() {
	TiXmlElement *sauve = new TiXmlElement("Partie");
	
	TiXmlElement niveau("Niveau");
	niveau.SetAttribute("numero", _numeroNiveau);
	sauve->InsertEndChild(niveau);
	
	TiXmlElement *perso = this->joueur()->sauvegarde();
	sauve->InsertEndChild(*perso);
	delete perso;
	
	return sauve;
}

TiXmlElement *Partie::charger(Image *fond, Shader const &s) {
	TiXmlDocument sauves(Session::cheminRessources() + "parties.xml");
	std::vector<Unichar> elements;
	
	TiXmlElement *slots[4] = {0, 0, 0, 0};
	if(!sauves.LoadFile()) {
		char const *doc = 
		"<?xml version=\"1.0\" standalone='no' >\n"
		"<Slots>\n"
		"</Slots>";
		
		sauves.Parse(doc);
	}
	
	TiXmlElement *pp = sauves.FirstChildElement("Slots");
	for(index_t pos = 1; pos <= 4; ++pos) {
		slots[pos - 1] = pp->FirstChildElement("Slot" + nombreVersTexte(pos));
		if(slots[pos - 1])
			elements.push_back("Slot " + nombreVersTexte(pos));
	}
	
	if(elements.empty())
		return 0;
	
	index_t slot = 0;
	Menu m("Choisissez une sauvegarde :", elements);
	
	slot = m.afficher(0, *fond, s);
	
	if(slot == elements.size())
		return 0;
	while(!slots[slot]) {
		++slot;
		std::cout << slot << std::endl;
	}
	
	return static_cast<TiXmlElement* >(slots[slot]->FirstChildElement("Partie")->Clone());
}

#define REGLAGES 0
#define SAUVE 1
#define CHARG 2
#define QUIT 3

struct triPotions_t {
	bool operator()(ObjetInventaire *p1, ObjetInventaire *p2) {
		return p1->vie() < p2->vie();
	}
};

TiXmlElement *Partie::commencer() {	
	Menu *menu = 0;
	{
		std::vector<Unichar> elem;
		elem.push_back("Réglages");
		elem.push_back("Sauvegarder la partie");
		elem.push_back("Charger une partie");
		elem.push_back("Menu principal");
		
		menu = new Menu("Pause", elem);
	}
	TiXmlElement *charge = 0;
	
	bool continuer = true;
	while(Session::boucle(FREQUENCE_RAFRAICHISSEMENT, continuer)) {
		Ecran::definirPointeurAffiche(_joueur->inventaireAffiche());
		Ecran::definirPointeur(0);
		
		Audio::audio_t musique = _niveau->musique();
		Audio::definirMusique(musique);
		Audio::jouerMusique();
		
		Ecran::effacer();
		_niveau->animer();
		this->afficher();
		Ecran::finaliser();
		
		if(_joueur->mort()) {
			charge = this->mortJoueur(continuer);
			if(!continuer) {
				break;
			}
			else {
				continue;
			}
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
				if(selection == REGLAGES) {
					Parametres::editerParametres(*apercu);
				}
				else if(selection == SAUVE) {
					this->sauvegarder(apercu);
				}
				else if(selection == CHARG) {
					charge = this->charger(apercu);
					if(charge) {
						continuer = false;
					}
				}
				else if(selection == -1) {
					this->reinitialiser();
				}
				else if(selection == QUIT) {
					continuer = false;
				}
			} while(selection == 0);
			delete apercu;
		}
		else if(Session::evenement(Session::T_e)) {
			Editeur *e = Editeur::editeur();
			e->editerNiveau("niveau" + nombreVersTexte(_numeroNiveau) + ".xml");
			delete Editeur::editeur();
			
			this->reinitialiser();
		}
		else if(Session::evenement(Parametres::evenementAction(Parametres::remplirVie))) {
			InventaireJoueur *j = static_cast<InventaireJoueur *>(_joueur->inventaire());
			std::list<ObjetInventaire *> potionsVie;
			for(InventaireJoueur::iterator i = j->debut(); i != j->fin(); ++i) {
				if(*i && (*i)->categorieObjet() == ObjetInventaire::potion) {
					potionsVie.push_back(*i);
				}
			}
			
			potionsVie.sort(triPotions_t());

			while(_joueur->vieActuelle() < _joueur->vieTotale() && potionsVie.size()) {
				int diff = _joueur->vieTotale() - _joueur->vieActuelle();
				int v = std::min(potionsVie.front()->vie(), diff);
				_joueur->modifierVieActuelle(v);
				potionsVie.front()->supprimerVie(v);
				if(potionsVie.front()->vie() == 0) {
					j->supprimerObjet(potionsVie.front());
					delete potionsVie.front();
					
					potionsVie.pop_front();
				}
			}
				
			Session::reinitialiser(Parametres::evenementAction(Parametres::remplirVie));
		}
		if(_joueur->inventaireAffiche()) {
			_joueur->inventaire()->gestionEvenements();
		}
		
		if(Session::evenement(Session::QUITTER)) {
			continuer = false;
		}
		
		Ecran::maj();
	}

	delete menu;
	
	return charge;
}

void Partie::afficher() {
	_niveau->afficher();
	
	if(_joueur->inventaireAffiche()) {
		_joueur->inventaire()->afficher();
	}
	
	if(!_joueur->mort())
		_tableauDeBord->afficher();
}

void Partie::reinitialiser() {
	_joueur->renaitre();
	
	delete _niveau;
	_niveau = new Niveau(_joueur, "niveau" + nombreVersTexte(_numeroNiveau) + ".xml");
	_joueur->definirNiveau(_niveau);
	_joueur->definirInventaireAffiche(false);
	_joueur->inventaire()->vider();
}

Rectangle Partie::zoneJeu() const {
	if(_marchand) {
		Rectangle r = Rectangle::aucun;
		r.hauteur = Ecran::hauteur() - _tableauDeBord->hauteur();
		return r;
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

TiXmlElement *Partie::mortJoueur(bool &continuer) {
	float const dureeTransition = 3.0f;
	Shader sMort(Session::cheminRessources() + "aucun.vert", Session::cheminRessources() + "mort.frag");
	sMort.definirParametre("duree", dureeTransition);

	std::vector<Unichar> elem;
	elem.push_back("Recommencer");
	elem.push_back("Charger une partie");
	elem.push_back("Menu principal");
		
	Menu menu("T'es mort !", elem, "");
	Image *ap = Ecran::apercu();
	
	
	index_t selection;
	
	TiXmlElement *retour = 0;

	do {
		selection = menu.afficher(0, *ap, sMort);
	
		if(selection == 0) {
			this->reinitialiser();
			continuer = true;
			break;
		}
		else if(selection == 1) {
			retour = this->charger(ap);
			continuer = !retour;
		}
		else if(selection == 2) {
			continuer = false;
			break;
		}
	} while(!retour);
	
	delete ap;
	
	return retour;
}
