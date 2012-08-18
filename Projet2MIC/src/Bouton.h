/*
 *  Bouton.h
 *  Jeu C++
 *
 *  Created by Rémi on 08/01/11.
 *  Copyright 2011 Rémi Saurel. All rights reserved.
 *
 */

#ifndef BOUTON_H
#define BOUTON_H

#include "ControleInterface.h"
#include "Texte.h"
#include "Image.h"
#include <algorithm>

class Image;
class Texte;

class Bouton : public ControleInterface {	
public:
	enum {
		normal,
		on_off
	};
	
	Bouton(glm::vec2 const &position, std::string const &texte, double hauteur = -1.0);
	Bouton(glm::vec2 const &position, Image const &image);
	virtual ~Bouton();
	
	inline int comportement() const { return _comportement; }
	void definirComportement(int c);
	
	Image const &image() const;
	void definirImage(Image const &img);
	Unichar const &texte() const;
	void definirTexte(Unichar const &txt);
	
	inline bool estImage() const { return _estImage; }
	inline bool estTexte() const { return !_estImage; }
	
	int valeurInt() const;
	bool valeurBool() const;
	
	void definirValeurBool(bool valeur);
	inline void definirValeurInt(int valeur) { this->definirValeurBool(valeur); }
	
protected:		
	Image _image;
	Texte _texte;
	bool _estImage;
	bool _valeur;
	int _comportement;
	bool _enfonce;
	
	Bouton(Bouton const &);
	Bouton &operator=(Bouton const &);

	void gestionClavier();	
	void dessiner();
	VueInterface *gestionClic();
};

class GroupeBoutons : public ControleInterface {	
public:
	GroupeBoutons(glm::vec2 const &pos, Bouton **elems, glm::vec2 const &dim, glm::vec2 const &ecart, int elemSelect = 0, bool obligatChoix = true);
	virtual ~GroupeBoutons();
	
	inline index_t elementSelectionne() const { return _elementSelectionne; }
	inline int valeurInt() const { return static_cast<int>(this->elementSelectionne()); }
	inline void definirValeurInt(int elem) { this->definirSelection(elem); }
	inline bool obligationChoix() const { return _obligationChoix; }
	
	inline void definirActif(int elem, bool actif) { this->element(elem)->definirActif(actif); }
	
	void definirSelection(int elem, bool action = true);
	
	virtual void definirCadre(Rectangle const &c);
	
protected:
	glm::vec2 _dimensionsGroupe;
	glm::vec2 _ecartementGroupe;
	index_t _elementSelectionne;
	bool _obligationChoix;
	
	GroupeBoutons (GroupeBoutons const &);
	GroupeBoutons &operator=(GroupeBoutons const &);
	inline Bouton *element(index_t elem) {
		std::list<VueInterface *>::const_iterator it = _objets.begin();
		std::advance(it, elem);
		return static_cast<Bouton *>(*it);
	}
	static void actionMembre(ControleInterface *bouton, void *groupe);

	//void dessiner();
};

class CaseACocher : public Bouton {	
public:
	CaseACocher(glm::vec2 const &pos, std::string const &txt, bool enfonce = false, Couleur const &c = Couleur::noir);
	//CaseACocher(Coordonnees const &pos, Texte const &txt, std::string const &description, boolenfonce = false);
	virtual ~CaseACocher();
		
	void definirCouleur(Couleur const &c) {
		_texte.definir(c);
	}
	
protected:
	CaseACocher (CaseACocher const &);
	CaseACocher &operator=(CaseACocher const &);
	
	void dessiner();
};

#endif
