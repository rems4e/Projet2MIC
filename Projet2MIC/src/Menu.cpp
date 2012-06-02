//
//  Menu.cpp
//  Projet2MIC
//
//  Créé par Marc Promé et Rémi Saurel.
//  Ce fichier et son contenu sont librement distribuables, modifiables et utilisables pour toute œuvre non commerciale, à condition d'en citer les auteurs.
//


#include "Menu.h"
#include "Image.h"
#include "Session.h"
#include "Ecran.h"
#include <algorithm>

#define TAILLE_TITRE_MENU 46
#define TAILLE_ELEMENTS_MENU 32
#define ECART_ELEM 5
#define COULEUR_ELEM Couleur(255, 160)

Menu::element_t::element_t(Unichar const &u) : _texte(u), _cadre() {
	_texte.definir(COULEUR_ELEM);
}

struct Menu::TestNul {
	bool operator()(Menu::element_t const &e) {
		return !e._texte.texte().size();
	}
};

Menu::Menu(Unichar const &titre, std::vector<Unichar> const &elements, Unichar const &dernierElement) : _titre(titre, POLICE_DECO, TAILLE_TITRE_MENU, Couleur::blanc), _elements(elements.begin(), elements.end()), _premierElementAffiche(0), _nbElementsAffiches(0), _echap(true) {
	std::remove_if(_elements.begin(), _elements.end(), Menu::TestNul());
	if(dernierElement.size()) {
		_elements.push_back(element_t(dernierElement));
	}
	else {
		_echap = false;
	}
}

Menu::~Menu() {

}

index_t Menu::afficher(index_t selection, Image const &fond, Shader const &s, bool voile) {
	bool continuer = true;
	index_t retour = 0;
	index_t elementSelectionne = selection;
	horloge_t ancienDefilement = 0;
	
	horloge_t const tempsInitial = horloge();
	
	_nbElementsAffiches = 0;
	size_t dim = 0;
	for(std::vector<element_t>::iterator i = _elements.begin(); i != _elements.end(); ++i) {
		dim += i->_texte.dimensions().y;
		if(dim > Menu::tailleMax())
			break;
		
		dim += ECART_ELEM * Ecran::echelleMin();
		++ _nbElementsAffiches;
		
		i->_texte.definir(POLICE_DECO, TAILLE_ELEMENTS_MENU * Ecran::echelleMin());
		i->_cadre.definirDimensions(i->_texte.dimensions());
	}

	Session::reinitialiserEvenements();
		
	while(Session::boucle(FREQUENCE_RAFRAICHISSEMENT, continuer)) {
		fond.redimensionner(Coordonnees(Ecran::largeur() / fond.dimensionsReelles().x, Ecran::hauteur() / fond.dimensionsReelles().y));

		Ecran::definirPointeurAffiche(true);
		Ecran::effacer();
		
		s.activer();
		s.definirParametre(Shader::temps, horloge() - tempsInitial);
		s.definirParametre(Shader::tempsAbsolu, horloge());
		fond.afficher(Coordonnees());
		Shader::desactiver();
		
		if(voile)
			Ecran::afficherRectangle(Ecran::ecran(), Couleur(0, 0, 0, 128));
		
		this->afficherElements(elementSelectionne);
		Ecran::finaliser();

		if(_echap && Session::evenement(Session::T_ESC)) {
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
		
	return retour;
}

void Menu::afficherElements(index_t elementSelectionne) {
	dimension_t ordonnee = 80 * Ecran::echelleMin();
	_titre.definir(TAILLE_TITRE_MENU * Ecran::echelleMin());
	_titre.afficher(Coordonnees((Ecran::largeur() - _titre.dimensions().x) / 2, ordonnee));
	ordonnee += _titre.dimensions().y + 40 * Ecran::echelleMin();
	
	_elements[elementSelectionne]._texte.definir(Couleur::blanc);
	
	for(std::vector<element_t>::iterator i = _elements.begin() + _premierElementAffiche; i != _elements.end() && i != _elements.begin() + _premierElementAffiche + _nbElementsAffiches; ++i) {
		i->_cadre.definirOrigine(Coordonnees((Ecran::largeur() - i->_texte.dimensions().x) / 2, ordonnee));
		i->_texte.definir(TAILLE_ELEMENTS_MENU * Ecran::echelleMin());
		i->_texte.afficher(i->_cadre.origine());
		ordonnee += i->_cadre.hauteur + ECART_ELEM * Ecran::echelleMin();
	}
	
	_elements[elementSelectionne]._texte.definir(COULEUR_ELEM);
}

size_t Menu::tailleMax() {
	return Ecran::hauteur() * 2 / 3;
}
