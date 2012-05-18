/*
 *  Menu.h
 *  Jeu C++
 *
 *  Created by Rémi on 17/12/07.
 *  Copyright 2007 Rémi Saurel. All rights reserved.
 *
 */

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
	
	index_t afficher(index_t selection, Image const &fond, Shader const &s = Shader::flou(1));
	
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
};
 

#endif
