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

