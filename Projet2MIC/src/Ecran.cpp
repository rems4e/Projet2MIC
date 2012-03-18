/*
 *  Ecran.cpp
 *  Jeu C++
 *
 *  Created by Rémi on 06/07/07.
 *  Copyright 2007 Rémi Saurel. All rights reserved.
 *
 */

#include "Ecran.h"
#include "Constantes.h"
#include "fonctions.h"

#include GL_H

#include "SDL/SDL.h"
#include "Texte.h"
#include "Image.h"
#include "Session.h"
#include <limits>
#include <cassert>
#include <algorithm>

Couleur teinte = Couleur(255, 255, 255);

Couleur const Couleur::noir(0);
Couleur const Couleur::gris(128);
Couleur const Couleur::grisClair(200);
Couleur const Couleur::grisFonce(80);
Couleur const Couleur::blanc(255);
Couleur const Couleur::rouge(255, 0, 0);
Couleur const Couleur::vert(0, 255, 0);
Couleur const Couleur::bleu(0, 0, 255);
Couleur const Couleur::transparent(0, 0);

namespace Ecran {	
	struct AttributsEcran {
		Texte _texte;
		Image *_pointeur;
		Image *_pointeurDefaut;
		Coordonnees _decalagePointeur;
		bool _pointeurAffiche;
		
		int _largeur;
		int _hauteur;
		int _profondeur;
		bool _pleinEcran;
						
		float _frequence;
		float _frequenceInstantanee;
		
		
		AttributsEcran();
		~AttributsEcran();
		
	private:
		AttributsEcran(AttributsEcran const &);
		AttributsEcran &operator=(AttributsEcran const &);
	};
	
	AttributsEcran _attributs;
	Rectangle _ecran;

	void init(unsigned int largeur, unsigned int hauteur, unsigned int profondeur, bool pleinEcran);
}

void Ecran::init(unsigned int largeur, unsigned int hauteur, unsigned int profondeur, bool pleinEcran) {
	Ecran::modifierResolution(largeur, hauteur, profondeur, pleinEcran);
}

Ecran::AttributsEcran::AttributsEcran() : _largeur(0L), _hauteur(0L), _profondeur(0), _pleinEcran(false), _frequence(1.0f), _frequenceInstantanee(1.0f), _texte("", POLICE_NORMALE, 12, Couleur::blanc), _pointeur(0), _pointeurDefaut(0), _decalagePointeur(), _pointeurAffiche() {

}

Ecran::AttributsEcran::~AttributsEcran() {
	SDL_ShowCursor(SDL_ENABLE);
	delete _pointeurDefaut;

	if(_pleinEcran) {
		Ecran::modifierResolution(800, 600, 32, false);
		Ecran::afficherRectangle(Ecran::ecran(), Couleur::noir);
		SDL_GL_SwapBuffers();
	}
}

void Ecran::modifierResolution(unsigned int largeur, unsigned int hauteur, unsigned int profondeur, bool pleinEcran) {
#if REUTILISATION_ID_TEXTURE
	Image::changerTexture(Image::aucuneTexture);
#endif
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, SYNCHRO_VERTICALE);
	SDL_Surface *resultat = 0;
	if(pleinEcran)
		resultat = SDL_SetVideoMode(largeur, hauteur, 32, SDL_OPENGL | SDL_FULLSCREEN | SDL_ASYNCBLIT);
	else
		resultat = SDL_SetVideoMode(largeur, hauteur, 32, SDL_OPENGL | SDL_ASYNCBLIT);
	
	
	if(resultat == 0) {
		std::cerr << "Impossible de définir l'écran à une résolution de " << largeur << "*" << hauteur << "*" << profondeur << " (plein écran : " << pleinEcran << "). Erreur : " << SDL_GetError() << std::endl;
		
		quitter(1);
	}
	SDL_ShowCursor(SDL_DISABLE);

	Ecran::_attributs._largeur = largeur;
	Ecran::_attributs._hauteur = hauteur;
	Ecran::_attributs._profondeur = profondeur;
	Ecran::_attributs._pleinEcran = pleinEcran;
	
	Ecran::_attributs._frequence = 0.0f;
	Ecran::_attributs._pointeurAffiche = false;
	delete Ecran::_attributs._pointeurDefaut;
	Ecran::_attributs._pointeurDefaut = new Image(Session::cheminRessources() + "souris.png");
	Ecran::definirPointeur(0);
	
	Ecran::_ecran = Rectangle(Coordonnees(), Ecran::dimensions());
		
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, Ecran::_attributs._largeur, Ecran::_attributs._hauteur, 0, -10, 10);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);
	Ecran::afficherRectangle(Ecran::ecran(), Couleur::noir);
	glClearColor(.8f, .8f, .8f, 1.0f);
	
	Ecran::maj();
}

