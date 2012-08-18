//
//  Coordonnees.h
//  Projet2MIC
//
//  Créé par Marc Promé et Rémi Saurel.
//  Ce fichier et son contenu sont librement distribuables, modifiables et utilisables pour toute œuvre non commerciale, à condition d'en citer les auteurs.
//

#ifndef COORDONNEES_H
#define COORDONNEES_H

#include <glm/glm.hpp>

//typedef glm::vec2 Coordonnees;

struct Rectangle;

typedef glm::vec2::value_type coordonnee_t;
typedef coordonnee_t dimension_t;

inline bool valeurNulle(coordonnee_t v) {
	return (v < 0 ? -v : v) < 0.001;
}

template<typename Vec>
inline bool vecteurNul(Vec const &v) {
	for(int i = 0; i < v.length(); ++i)
		if(!valeurNulle(v[i]))
			return false;
	return true;
}

extern glm::vec2 const vec2Aucun;

/*struct Coordonnees {
	static inline bool valeurNulle(coordonnee_t v) { return (v < 0 ? -v : v) < 0.001; }
	coordonnee_t x, y, z;
	
	static Coordonnees const aucun, zero, un, eX, eY, eZ;
	static coordonnee_t const coordMin, coordMax;
	
	inline Coordonnees() : x(0), y(0), z(0) {}
	inline Coordonnees(coordonnee_t const &_x, coordonnee_t const &_y, coordonnee_t const &_z = 0) : x(_x), y(_y), z(_z) {}
				
	inline virtual ~Coordonnees() {}
	
	inline bool operator==(Coordonnees const &c) const { return valeurNulle(x - c.x) && valeurNulle(y - c.y) && valeurNulle(z - c.z); }
	inline bool operator!=(Coordonnees const &c) const { return !(*this == c); }
	
	
	inline Coordonnees operator+(Coordonnees const &c) const { return Coordonnees(x + c.x, y + c.y, z + c.z); }
	inline Coordonnees operator-(Coordonnees const &c) const { return Coordonnees(x - c.x, y - c.y, z - c.z); }
	inline Coordonnees operator-() const { return Coordonnees(-x, -y, -z); }
	
	inline Coordonnees &operator=(Coordonnees const &c) {
		x = c.x;
		y = c.y;
		z = c.z;
		
		return *this;
	}
	
	inline Coordonnees &operator+=(Coordonnees const &c) {
		x += c.x;
		y += c.y;
		z += c.z;
		
		return *this;
	}
	
	inline Coordonnees &operator-=(Coordonnees const &c) {
		x -= c.x;
		y -= c.y;
		z -= c.z;
		
		return *this;
	}
	
	inline Coordonnees &operator*=(double d) {
		x *= d;
		y *= d;
		z *= d;
		
		return *this;
	}
	
	inline Coordonnees operator*(double d) const { return Coordonnees(x * d, y * d, z * d); }
	
	inline Coordonnees &operator/=(double d) {
		x /= d;
		y /= d;
		z /= d;
		
		return *this;
	}
	
	inline Coordonnees operator/(double d) const { return Coordonnees(x / d, y / d, z / d); }
	
	inline Coordonnees etirer(Coordonnees const &c) const { return Coordonnees(x * c.x, y * c.y, z * c.z); }
		
	// Inclusion dans @r
	bool operator<(Rectangle const &r) const;
	
	inline dimension_t norme2() const { return x * x + y * y + z * z; }
	dimension_t norme() const;
	
	inline Coordonnees &normaliser() { *this /= this->norme(); return *this; }
	inline Coordonnees normaliser() const { return *this / this->norme(); }
	inline bool vecteurNul() const { return valeurNulle(x) && valeurNulle(y); }
};

inline Coordonnees operator*(double d, Coordonnees const &c) { return c * d; }
inline coordonnee_t pScalaire(Coordonnees const &v1, Coordonnees &v2) {
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}
inline Coordonnees pVectoriel(Coordonnees const &v1, Coordonnees &v2) {
	return Coordonnees(v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x);
}*/

#include "Rectangle.h"

#endif
