//
//  jeu.h
//  Projet2MIC
//
//  Created by Rémi on 31/07/12.
//  Copyright (c) 2012 Rémi Saurel. All rights reserved.
//

#ifndef Projet2MIC_jeu_h
#define Projet2MIC_jeu_h

#include "Affichage.h"
#include "Menu.h"

// Pour la commande open
#if defined(__WIN32__)
namespace WinWin {
#include <windows.h>
}
#elif defined(__LINUX__)
#include <cstdlib>
#elif defined(__MACOSX__)
#include <cstdlib>
#endif

void jeu();

struct AfficheurMaj : public Menu::AfficheurAuxiliaire {
	AfficheurMaj(Menu &m, bool maj) : Menu::AfficheurAuxiliaire(m), _maj(maj), _txt() {

	}
	
	virtual void preparationDessin() override {
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
		_cadre.definirDimensions(_txt.dimensions() + glm::vec2(10, 10));
		_cadre.definirOrigine(glm::vec2(0, Ecran::hauteur() - _cadre.hauteur));
		
		this->definirCadre(_cadre);
	}
	
	virtual void dessiner() override {
		_cadre.definirOrigine(glm::vec2(0, 0));
		Affichage::afficherRectangle(_cadre, Couleur(0, 0, 0, 128));
		_txt.afficher(_cadre.origine() + glm::vec2(5, 5));
	}
	
	virtual void gestionClavier() override {
		if(!_maj)
			return;
		
		if(_cadre.contientPoint(Session::souris()) && Session::evenement(Session::B_GAUCHE)) {
			Session::reinitialiser(Session::B_GAUCHE);
			
			std::string destination = Parametres::URLMaj();
#if defined(__WIN32__)
			
			WinWin::ShellExecute(NULL, "open", destination.c_str(), NULL, NULL, SW_SHOWNORMAL);
			
#elif defined(__LINUX__)
			
			system(("xdg-open " + destination).c_str());
			
#elif defined(__MACOSX__)
			
			system(("/usr/bin/open " + destination).c_str());
			
#endif
			return;
		}
		
		return;
	}
	
	void definirMaj(bool m) {
		_maj = m;
	}
	
private:
	bool _maj;
	mutable Rectangle _cadre;
	mutable Texte _txt;
};

#endif /* defined(__Projet2MIC__jeu__) */
