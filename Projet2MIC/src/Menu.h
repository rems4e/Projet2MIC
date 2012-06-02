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

class Image;

class Menu {
public:
	Menu(Unichar const &titre, std::vector<Unichar> const &elements, Unichar const &dernierElement = "Retour");
	virtual ~Menu();
	
	index_t afficher(index_t selection, Image const &fond, Shader const &s = Shader::flou(1), bool voile = true);
	
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
 

#endif
