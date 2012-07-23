//
//  Menu.h
//  Projet2MIC
//
//  Créé par Marc Promé et Rémi Saurel.
//  Ce fichier et son contenu sont librement distribuables, modifiables et utilisables pour toute œuvre non commerciale, à condition d'en citer les auteurs.
//


#ifndef EN_TETE_MENU
#define EN_TETE_MENU

#include "Constantes.h"
#include "Geometrie.h"
#include <string>
#include <vector>
#include "Unichar.h"
#include "Texte.h"
#include "horloge.h"
#include "Shader.h"
#include "Image.h"
#include "Session.h"
#include "Ecran.h"
#include <algorithm>

#define TAILLE_TITRE_MENU 46
#define TAILLE_ELEMENTS_MENU 32
#define ECART_ELEM 5
#define COULEUR_ELEM Couleur(255, 160)

class Image;

class Menu {
public:
	struct AfficheRien {
		Rectangle afficher() const {
			return Rectangle::aucun;
		}
		
		bool gestionEvenements() const {
			return true;
		}
	};
	
	Menu(Unichar const &titre, std::vector<Unichar> const &elements, Unichar const &dernierElement = TRAD("menu Retour"));
	virtual ~Menu();
	
	template<class AfficheurAuxiliaire = AfficheRien>
	index_t afficher(index_t selection, Image const &fond, Shader const &s = Shader::flou(1), horloge_t tempsInitial = horloge(), AfficheurAuxiliaire const &aff = AfficheurAuxiliaire());
	
protected:
	Menu (Menu const &);
	Menu &operator=(Menu const &);

	void afficherElements(index_t elementSelectionne);
	static size_t tailleMax();

private:
	struct TestNul;
	struct element_t {
		element_t(Unichar const &txt);
		Texte _texte;
		Rectangle _cadre;
	};
	
	Texte _titre;
	std::vector<element_t> _elements;
	index_t _premierElementAffiche;
	size_t _nbElementsAffiches;
	bool _echap;
};
 
template<class AfficheurAuxiliaire>
index_t Menu::afficher(index_t selection, Image const &fond, Shader const &s, horloge_t tempsInitial, AfficheurAuxiliaire const &afficheurAuxiliaire) {
	bool continuer = true;
	index_t retour = 0;
	index_t elementSelectionne = selection;
	horloge_t ancienDefilement = 0;
		
	Session::reinitialiserEvenements();
	
	while(Session::boucle(FREQUENCE_RAFRAICHISSEMENT, continuer)) {
		fond.redimensionner(Coordonnees(Ecran::largeur() / fond.dimensionsReelles().x, Ecran::hauteur() / fond.dimensionsReelles().y));
		
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
		
		Ecran::definirPointeurAffiche(true);
		Ecran::effacer();
		
		s.activer();
		s.definirParametre(Shader::temps, horloge() - tempsInitial);
		s.definirParametre(Shader::tempsAbsolu, horloge());
		fond.afficher(Coordonnees());
		Shader::desactiver();
				
		this->afficherElements(elementSelectionne);
		afficheurAuxiliaire.afficher();
		
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
		
		if(!afficheurAuxiliaire.gestionEvenements()) {
			retour = _elements.size() - 1;
			continuer = false;
		}
		
		Ecran::maj();
	}
	
	return retour;
}

#endif
