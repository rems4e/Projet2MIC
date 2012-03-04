/*
 *  Image.h
 *  Jeu C++
 *
 *  Created by Rémi on 10/04/08.
 *  Copyright 2008 Rémi Saurel. All rights reserved.
 *
 */

#ifndef EN_TETE_IMAGE
#define EN_TETE_IMAGE

#include "Constantes.h"
#include <SDL/SDL.h>

#include GL_H

#include <string>
#include "Ecran.h"
#include "Geometrie.h"
#include <stack>

#define REUTILISATION_ID_TEXTURE 1

struct ImageBase;

class Image {
	friend void quitter(int code);
public:
	Image(std::string const &fichier);
	Image(unsigned char *pixels, int largeur, int hauteur, int profondeur, bool retourner = false);
	Image(Image const &img);
	Image();
	Image &operator=(Image const &img);
	virtual ~Image();
	
	// L'image a été chargée correctement
	inline bool valide() const { return _base; }
	
	// Dimensions de l'image multipliées par le facteur de zoom
	Coordonnees dimensions() const;
	// Dimensions de l'image zommée à 100 %
	Coordonnees dimensionsReelles() const;
	
	// Transformations de l'image
	Image const &tourner(float angle) const;
	Image const &redimensionner(facteur_t facteur) const;
	Image const &redimensionner(facteur_t facteurX, facteur_t facteurY) const;
	
	// Affichage de l'image à une position donnée
	inline void afficher(Coordonnees const &position) const { this->afficher(position, Rectangle(Coordonnees(), this->dimensionsReelles())); }
	// Affichage de la portion de l'image définie par le rectangle
	void afficher(Coordonnees const &position, Rectangle const &filtre) const;
	
	// Fichier de l'image ou chaîne vide si l'image a été générée à partir d'une matrice de pixels
	inline std::string const &fichier() const;
	
	// Opacité de l'affichage de 0 = transparent à 255 = opaque. Valable pour toutes les images.
	static unsigned char opacite();
	static void definirOpacite(unsigned char o);
	
	// Teinte de l'image : Couleur::blanc -> image non teintée. Valable pour toutes les images.
	static Couleur teinte();
	static void definirTeinte(Couleur const &c);
	
#if REUTILISATION_ID_TEXTURE
	static GLuint const aucuneTexture;
	static void changerTexture(GLuint tex);	
#endif

protected:
	
	ImageBase *_base;
	
	mutable facteur_t _facteurX, _facteurY;
	mutable float _angle;
#if REUTILISATION_ID_TEXTURE
	static GLuint _derniereTexture;
#endif
	static Couleur _teinte;
	static unsigned char _opacite;
		
	static void nettoyer();
	
	Image *charger(std::string const &fichier);
	Image *charger(unsigned char *pixels, int largeur, int hauteur, int profondeur, bool retourner);
};

#endif
