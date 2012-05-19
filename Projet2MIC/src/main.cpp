//
//  main.cpp
//  Projet2MIC
//
//  Created by Rémi Saurel on 31/01/12.
//  Copyright (c) 2012 Rémi Saurel. All rights reserved.
//

#include "Session.h"
#include "Shader.h"
#include "Menu.h"
#include "Partie.h"
#include "Audio.h"
#include "Image.h"
#include "tinyxml.h"

#ifdef __cplusplus
extern "C"
#endif

void jeu();

int main(int, char **) {	
	// Initialise tous les sous-programmes requis pour le jeu : affichage (images, textes, écran…), audio, événements, paramètres utilisateur…
	Session::initialiser();
	
	jeu();
	
	// Passé ce point, toutes les ressources doivent être déjà désallouées, car à cause du système de gestion de ressources partagées, leur désallocation après le nettoyage entrainerait un plantage du programme.
	Session::nettoyer();
	
	return 0;
}

void jeu() {
	Audio::audio_t musique = Audio::chargerMusique(Session::cheminRessources() + "menu.mp3");
	
	Shader sFond(Session::cheminRessources() + "aucun.vert", Session::cheminRessources() + "menuPrincipal.frag");
	std::vector<Unichar> elements;
	elements.push_back("Nouvelle partie");
	elements.push_back("Charger une partie");
	elements.push_back("Réglages");
	Menu menu("Menu principal", elements, "Quitter");
	
	Image fond(Session::cheminRessources() + "tex0.jpg");
	index_t selection = 0;
	
	TiXmlElement *charge = 0;
	do {		
		if(charge) {
			Partie *nouvellePartie = Partie::creerPartie(charge);
			delete charge;
			charge = nouvellePartie->commencer();
			delete nouvellePartie;
		}
		else {
			Audio::definirMusique(musique);
			Audio::jouerMusique();
			
			selection = menu.afficher(0, fond, sFond);
			if(selection == 0) {
				Partie *nouvellePartie = Partie::creerPartie();
				charge = nouvellePartie->commencer();
				delete nouvellePartie;
			}
			else if(selection == 1) {
				charge = Partie::charger(&fond, sFond);
			}
			else if(selection == 2) {
				Parametres::editerParametres(fond, sFond);
			}
		}
	} while(selection != elements.size());
	
	delete charge;
	Audio::libererSon(musique);
}
