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

class VueMarchand : public VueInterface {
public:
	VueMarchand() : VueInterface(Rectangle(0, 0, Ecran::largeur(), Ecran::hauteur())) {
		
	}
protected:
	void dessiner() override;
	void gestionClavier() override;
};

Partie *Partie::_partie = 0;

Partie *Partie::partie() {
	if(_partie == 0)
		_partie = new Partie;
	return _partie;
}

Partie::Partie() : VueInterface(Rectangle(0, 0, Ecran::largeur(), Ecran::hauteur())), _niveau(0), _joueur(0), _tableauDeBord(0), _marchand(0), _numeroNiveau(1), _niveauTermine(false), _derniereSauvegarde(0) {
	_joueur = ElementNiveau::elementNiveau<Joueur>(false, 0, 0);
	_tableauDeBord = new TableauDeBord(_joueur);
	this->ajouterEnfant(*_tableauDeBord);
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
	this->supprimerEnfant(*_tableauDeBord);
	delete _tableauDeBord;
	delete _joueur;
	delete _derniereSauvegarde;
	
	_partie = 0;
}

void Partie::sauvegarder(Image &fond) {
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
			elements.push_back(TRAD("partie Slot %1 (occupé)", pos));
		else
			elements.push_back(TRAD("partie Slot %1 (libre)", pos));
	}
	
	index_t slot = 0;
	bool choix;
	do {
		choix = true;
		Menu m(TRAD("partie Choisissez une sauvegarde :"), elements);
		
		slot = m.afficher(0, fond);
		
		if(slot == 4)
			return;
		
		if(slots[slot]) {
			std::vector<Unichar> elements;
			elements.push_back(TRAD("partie Remplacer"));
			Menu m(TRAD("partie Remplacer la sauvegarde ?"), elements, TRAD("partie Annuler"));
			
			index_t val = m.afficher(0, fond);
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

TiXmlElement *Partie::charger(Image &fond, Shader const &s, horloge_t tempsInitial) {
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
			elements.push_back(TRAD("partie Slot %1", pos));
	}
	
	if(elements.empty())
		return 0;
	
	index_t slot = 0;
	Menu m(TRAD("partie charge Choisissez une sauvegarde :"), elements);
	
	slot = m.afficher(0, fond, s, tempsInitial);
	
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
	_charge = 0;
	_apercu = 0;
	
	Session::ajouterVueFenetre(this);

	delete _apercu;

	return _charge;
}

void Partie::gestionClavier() {		
	Audio::audio_t musique = _niveau->musique();
	Audio::definirMusique(musique);
	Audio::jouerMusique();
	
	delete _apercu;
	_apercu = 0;

	bool continuer = true;
	_tableauDeBord->definirMasque(_joueur->mortTerminee());
	if(_joueur->mortTerminee()) {
		_charge = this->mortJoueur(continuer);
		if(!continuer) {
			Session::supprimerVueFenetre();
		}
		return;
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
		if(!_apercu)
			_apercu = Ecran::apercu();
		index_t selection = 0;
		do {
			std::vector<Unichar> elem;
			elem.push_back(TRAD("gen Réglages"));
			elem.push_back(TRAD("partie Sauvegarder la partie"));
			elem.push_back(TRAD("partie Charger une partie"));
			elem.push_back(TRAD("partie Menu principal"));
			
			Menu menu(TRAD("partie Pause"), elem);
			selection = menu.afficher(0, *_apercu);
			if(selection == REGLAGES) {
				Parametres::editerParametres(*_apercu);
			}
			else if(selection == SAUVE) {
				this->sauvegarder(*_apercu);
			}
			else if(selection == CHARG) {
				_charge = this->charger(*_apercu, Shader::flou(1.0), 0);
				if(_charge) {
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
		if(!_apercu)
			_apercu = Ecran::apercu();
		
		std::vector<Unichar> e;
		e.push_back(TRAD("partie Quitter"));
		Menu m(TRAD("partie Quitter sans sauvegarder ?"), e, TRAD("partie Retour"));
		if(m.afficher(0, *_apercu) == 1) {
			continuer = true;
			delete _charge;
			_charge = 0;
		}
		
		if(!continuer) {
			Session::supprimerVueFenetre();
		}
	}
}

void Partie::terminerNiveau() {
	_niveauTermine = true;
}

bool Partie::niveauTermine() const {
	return _niveauTermine;
}

void Partie::dessiner() {
	Ecran::definirPointeurAffiche(_joueur->inventaireAffiche());
	Ecran::definirPointeur(0);

	_niveau->animer();
	_niveau->afficher();
	
	if(_joueur->inventaireAffiche()) {
		_joueur->inventaire()->afficher();
	}
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
		return Rectangle(glm::vec2(0), glm::vec2(Ecran::largeur(), Ecran::hauteur() - _tableauDeBord->hauteur()));
	}
	else {
		return Rectangle(glm::vec2(Ecran::largeur() / 2, 0), glm::vec2(Ecran::largeur(), Ecran::hauteur() - _tableauDeBord->hauteur()));
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

		VueMarchand vue;
		Session::ajouterVueFenetre(&vue);
		
		_marchand->inventaire()->masquer();
		_joueur->inventaire()->masquer();
		_marchand = 0;
	}
}

void VueMarchand::dessiner() {
	Ecran::definirPointeurAffiche(true);
	Ecran::definirPointeur(0);
	
	Partie *partie = Partie::partie();
	partie->joueur()->inventaire()->afficher();
	partie->marchand()->inventaire()->afficher();
}

void VueMarchand::gestionClavier() {
	bool continuer = true;
	Partie *partie = Partie::partie();
	
	continuer &= partie->joueur()->inventaire()->gestionEvenements();
	continuer &= partie->marchand()->inventaire()->gestionEvenements();
	if(Session::evenement(Session::T_ESC)) {
		continuer = false;
	}
	
	if(!continuer) {
		Session::supprimerVueFenetre();
	}
}

TiXmlElement *Partie::mortJoueur(bool &continuer) {
	float const dureeTransition = 3.0f;
	Shader sMort(Session::cheminRessources() + "aucun.vert", Session::cheminRessources() + "mort.frag");
	sMort.definirParametre("duree", dureeTransition);

	std::vector<Unichar> elem;
	elem.push_back(TRAD("partie Recommencer"));
	elem.push_back(TRAD("gen Charger une partie"));
	elem.push_back(TRAD("partie Menu principal"));
		
	Menu menu(TRAD("partie T'es mort !"), elem, "");
	Image *ap = Ecran::apercu();
	
	
	index_t selection;
	
	TiXmlElement *retour = 0;

	horloge_t tempsInitial = horloge();
	do {
		selection = menu.afficher(0, *ap, sMort, tempsInitial);
	
		if(selection == 0) {
			this->reinitialiser();
			continuer = true;
			break;
		}
		else if(selection == 1) {
			retour = this->charger(*ap, sMort, tempsInitial);
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
