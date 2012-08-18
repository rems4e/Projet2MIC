//
//  Rectangle.h
//  Projet2MIC
//
//  Créé par Marc Promé et Rémi Saurel.
//  Ce fichier et son contenu sont librement distribuables, modifiables et utilisables pour toute œuvre non commerciale, à condition d'en citer les auteurs.
//

#ifndef RECTANGLE_H
#define RECTANGLE_H

#include "Coordonnees.h"
#include <algorithm>

struct Rectangle {
	coordonnee_t gauche, haut;
	dimension_t largeur, hauteur;
	
	static Rectangle const aucun;
	static Rectangle const zero;
	
	inline Rectangle() : gauche(0), haut(0), largeur(0), hauteur(0) { }
	inline Rectangle(coordonnee_t const &g, coordonnee_t const &h, dimension_t const &la, dimension_t const &ha) : gauche(g), haut(h), largeur(la), hauteur(ha) { }
	inline Rectangle(glm::vec2 const &origine, glm::vec2 const &taille) : gauche(origine.x), haut(origine.y), largeur(taille.x), hauteur(taille.y) { }
	inline Rectangle(Rectangle const &r, int) : gauche(0), haut(0), largeur(r.largeur), hauteur(r.hauteur) { }
	
	inline Rectangle &definirOrigine(glm::vec2 const &c) { gauche = c.x, haut = c.y; return *this; }
	inline Rectangle &definirDimensions(glm::vec2 const &c) { largeur = c.x, hauteur = c.y; return *this; }
	
	inline glm::vec2 origine() const { return glm::vec2(gauche, haut); }
	inline glm::vec2 dimensions() const { return glm::vec2(largeur, hauteur); }
	
	inline Rectangle etirer(glm::vec2 const &c) const { return Rectangle(gauche * c.x, haut * c.y, largeur * c.x, hauteur * c.y); }

	inline Rectangle cadreDecale(glm::vec2 const &vec) const {
		return Rectangle(gauche + vec.x, haut + vec.y, largeur, hauteur);
	}
	inline Rectangle &decaler(glm::vec2 const &vec) {
		gauche += vec.x;
		haut += vec.y;
		
		return *this;
	}
	
	/*inline Rectangle operator+(glm::vec2 const &c) const { return Rectangle(gauche + c.x, haut + c.y, largeur, hauteur); }
	inline Rectangle operator+=(glm::vec2 const &c) {
		gauche += c.x;
		haut += c.y;
		return *this;
	}
	inline Rectangle operator-(glm::vec2 const &c) const { return Rectangle(gauche - c.x, haut - c.y, largeur, hauteur); }
	inline Rectangle operator-=(glm::vec2 const &c) {
		gauche -= c.x;
		haut -= c.y;
		return *this;
	}
	inline glm::vec2 operator-(Rectangle const &r) const {
		return glm::vec2(gauche - r.gauche, haut - r.haut);
	}*/
	
	inline bool contientPoint(glm::vec2 const &p) const {
		return Rectangle(p, glm::vec2(0.0)) < *this;
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

	inline bool estVide() const { return valeurNulle(largeur * hauteur); }
	
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