Image *Ecran::apercu() {
#if REUTILISATION_ID_TEXTURE
	Image::changerTexture(Image::aucuneTexture);
#endif
	
	unsigned char *pixels = new unsigned char[static_cast<int>(Ecran::_attributs._largeur) * static_cast<int>(Ecran::_attributs._hauteur) * 4];

	glReadPixels(0, 0, static_cast<int>(Ecran::_attributs._largeur), static_cast<int>(Ecran::_attributs._hauteur), GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	
	Image *retour = new Image(pixels, static_cast<int>(Ecran::_attributs._largeur), static_cast<int>(Ecran::_attributs._hauteur), 4, true);
	
	delete[] pixels;
	
	return retour;
}

void Ecran::maj() {			
	static float h = 0.0f, h1 = 0.0f;
	static unsigned int c = 0;
	c++;
	float f = horloge();
	Ecran::_attributs._frequenceInstantanee = 1.0f / (f - h1);
	h1 = f;
	if(f - h >= 3.0f) {
		Ecran::_attributs._frequence = c / (f - h);
		c = 0;
		h = f;
	}

#if IPS
	static char ips[255];

	snprintf(ips, 255, "%.2f s - %.1f ips", horloge(), Ecran::_attributs._frequence);
	Ecran::_attributs._texte.definir(ips);
	Coordonnees posIps(Ecran::_attributs._largeur - Ecran::_attributs._texte.dimensions().x - 2, Ecran::_attributs._hauteur - Ecran::_attributs._texte.dimensions().y - 2);
	Ecran::afficherRectangle(Rectangle(posIps - Coordonnees(2, 2), Ecran::_attributs._texte.dimensions() + Coordonnees(4, 4)), Couleur::noir);
	Ecran::_attributs._texte.definir(Couleur::blanc);
	Ecran::_attributs._texte.afficher(posIps);
#endif
	
	if(Ecran::_attributs._pointeurAffiche && Ecran::_attributs._pointeur) {
		Ecran::_attributs._pointeur->afficher(Session::souris() - Ecran::_attributs._decalagePointeur);
	}
	
#if REUTILISATION_ID_TEXTURE
	Image::changerTexture(Image::aucuneTexture);
#endif
	SDL_GL_SwapBuffers();

	glLoadIdentity();
}

void Ecran::effacer() {
	glClear(GL_COLOR_BUFFER_BIT );
}

void Ecran::afficherRectangle(Rectangle const &r, Couleur const &c) {
	Ecran::afficherQuadrilatere(Coordonnees(r.gauche, r.haut), Coordonnees(r.gauche + r.largeur, r.haut), Coordonnees(r.gauche + r.largeur, r.haut + r.hauteur), Coordonnees(r.gauche, r.haut + r.hauteur), c);
}

void Ecran::afficherLigne(Coordonnees const &depart, Coordonnees const &arrivee, Couleur const &c, dimension_t epaisseur) {
	if((depart - arrivee).vecteurNul())
		return;
	Coordonnees normale = (depart - arrivee).normaliser() * epaisseur;
	std::swap(normale.x, normale.y);
	normale.y *= -1;

	Ecran::afficherQuadrilatere(depart - normale, depart + normale, arrivee + normale, arrivee - normale, c);
}

void Ecran::afficherQuadrilatere(Coordonnees const &p1, Coordonnees const &p2, Coordonnees const &p3, Coordonnees const &p4, Couleur const &c) {
#if REUTILISATION_ID_TEXTURE
	Image::changerTexture(Image::aucuneTexture);
#endif	
	glDisable(GL_TEXTURE_2D);
	
	glBegin(GL_QUADS);
	glColor4ub(c.r, c.v, c.b, c.a);
	glVertex2d(p1.x, p1.y);
	glVertex2d(p2.x, p2.y);
	glVertex2d(p3.x, p3.y);
	glVertex2d(p4.x, p4.y);
	glEnd();
	
	glEnable(GL_TEXTURE_2D);	
}

Rectangle Ecran::ecran() {
	return Rectangle(0, 0, Ecran::largeur(), Ecran::hauteur());
}

Coordonnees Ecran::dimensions() {
	return Coordonnees(_attributs._largeur, _attributs._hauteur);
}

int Ecran::largeur() {
	return _attributs._largeur;
}

int Ecran::hauteur() {
	return _attributs._hauteur;
}

int Ecran::profondeur() {
	return _attributs._profondeur;
}

bool Ecran::pleinEcran() {
	return _attributs._pleinEcran;
}

float Ecran::frequence() {
	return _attributs._frequence;
}

float Ecran::frequenceInstantanee() {
	return _attributs._frequenceInstantanee;
}

bool Ecran::pointeurAffiche() {
	return _attributs._pointeurAffiche;
}

void Ecran::definirPointeurAffiche(bool af) {
	_attributs._pointeurAffiche = af;
}

Image const *Ecran::pointeur() {
	return _attributs._pointeur;
}

void Ecran::definirPointeur(Image *image, Coordonnees const &decalage) {
	if(image == 0) {
		_attributs._pointeur = _attributs._pointeurDefaut;
		_attributs._decalagePointeur = Coordonnees();
	}
	else {
		_attributs._pointeur = image;
		_attributs._decalagePointeur = decalage;
	}
}

