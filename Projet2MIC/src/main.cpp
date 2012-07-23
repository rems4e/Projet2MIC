//
//  main.cpp
//  Projet2MIC
//
//  Créé par Marc Promé et Rémi Saurel.
//  Ce fichier et son contenu sont librement distribuables, modifiables et utilisables pour toute œuvre non commerciale, à condition d'en citer les auteurs.
//


#include "Session.h"
#include "Shader.h"
#include "Menu.h"
#include "Partie.h"
#include "Audio.h"
#include "Image.h"
#include "tinyxml.h"
#include "Editeur.h"

#if defined(__WIN32__)
namespace WinWin {
#include <windows.h>
}
#elif defined(__LINUX__)
#include <cstdlib>
#elif defined(__MACOSX__)
#include <cstdlib>
#endif

struct AfficheurMaj {
	AfficheurMaj(bool maj) : _maj(maj), _txt() {
		
	}
	
	Rectangle afficher() const {
		Unichar txt;
#if defined(DEVELOPPEMENT)
		txt += TRAD("gen Version de développement") + "\n";
#endif
		txt += std::string("v. ") + Parametres::versionTexte();
		if(_maj)
			txt += "\n" + TRAD("gen Mise à jour disponible !") + "\n" + TRAD("gen Cliquez pour l'afficher.");

		_txt.definir(txt);
		_txt.definir(POLICE_NORMALE, 16 * Ecran::echelleMin());
		_txt.definir(Couleur::blanc);
		_cadre.definirDimensions(_txt.dimensions() + Coordonnees(10, 10));
		_cadre.definirOrigine(Coordonnees(0, Ecran::hauteur() - _cadre.hauteur));
		
		Ecran::afficherRectangle(_cadre, Couleur(0, 0, 0, 128));
		_txt.afficher(_cadre.origine() + Coordonnees(5, 5));
		
		return _cadre;
	}
	
	bool gestionEvenements() const {
		if(!_maj)
			return true;
		
		if(Session::souris() < _cadre && Session::evenement(Session::B_GAUCHE)) {
			Session::reinitialiser(Session::B_GAUCHE);

			std::string destination = Parametres::URLMaj();
#if defined(__WIN32__)
			
			WinWin::ShellExecute(NULL, "open", destination.c_str(), NULL, NULL, SW_SHOWNORMAL);
			
#elif defined(__LINUX__)
			
			system(("xdg-open " + destination).c_str());
			
#elif defined(__MACOSX__)
			
			system(("/usr/bin/open " + destination).c_str());
			
#endif
			return false;
		}
		
		return true;
	}
	
	void definirMaj(bool m) {
		_maj = m;
	}
	
private:
	bool _maj;
	mutable Rectangle _cadre;
	mutable Texte _txt;
};

static void jeu();

#ifdef __cplusplus
extern "C"
#endif

int main(int, char **) {	
	// Initialise tous les sous-programmes requis pour le jeu : affichage (images, textes, écran…), audio, événements, paramètres utilisateur…
	Session::initialiser();
	
	jeu();
	
	// Passé ce point, toutes les ressources doivent être déjà désallouées, car à cause du système de gestion de ressources partagées, leur désallocation après le nettoyage entrainerait un plantage du programme.
	Session::nettoyer();
	
	return 0;
}

static void jeu() {
	Audio::audio_t musique = Audio::chargerMusique(Session::cheminRessources() + "menu.mp3");
	
	Shader sFond(Session::cheminRessources() + "aucun.vert", Session::cheminRessources() + "menuPrincipal.frag");
	
	Image fond(Session::cheminRessources() + "tex0.jpg");
	index_t selection = 0;
	
	AfficheurMaj afficheurMaj(false);
	TiXmlElement *charge = 0;
	std::vector<Unichar> elements;

	do {		
		if(charge) {
			Partie *nouvellePartie = Partie::partie();
			nouvellePartie->restaurer(charge);
			charge = nouvellePartie->commencer();
			delete nouvellePartie;
		}
		else {
			Audio::definirMusique(musique);
			Audio::jouerMusique();
			
			afficheurMaj.definirMaj(Parametres::rechercherMaj() && Parametres::majDisponible());
	
			elements.clear();
			elements.push_back(TRAD("gen Nouvelle partie"));
			elements.push_back(TRAD("gen Charger une partie"));
			elements.push_back(TRAD("gen Réglages"));
			elements.push_back(TRAD("gen Crédits"));
			elements.push_back(TRAD("gen Éditeur de niveaux"));
			elements.push_back(TRAD("gen Quitter"));
			Menu menu(TRAD("gen Menu principal"), elements, "");

			selection = menu.afficher(0, fond, sFond, 0, afficheurMaj);
			if(selection == 0) {
				Partie *nouvellePartie = Partie::partie();
				nouvellePartie->restaurer(0);
				charge = nouvellePartie->commencer();
				delete nouvellePartie;
			}
			else if(selection == 1) {
				charge = Partie::charger(fond, sFond, 0);
			}
			else if(selection == 2) {
				Parametres::editerParametres(fond, sFond);
			}
			else if(selection == 3) {
				Parametres::afficherCredits(fond, sFond);
			}
			else if(selection == 4) {
				Editeur *e = Editeur::editeur();
				e->ouvrirEditeur(fond, sFond);
				delete Editeur::editeur();
			}
		}
	} while(selection != elements.size() - 1);
	
	delete charge;
	
	Audio::libererSon(musique);
}
