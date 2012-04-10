/*
 *  Constantes.h
 *  Jeu C++
 *
 *  Created by Rémi on 20/06/07.
 *  Copyright 2007 Rémi Saurel. All rights reserved.
 *
 */

#ifndef CONSTANTES_H
#define CONSTANTES_H

#define DEVELOPPEMENT

#ifndef DEVELOPPEMENT
#define NDEBUG
#endif

//#define MULTITHREAD 0

#include <string>
#include <cassert>
#include <SDL/SDL.h>

#ifdef __MACOSX__
#define GL_H <OpenGL/gl.h>
#else
#define GL_H <GL/gl.h>
#endif

#define GAUCHE 0
#define DROITE 1
#define HAUT 2
#define BAS 3

#define TAILLE_TEXTE_CHARGEMENT 42
#define INTERVALLE_DEFILEMENT 0.150f

typedef size_t uindex_t;
typedef ssize_t index_t;

namespace Session {
	std::string const &cheminRessources();
}

#endif
