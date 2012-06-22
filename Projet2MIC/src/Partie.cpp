//
//  Partie.cpp
//  Projet2MIC
//
//  Créé par Marc Promé et Rémi Saurel.
//  Ce fichier et son contenu sont librement distribuables, modifiables et utilisables pour toute œuvre non commerciale, à condition d'en citer les auteurs.
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

Partie *Partie::partie() {
	if(_partie == 0)
		_partie = new Partie;
	return _partie;
}

Partie::Partie() : _niveau(0), _joueur(0), _tableauDeBord(0), _marchand(0), _numeroNiveau(1), _niveauTermine(false), _derniereSauvegarde(0) {
	_joueur = ElementNiveau::elementNiveau<Joueur>(false, 0, 0);
	_tableauDeBord = new TableauDeBord(_joueur);
}

void Partie::restaurer(TiXmlElement *sauve) {
	if(sauve != _derniereSauvegarde)
		delete _derniereSauvegarde;
	
	_joueur->renaitre();
	if(sauve) {
		sauve->FirstChildElement("Niveau")->Attribute("numero", &_numeroNiveau);
	}
	
	delete _niveau;
	_niveau = new Niveau(_joueur, "niveau" + nombreVersTexte(_numeroNiveau) + ".xml");
	
	_joueur->definirNiveau(_niveau);

	if(sauve) {
		_joueur->restaurer(sauve);
	}
	
	_derniereSauvegarde = sauve;
}

Partie::~Partie() {
	delete _niveau;
	delete _tableauDeBord;
	delete _joueur;
	delete _derniereSauvegarde;
	
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

TiXmlElement *Partie::charger(Image &fond, Shader const &s) {
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
	
	slot = m.afficher(0, fond, s);
	
	if(slot == elements.size())
		return 0;
	while(!slots[slot]) {
		++slot;
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
	Image *apercu = 0;
	
	bool continuer = true;
	while(Session::boucle(FREQUENCE_RAFRAICHISSEMENT, continuer)) {
		delete apercu;
		apercu = 0;
		
		Ecran::definirPointeurAffiche(_joueur->inventaireAffiche());
		Ecran::definirPointeur(0);
		
		Audio::audio_t musique = _niveau->musique();
		Audio::definirMusique(musique);
		Audio::jouerMusique();
		
		Ecran::effacer();
		_niveau->animer();
		this->afficher();
		Ecran::finaliser();
		
		if(_joueur->mortTerminee()) {
			charge = this->mortJoueur(continuer);
			if(!continuer) {
				break;
			}
			else {
				continue;
			}
		}
		else if(_niveauTermine) {
			++_numeroNiveau;
			_niveauTermine = false;
			for(InventaireJoueur::iterator i = static_cast<InventaireJoueur *>(_joueur->inventaire())->debut(); i != static_cast<InventaireJoueur *>(_joueur->inventaire())->fin(); ++i) {
				if(*i && (*i)->index() == 666) {
					_joueur->inventaire()->supprimerObjet(*i);
					break;
				}
			}
			this->restaurer(this->sauvegarde());
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
		if(Session::evenement(Session::T_ENTREE)) {
			_joueur->definirInvicible(!_joueur->invincible());
			Session::reinitialiser(Session::T_ENTREE);
		}
		if(Session::evenement(Session::T_ESC)) {
			if(!apercu)
				apercu = Ecran::apercu();
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
					charge = this->charger(*apercu);
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
			if(!_joueur->inventaire()->gestionEvenements()) {
				_joueur->definirInventaireAffiche(false);
			}
		}
		
		if(Session::evenement(Session::QUITTER)) {
			continuer = false;
		}
		
		if(!continuer) {
			if(!apercu)
				apercu = Ecran::apercu();

			std::vector<Unichar> e;
			e.push_back("Quitter");
			Menu m("Quitter sans sauvegarder ?", e, "Retour");
			if(m.afficher(0, *apercu) == 1) {
				continuer = true;
				delete charge;
				charge = 0;
			}
		}

		Ecran::maj();
	}

	delete apercu;
	delete menu;

	return charge;
}

void Partie::terminerNiveau() {
	_niveauTermine = true;
}

bool Partie::niveauTermine() const {
	return _niveauTermine;
}

void Partie::afficher() {
	_niveau->afficher();
	
	if(_joueur->inventaireAffiche()) {
		_joueur->inventaire()->afficher();
	}
	
	if(!_joueur->mortTerminee())
		_tableauDeBord->afficher();
}

void Partie::reinitialiser() {
	this->restaurer(_derniereSauvegarde);
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
			
			continuer &= _joueur->inventaire()->gestionEvenements();
			continuer &= _marchand->inventaire()->gestionEvenements();
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
		selection = menu.afficher(0, *ap, sMort, false);
	
		if(selection == 0) {
			this->reinitialiser();
			continuer = true;
			break;
		}
		else if(selection == 1) {
			retour = this->charger(*ap);
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
