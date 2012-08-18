//
//  Menu.h
//  Projet2MIC
//
//  Créé par Marc Promé et Rémi Saurel.
//  Ce fichier et son contenu sont librement distribuables, modifiables et utilisables pour toute œuvre non commerciale, à condition d'en citer les auteurs.
//


#ifndef EN_TETE_MENU
#define EN_TETE_MENU

#include "Geometrie.h"
#include <string>
#include <vector>
#include "Unichar.h"
#include "Texte.h"
#include "horloge.h"
#include "Shader.h"
#include "Image.h"
#include "Session.h"
#include "VueInterface.h"

#define TAILLE_TITRE_MENU 46
#define TAILLE_ELEMENTS_MENU 32
#define ECART_ELEM 5
#define COULEUR_ELEM Couleur(255, 160)

class Image;

class Menu : protected VueInterface {
public:	
	struct AfficheurAuxiliaire : public VueInterface {
		AfficheurAuxiliaire(Menu &m) : VueInterface(Rectangle()), _m(m) {
		
		}
		
		Menu &menu() {
			return _m;
		}
		
	private:
		Menu &_m;
	};

	Menu(Unichar const &titre, std::vector<Unichar> const &elements, Unichar const &dernierElement = TRAD("menu Retour"));
	virtual ~Menu();
	
	index_t afficher(index_t selection, Image const &fond, Shader const &s = Shader::flou(1), horloge_t tempsInitial = horloge(), AfficheurAuxiliaire *afficheurAuxiliaire = 0);
	
	void quitterMenu();
	void quitterMenu(index_t selection);
	
protected:
	Menu (Menu const &);
	Menu &operator=(Menu const &);

	void afficherElements(index_t elementSelectionne);
	static size_t tailleMax();
	
	void dessiner();
	void gestionClavier();

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
	
	index_t _selection;
	Image const *_fond;
	Shader const *_s;
	horloge_t _tempsInitial;
	horloge_t _ancienDefilement;
	index_t _retour;
	
	bool _continuer;
	
	VueInterface *_afficheurAuxiliaire;
};
 
#endif
