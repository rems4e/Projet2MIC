/*
 *  Menu.cpp
 *  Jeu C++
 *
 *  Created by Rémi on 17/12/07.
 *  Copyright 2007 Rémi Saurel. All rights reserved.
 *
 */

#include "Menu.h"
#include "Image.h"
#include "Session.h"
#include "Ecran.h"

#define INTERVALLE_DEFILEMENT 0.150f
#define TAILLE_TITRE_MENU 42
#define TAILLE_ELEMENTS_MENU 30

Menu::element_t::element_t(Unichar const &u) : _texte(u), _cadre() {
	_texte.definir(POLICE_DECO, TAILLE_ELEMENTS_MENU);
	_cadre.definirDimensions(_texte.dimensions());
	_texte.definir(Couleur::grisClair);
}

Menu::Menu(Unichar const &titre, std::vector<Unichar> const &elements) : _titre(titre, POLICE_DECO, TAILLE_TITRE_MENU, Couleur::blanc), _elements(elements.begin(), elements.end()) {	
	_elements.push_back(element_t("Retour"));
}

Menu::~Menu() {

}

index_t Menu::afficher() {
	bool continuer = true;
	index_t retour = 0;
	index_t elementSelectionne = 0;
	horloge_t ancienDefilement = 0;
	
	Image *apercu = Ecran::apercu();
	Session::reinitialiserEvenements();
	
	while(Session::boucle(60, continuer)) {
		Ecran::effacer();
		apercu->afficher(Coordonnees());
		Ecran::afficherRectangle(Ecran::ecran(), Couleur(0, 0, 0, 160));
		this->afficherElements(elementSelectionne);
				
		if(Session::evenement(Session::QUITTER) || Session::evenement(Session::T_ESC)) {
			retour = _elements.size() - 1;
			continuer = false;
		}
		else if(Session::evenement(Session::T_ENTREE) || Session::evenement(Session::T_ESPACE) || (Session::evenement(Session::B_GAUCHE) && Session::souris() < _elements[elementSelectionne]._cadre)) {
			retour = elementSelectionne;
			continuer = false;
		}
		else if(Session::evenement(Session::T_HAUT) && horloge() - ancienDefilement > INTERVALLE_DEFILEMENT) {
			if(elementSelectionne > 0)
				--elementSelectionne;
			else
				elementSelectionne = _elements.size() - 1;
			
			ancienDefilement = horloge();
		}
		else if(Session::evenement(Session::T_BAS) && horloge() - ancienDefilement > INTERVALLE_DEFILEMENT) {
			if(elementSelectionne < _elements.size() - 1)
				++elementSelectionne;
			else
				elementSelectionne = 0;
			ancienDefilement = horloge();
		}
		else if(Session::evenement(Session::SOURIS)) {
			for(std::vector<element_t>::iterator i = _elements.begin(); i != _elements.end(); ++i) {
				if(Session::souris() < i->_cadre) {
					elementSelectionne = i - _elements.begin();
				}
			}
		}
		
		Ecran::maj();
	}
	
	delete apercu;
	
	return retour;
}

void Menu::afficherElements(index_t elementSelectionne) {
	dimension_t ordonnee = 80;
	_titre.afficher(Coordonnees((Ecran::largeur() - _titre.dimensions().x) / 2, ordonnee));
	ordonnee += _titre.dimensions().y + 40;
	
	_elements[elementSelectionne]._texte.definir(Couleur::blanc);
	
	for(std::vector<element_t>::iterator i = _elements.begin(); i != _elements.end(); ++i) {
		i->_cadre.definirOrigine(Coordonnees((Ecran::largeur() - i->_texte.dimensions().x) / 2, ordonnee));
		i->_texte.afficher(i->_cadre.origine());
		ordonnee += i->_cadre.hauteur + 5;
	}
	
	_elements[elementSelectionne]._texte.definir(Couleur::grisClair);
}
