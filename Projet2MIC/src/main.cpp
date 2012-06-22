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
	std::vector<Unichar> elements;
	elements.push_back("Nouvelle partie");
	elements.push_back("Charger une partie");
	elements.push_back("Réglages");
	elements.push_back("Crédits");
	elements.push_back("Éditeur de niveaux");
	elements.push_back("Quitter");
	Menu menu("Menu principal", elements, "");
	
	Image fond(Session::cheminRessources() + "tex0.jpg");
	index_t selection = 0;
	
	TiXmlElement *charge = 0;
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
			
			selection = menu.afficher(0, fond, sFond);
			if(selection == 0) {
				Partie *nouvellePartie = Partie::partie();
				nouvellePartie->restaurer(0);
				charge = nouvellePartie->commencer();
				delete nouvellePartie;
			}
			else if(selection == 1) {
				charge = Partie::charger(fond, sFond);
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
