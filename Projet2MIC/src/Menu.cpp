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
#include "Affichage.h"

Menu::element_t::element_t(Unichar const &u) : _texte(u), _cadre() {
	_texte.definir(COULEUR_ELEM);
}

struct Menu::TestNul {
	bool operator()(Menu::element_t const &e) {
		return !e._texte.texte().size();
	}
};

Menu::Menu(Unichar const &titre, std::vector<Unichar> const &elements, Unichar const &dernierElement) : VueInterface(Rectangle(0, 0, Ecran::largeur(), Ecran::hauteur())), _titre(titre, POLICE_DECO, TAILLE_TITRE_MENU, Couleur::blanc), _elements(elements.begin(), elements.end()), _premierElementAffiche(0), _nbElementsAffiches(0), _echap(true) {
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

index_t Menu::afficher(index_t selection, Image const &fond, Shader const &s, horloge_t tempsInitial, AfficheurAuxiliaire *afficheurAuxiliare) {
	_selection = selection;
	_fond = &fond;
	_s = &s;
	_tempsInitial = tempsInitial;
	_ancienDefilement = 0;
	_continuer = true;
	
	_afficheurAuxiliaire = afficheurAuxiliare;
	if(_afficheurAuxiliaire) {
		this->ajouterEnfant(*_afficheurAuxiliaire);
	}
	
	Session::ajouterVueFenetre(this);
	
	return _retour;
}

void Menu::dessiner() {
	_fond->redimensionner(glm::vec2(Ecran::largeur() / static_cast<coordonnee_t>(_fond->dimensionsReelles().x), Ecran::hauteur() /static_cast<coordonnee_t>( _fond->dimensionsReelles().y)));
	
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
	
	_s->activer();
	_s->definirParametre(Shader::temps, horloge() - _tempsInitial);
	_s->definirParametre(Shader::tempsAbsolu, horloge());
	_fond->afficher(glm::vec2(0));
	Shader::desactiver();
	
	this->afficherElements(_selection);
}

void Menu::afficherElements(index_t elementSelectionne) {
	dimension_t ordonnee = 80 * Ecran::echelleMin();
	_titre.definir(TAILLE_TITRE_MENU * Ecran::echelleMin());
	_titre.afficher(glm::vec2((Ecran::largeur() - _titre.dimensions().x) / 2, ordonnee));
	ordonnee += _titre.dimensions().y + 40 * Ecran::echelleMin();
	
	_elements[elementSelectionne]._texte.definir(Couleur::blanc);
	
	for(std::vector<element_t>::iterator i = _elements.begin() + _premierElementAffiche; i != _elements.end() && i != _elements.begin() + _premierElementAffiche + _nbElementsAffiches; ++i) {
		i->_cadre.definirOrigine(glm::vec2((Ecran::largeur() - i->_texte.dimensions().x) / 2, ordonnee));
		i->_texte.definir(TAILLE_ELEMENTS_MENU * Ecran::echelleMin());
		i->_texte.afficher(i->_cadre.origine());
		ordonnee += i->_cadre.hauteur + ECART_ELEM * Ecran::echelleMin();
	}
	
	_elements[elementSelectionne]._texte.definir(COULEUR_ELEM);
}

size_t Menu::tailleMax() {
	return Ecran::hauteur() * 2 / 3;
}

void Menu::gestionClavier() {	
	Ecran::definirPointeurAffiche(true);

	if(_continuer) {
		if(_echap && Session::evenement(Session::T_ESC)) {
			_retour = _elements.size() - 1;
			_continuer = false;
		}
		else if(Session::evenement(Session::T_ENTREE) || Session::evenement(Session::T_ESPACE) || (Session::evenement(Session::B_GAUCHE) && _elements[_selection]._cadre.contientPoint(Session::souris()))) {
			_retour = _selection;
			_continuer = false;
		}
		else if(Session::evenement(Session::T_HAUT) && horloge() - _ancienDefilement > INTERVALLE_DEFILEMENT) {
			if(_selection > 0) {
				--_selection;
				if(_selection < _premierElementAffiche)
					--_premierElementAffiche;
			}
			else {
				_selection = _elements.size() - 1;
				if(_elements.size() > _nbElementsAffiches)
					_premierElementAffiche = _elements.size() - _nbElementsAffiches;
			}
			
			_ancienDefilement = horloge();
		}
		else if(Session::evenement(Session::T_BAS) && horloge() - _ancienDefilement > INTERVALLE_DEFILEMENT) {
			if(_selection < _elements.size() - 1) {
				++_selection;
				if(_selection >= _premierElementAffiche + _nbElementsAffiches)
					++_premierElementAffiche;
			}
			else {
				_selection = 0;
				_premierElementAffiche = 0;
			}
			
			_ancienDefilement = horloge();
		}
		else if(Session::evenement(Session::SOURIS)) {
			for(std::vector<element_t>::iterator i = _elements.begin() + _premierElementAffiche; i != _elements.end() && i != _elements.begin() + _premierElementAffiche + _nbElementsAffiches; ++i) {
				if(i->_cadre.contientPoint(Session::souris())) {
					_selection = i - _elements.begin();
				}
			}
		}
	}
	
	if(!_continuer)
		Session::supprimerVueFenetre();
	
	/*if(!afficheurAuxiliaire.gestionEvenements()) {
		_retour = _elements.size() - 1;
		_continuer = false;
	}*/
}

void Menu::quitterMenu() {
	this->quitterMenu(_elements.size() - 1);
}

void Menu::quitterMenu(index_t selection) {
	if(selection < 0 || selection >= _elements.size())
		throw std::out_of_range("Valeur de retour demandée pour le menu incorrecte !");
}

