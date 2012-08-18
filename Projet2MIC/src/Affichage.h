//
//  Affichage.h
//  Projet2MIC
//
//  Created by Rémi on 12/08/12.
//  Copyright (c) 2012 Rémi Saurel. All rights reserved.
//

#ifndef EN_TETE_AFFICHAGE
#define EN_TETE_AFFICHAGE

#include "Constantes.h"
#include "Ecran.h"
#include "Image.h"
#include "Coordonnees.h"

#ifdef __MACOSX__
#include <OpenGL/gl.h>
#elif defined(__WIN32__)
#include <GL/glew.h>
#else
#define GL_GLEXT_LEGACY
#include <GL/gl.h>
#define GL_GLEXT_PROTOTYPES
#include <GL/glext.h>
#endif

namespace Affichage {
	// Opacité de l'affichage de 0 = transparent à 255 = opaque.
	Couleur::composante_t const &opacite();
	void ajouterOpacite(Couleur::composante_t o);
	void supprimerOpacite();
	
	// Teinte de l'image : Couleur::blanc -> image non teintée.
	Couleur const &teinte();
	void definirTeinte(Couleur const &c);

	void changerTexture(GLuint tex);
	void ajouterSommet(glm::vec2 const &pos, glm::vec2 const &posTex, Couleur const &couleur, bool depuisEcran);
	
	extern size_t const aucunePosition;
	
	// composantesSommet : 3 ou 4
	void ajouterSommets(size_t sommets, size_t composantesSommet, coordonnee_t const *pos, coordonnee_t const *couleurs, GLushort const *index = nullptr);
	void ajouterSommets(size_t sommets, size_t composantesSommet, coordonnee_t const *pos, coordonnee_t const *posTex, coordonnee_t const *couleurs);
	
	void afficherSommets(size_t sommets, GLfloat const *pos, GLubyte const *couleurs, GLfloat const *normales, GLuint const *index = nullptr);
	void afficherSommetsVBO(size_t sommets, GLuint vbo, size_t posVert, size_t posCouleurs, size_t posNormales, GLuint const *index = nullptr);

	// Affiche une couleur unie dans un rectangle.
	void afficherRectangle(Rectangle const &r, Couleur const &c);
	void afficherLigne(glm::vec2 const &depart, glm::vec2 const &arrivee, Couleur const &c, dimension_t epaisseur = 1.0);
	void afficherQuadrilatere(glm::vec2 const &p1, glm::vec2 const &p2, glm::vec2 const &p3, glm::vec2 const &p4, Couleur const &c);
	void afficherTriangle(glm::vec2 const &p1, glm::vec2 const &p2, glm::vec2 const &p3, Couleur const &c);
}

#endif
