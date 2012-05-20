/*
 *  Rectangle.h
 *  Jeu C++
 *
 *  Created by Rémi on 28/07/11.
 *  Copyright 2011 Rémi Saurel. All rights reserved.
 *
 */

#ifndef RECTANGLE_H
#define RECTANGLE_H

#include "Coordonnees.h"
#include <algorithm>

struct Rectangle {
	coordonnee_t gauche, haut;
	dimension_t largeur, hauteur;
	
	static Rectangle const aucun;
	
	inline Rectangle() : gauche(0), haut(0), largeur(0), hauteur(0) { }
	inline Rectangle(coordonnee_t const &g, coordonnee_t const &h, dimension_t const &la, dimension_t const &ha) : gauche(g), haut(h), largeur(la), hauteur(ha) { }
	inline Rectangle(Coordonnees const &origine, Coordonnees const &taille) : gauche(origine.x), haut(origine.y), largeur(taille.x), hauteur(taille.y) { }
	inline Rectangle(Rectangle const &r, int) : gauche(0), haut(0), largeur(r.largeur), hauteur(r.hauteur) { }
	
	inline Rectangle &definirOrigine(Coordonnees const &c) { gauche = c.x, haut = c.y; return *this; }
	inline Rectangle &definirDimensions(Coordonnees const &c) { largeur = c.x, hauteur = c.y; return *this; }
	
	inline Coordonnees origine() const { return Coordonnees(gauche, haut); }
	inline Coordonnees dimensions() const { return Coordonnees(largeur, hauteur); }
	
	inline Rectangle etirer(Coordonnees const &c) const { return Rectangle(gauche * c.x, haut * c.y, largeur * c.x, hauteur * c.y); }

	inline Rectangle operator+(Coordonnees const &c) const { return Rectangle(gauche + c.x, haut + c.y, largeur, hauteur); }
	inline Rectangle operator+=(Coordonnees const &c) {
		gauche += c.x;
		haut += c.y;
		return *this;
	}
	inline Rectangle operator-(Coordonnees const &c) const { return Rectangle(gauche - c.x, haut - c.y, largeur, hauteur); }
	inline Rectangle operator-=(Coordonnees const &c) {
		gauche -= c.x;
		haut -= c.y;
		return *this;
	}
	inline Coordonnees operator-(Rectangle const &r) const {
		return Coordonnees(gauche - r.gauche, haut - r.haut);
	}
	
	inline bool operator==(Rectangle const &r) const {
		return gauche == r.gauche && haut == r.haut && largeur == r.largeur && hauteur == r.hauteur;
	}
	inline bool operator!=(Rectangle const &r) const {
		return !(*this == r);
	}
	
	inline bool operator<(Rectangle const &r) const { // Inclusion stricte dans r
		return gauche >= r.gauche && haut >= r.haut && gauche + largeur <= r.gauche + r.largeur && haut + hauteur <= r.haut + r.hauteur && *this != r;
	}
	inline bool operator<=(Rectangle const &r) const { // Inclusion dans r
		return *this < r || *this == r;
	}

	inline bool estVide() const { return Coordonnees::valeurNulle(largeur * hauteur); }
	
	inline Rectangle intersection(Rectangle const &r2) const {
		Rectangle retour;
		retour.gauche = std::max(gauche, r2.gauche);
		retour.haut = std::max(haut, r2.haut);
		retour.largeur = std::min(gauche + largeur - retour.gauche, r2.gauche + r2.largeur - retour.gauche);
		retour.hauteur = std::min(haut + hauteur - retour.haut, r2.haut + r2.hauteur - retour.haut);
		if(retour.largeur < 0 || retour.hauteur < 0)
			retour = Rectangle();
		
		return retour;
	}
	
	inline Rectangle unionRect(Rectangle const &r2) const {
		Rectangle retour;
		retour.gauche = std::min(gauche, r2.gauche);
		retour.haut = std::min(haut, r2.haut);
		retour.largeur = std::max(gauche + largeur - retour.gauche, r2.gauche + r2.largeur - retour.gauche);
		retour.hauteur = std::max(haut + hauteur - retour.haut, r2.haut + r2.hauteur - retour.haut);
		
		return retour;
	}
	
	inline bool superposition(Rectangle const &r2) const {
		return ((gauche >= r2.gauche && gauche < r2.gauche + r2.largeur) || (gauche < r2.gauche && gauche + largeur > r2.gauche)) && ((haut >= r2.haut && haut < r2.haut + r2.hauteur) || (haut < r2.haut && haut + hauteur > r2.haut));
	}
};

#endif
