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
#include <algorithm>

#define TAILLE_TITRE_MENU 42
#define TAILLE_ELEMENTS_MENU 30
#define ECART_ELEM 5
#define TEMPS_ENTREE 1.0f
#define FLOU_FOND 1.0f

Menu::element_t::element_t(Unichar const &u) : _texte(u), _cadre() {
	_texte.definir(POLICE_DECO, TAILLE_ELEMENTS_MENU);
	_cadre.definirDimensions(_texte.dimensions());
	_texte.definir(Couleur::grisClair);
}

struct Menu::TestNul {
	bool operator()(Menu::element_t const &e) {
		return !e._texte.texte().size();
	}
};

Menu::Menu(Unichar const &titre, std::vector<Unichar> const &elements) : _titre(titre, POLICE_DECO, TAILLE_TITRE_MENU, Couleur::blanc), _elements(elements.begin(), elements.end()), _premierElementAffiche(0), _nbElementsAffiches(0) {
	std::remove_if(_elements.begin(), _elements.end(), Menu::TestNul());
	_elements.push_back(element_t("Retour"));
	size_t dim = 0;
	for(std::vector<element_t>::iterator i = _elements.begin(); i != _elements.end(); ++i) {
		dim += i->_texte.dimensions().y;
		if(dim > Menu::tailleMax())
			break;
		
		dim += ECART_ELEM;
		++ _nbElementsAffiches;
	}
}

Menu::~Menu() {

}

index_t Menu::afficher(Image *fond) {
	bool continuer = true;
	index_t retour = 0;
	index_t elementSelectionne = 0;
	horloge_t ancienDefilement = 0;
	
	Image *apercu = fond;
	if(apercu == 0)
		apercu = Ecran::apercu();
		
	Session::reinitialiserEvenements();
	
	horloge_t ref = horloge();
	
	while(Session::boucle(100, continuer)) {
		Ecran::definirPointeurAffiche(true);
		Ecran::effacer();
		
		Image::definirTeinte(Couleur(0, std::min((horloge() - ref) / TEMPS_ENTREE, 1.0f) * 80));
		Shader::flou(FLOU_FOND).activer();
		apercu->afficher(Coordonnees());
		Shader::desactiver();
		
		Image::definirTeinte(Couleur::blanc);
		
		//Ecran::afficherRectangle(Ecran::ecran(), Couleur(0, 0, 0, 128));
		this->afficherElements(elementSelectionne);
		Ecran::finaliser();
				
		if(Session::evenement(Session::T_ESC)) {
			retour = _elements.size() - 1;
			continuer = false;
		}
		else if(Session::evenement(Session::T_ENTREE) || Session::evenement(Session::T_ESPACE) || (Session::evenement(Session::B_GAUCHE) && Session::souris() < _elements[elementSelectionne]._cadre)) {
			retour = elementSelectionne;
			continuer = false;
		}
		else if(Session::evenement(Session::T_HAUT) && horloge() - ancienDefilement > INTERVALLE_DEFILEMENT) {
			if(elementSelectionne > 0) {
				--elementSelectionne;
				if(elementSelectionne < _premierElementAffiche)
					--_premierElementAffiche;
			}
			else {
				elementSelectionne = _elements.size() - 1;
				if(_elements.size() > _nbElementsAffiches)
					_premierElementAffiche = _elements.size() - _nbElementsAffiches;
			}
			
			ancienDefilement = horloge();
		}
		else if(Session::evenement(Session::T_BAS) && horloge() - ancienDefilement > INTERVALLE_DEFILEMENT) {
			if(elementSelectionne < _elements.size() - 1) {
				++elementSelectionne;
				if(elementSelectionne >= _premierElementAffiche + _nbElementsAffiches)
					++_premierElementAffiche;
			}
			else {
				elementSelectionne = 0;
				_premierElementAffiche = 0;
			}
			
			ancienDefilement = horloge();
		}
		else if(Session::evenement(Session::SOURIS)) {
			for(std::vector<element_t>::iterator i = _elements.begin() + _premierElementAffiche; i != _elements.end() && i != _elements.begin() + _premierElementAffiche + _nbElementsAffiches; ++i) {
				if(Session::souris() < i->_cadre) {
					elementSelectionne = i - _elements.begin();
				}
			}
		}
		
		Ecran::maj();
	}
	
	if(fond == 0)
		delete apercu;
	
	return retour;
}

void Menu::afficherElements(index_t elementSelectionne) {
	dimension_t ordonnee = 80;
	_titre.afficher(Coordonnees((Ecran::largeur() - _titre.dimensions().x) / 2, ordonnee));
	ordonnee += _titre.dimensions().y + 40;
	
	_elements[elementSelectionne]._texte.definir(Couleur::blanc);
	
	for(std::vector<element_t>::iterator i = _elements.begin() + _premierElementAffiche; i != _elements.end() && i != _elements.begin() + _premierElementAffiche + _nbElementsAffiches; ++i) {
		i->_cadre.definirOrigine(Coordonnees((Ecran::largeur() - i->_texte.dimensions().x) / 2, ordonnee));
		i->_texte.afficher(i->_cadre.origine());
		ordonnee += i->_cadre.hauteur + ECART_ELEM;
	}
	
	_elements[elementSelectionne]._texte.definir(Couleur::grisClair);
}

size_t Menu::tailleMax() {
	return Ecran::hauteur() * 2 / 3;
}
