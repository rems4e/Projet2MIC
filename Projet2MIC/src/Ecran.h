/*
 *  Ecran.h
 *  Jeu C++
 *
 *  Created by Rémi on 06/07/07.
 *  Copyright 2007 Rémi Saurel. All rights reserved.
 *
 */

#ifndef EN_TETE_ECRAN
#define EN_TETE_ECRAN

#include "Geometrie.h"
#include <string>
#include <list>
#include <stack>
#include "fonctions.h"
#include "Shader.h"

#define LARGEUR_ECRAN 800
#define HAUTEUR_ECRAN 600
#define PROFONDEUR_COULEURS 32
#define PLEIN_ECRAN false
#define SYNCHRO_VERTICALE true
#define VITESSE_RAFRAICHISSEMENT 100
#define IPS true

class Texte;
class Image;

struct Couleur {
	unsigned char r, v, b, a;

	static Couleur const noir;
	static Couleur const gris;
	static Couleur const grisClair;
	static Couleur const grisFonce;
	static Couleur const blanc;
	static Couleur const rouge;
	static Couleur const vert;
	static Couleur const bleu;
	static Couleur const transparent;
	
	inline Couleur() : r(255), v(255), b(255), a(255) { }
	inline Couleur(unsigned char _r, unsigned char _v, unsigned char _b, unsigned char _a = 255) : r(_r), v(_v), b(_b), a(_a) { }
	inline Couleur(unsigned char gris, unsigned char _a = 255) : r(gris), v(gris), b(gris), a(_a) { }
	inline Couleur(Couleur const &c, unsigned char _a) : r(c.r), v(c.v), b(c.b), a(_a) { }
};

namespace Ecran {
	void modifierResolution(unsigned int largeur, unsigned int hauteur, unsigned int profondeur, bool pleinEcran);
	
	Coordonnees dimensions();
	int largeur();
	int hauteur();
	int profondeur();
	
	Rectangle ecran();
	
	bool pleinEcran();
		
	float frequence();
	float frequenceInstantanee();
	
	Image *apercu();
	
	// Met à jour l'affichage en fonction des éléments affichés depuis le dernier appel à la fonction
	void maj();
	// Remplis l'écran avec une couleur unie
	void effacer();
	
	// Affiche une couleur unie dans un rectangle. La couleur peut être transparente.
	void afficherRectangle(Rectangle const &r, Couleur const &c, Shader const &s = Shader::aucun());
	void afficherLigne(Coordonnees const &depart, Coordonnees const &arrivee, Couleur const &c, dimension_t epaisseur = 1.0, Shader const &s = Shader::aucun());
	void afficherQuadrilatere(Coordonnees const &p1, Coordonnees const &p2, Coordonnees const &p3, Coordonnees const &p4, Couleur const &c, Shader const &s = Shader::aucun());
	
	// Pointeur
	bool pointeurAffiche();
	void definirPointeurAffiche(bool af);
	
	Image const *pointeur();
	// Si image vaut 0, le pointeur par défaut est utilisé. La valeur decalage représente les coordonnées du point cliquable de l'image du pointeur.
	void definirPointeur(Image const *image, Coordonnees const &decalage = Coordonnees());
}

#endif
