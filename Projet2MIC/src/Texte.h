/*
 *  Texte.h
 *  Jeu C++
 *
 *  Created by Rémi on 12/04/08.
 *  Copyright 2008 Rémi Saurel. All rights reserved.
 *
 */


#ifndef EN_TETE_TEXTE
#define EN_TETE_TEXTE

#include "Ecran.h"
#include <string>
#include "Unichar.h"

typedef unsigned int taillePolice_t;
enum police_t {POLICE_NORMALE, POLICE_DECO};

class Texte {	
public:
	Texte(Texte const &);
	Texte &operator=(Texte const &);

	Texte(Unichar const &txt = Unichar(), police_t police = POLICE_NORMALE, taillePolice_t taille = 12, Couleur const &coul = Couleur::noir);
	
	virtual ~Texte();
	
	static Coordonnees dimensions(Unichar const &texte, police_t police, taillePolice_t taille);
	static Coordonnees dimensions(Unichar const &texte, uindex_t p, size_t n, police_t police, taillePolice_t taille);
	static dimension_t hauteur(police_t police, taillePolice_t taille);
	
	Unichar const &texte() const { return _texte; }
	police_t police() const { return _police; }
	taillePolice_t taille() const { return _taille; }
	Couleur couleur() const { return _couleur; }

	inline Coordonnees dimensions() const { return _dimensions; }
	inline Coordonnees dimensions(unsigned long p, size_t n) const { return Texte::dimensions(_texte, p, n, _police, _taille); }
	
	Texte &definir(Unichar const &txt);
	Texte &definir(police_t police, taillePolice_t taille);
	Texte &definir(Unichar const &txt, police_t police, taillePolice_t taille);
	Texte &definir(Couleur const &coul);
	
	void afficher(Coordonnees const &pos) const;
	static void afficher(Unichar const &texte, police_t police, taillePolice_t taille, Couleur const &couleur, Coordonnees pos);
	
	static void nettoyer();

protected:
	Unichar _texte;
	police_t _police;
	taillePolice_t _taille;
	Couleur _couleur;
	
	Coordonnees _dimensions;
	
};

#endif
