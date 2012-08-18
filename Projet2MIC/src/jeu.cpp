//
//  jeu.cpp
//  Projet2MIC
//
//  Created by Rémi on 31/07/12.
//  Copyright (c) 2012 Rémi Saurel. All rights reserved.
//

#include "jeu.h"
#include "Shader.h"
#include "Menu.h"
#include "Partie.h"
#include "Audio.h"
#include "Image.h"
#include "tinyxml.h"
#include "Editeur.h"
#include "VueInterface.h"

void jeu() {
	Audio::audio_t musique = Audio::chargerMusique(Session::cheminRessources() + "menu.mp3");
	
	Shader sFond(Session::cheminRessources() + "aucun.vert", Session::cheminRessources() + "menuPrincipal.frag");
	
	Image fond(Session::cheminRessources() + "tex0.jpg");
	index_t selection = 0;
	
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
			
			elements.clear();
			elements.push_back(TRAD("gen Nouvelle partie"));
			elements.push_back(TRAD("gen Charger une partie"));
			elements.push_back(TRAD("gen Réglages"));
			elements.push_back(TRAD("gen Crédits"));
			elements.push_back(TRAD("gen Éditeur de niveaux"));
			elements.push_back(TRAD("gen Quitter"));
			Menu menu(TRAD("gen Menu principal"), elements, "");
			
			// FIXME: éviter la reconstruction de l'objet, ou au moins la rendre moins couteuse.
			AfficheurMaj afficheurMaj(menu, Parametres::rechercherMaj() && Parametres::majDisponible());
			
			selection = menu.afficher(0, fond, sFond, 0, &afficheurMaj);
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
