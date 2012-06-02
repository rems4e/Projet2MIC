//
//  Coordonnees.h
//  Projet2MIC
//
//  Créé par Marc Promé et Rémi Saurel.
//  Ce fichier et son contenu sont librement distribuables, modifiables et utilisables pour toute œuvre non commerciale, à condition d'en citer les auteurs.
//

#ifndef COORDONNEES_H
#define COORDONNEES_H

struct Rectangle;

typedef double coordonnee_t;
typedef coordonnee_t dimension_t;

struct Coordonnees {
	static inline bool valeurNulle(coordonnee_t v) { return (v < 0 ? -v : v) < 0.001; }
	coordonnee_t x, y;
	
	static Coordonnees const aucun, zero, un, eX, eY;
	static coordonnee_t const coordMin, coordMax;
	
	inline Coordonnees() : x(0), y(0) {}
	inline Coordonnees(coordonnee_t const &_x, coordonnee_t const &_y) : x(_x), y(_y) {}
				
	inline virtual ~Coordonnees() {}
	
	inline bool operator==(Coordonnees const &c) const { return valeurNulle(x - c.x) && valeurNulle(y - c.y); }
	inline bool operator!=(Coordonnees const &c) const { return !(*this == c); }
	
	
	inline Coordonnees operator+(Coordonnees const &c) const { return Coordonnees(x + c.x, y + c.y); }
	inline Coordonnees operator-(Coordonnees const &c) const { return Coordonnees(x - c.x, y - c.y); }
	inline Coordonnees operator-() const { return Coordonnees(-x, -y); }
	
	inline Coordonnees &operator=(Coordonnees const &c) {
		x = c.x;
		y = c.y;
		
		return *this;
	}
	
	inline Coordonnees &operator+=(Coordonnees const &c) {
		x += c.x;
		y += c.y;
		
		return *this;
	}
	
	inline Coordonnees &operator-=(Coordonnees const &c) {
		x -= c.x;
		y -= c.y;
		
		return *this;
	}
	
	inline Coordonnees &operator*=(double d) {
		x *= d;
		y *= d;
		
		return *this;
	}
	
	inline Coordonnees operator*(double d) const { return Coordonnees(x * d, y * d); }
	
	inline Coordonnees &operator/=(double d) {
		x /= d;
		y /= d;
		
		return *this;
	}
	
	inline Coordonnees operator/(double d) const { return Coordonnees(x / d, y / d); }
	
	inline Coordonnees etirer(Coordonnees const &c) const { return Coordonnees(x * c.x, y * c.y); }
		
	// Inclusion dans @r
	bool operator<(Rectangle const &r) const;
	
	inline dimension_t norme2() const { return x * x + y * y; }
	dimension_t norme() const;
	
	inline Coordonnees &normaliser() { *this /= this->norme(); return *this; }
	inline Coordonnees normaliser() const { return *this / this->norme(); }
	inline bool vecteurNul() const { return valeurNulle(x) && valeurNulle(y); }
};

inline Coordonnees operator*(double d, Coordonnees const &c) { return c * d; }

#include "Rectangle.h"

#endif
