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

#define VERSION_MAJ 1
#define VERSION_MIN 1

#define DEVELOPPEMENT

#ifndef DEVELOPPEMENT
#define NDEBUG
#endif

#include <string>
#include <cassert>

// Pour avoir les macros qui déterminent la plate-forme de compilation
#include <SDL/SDL.h>

#define GAUCHE 0
#define DROITE 1
#define HAUT 2
#define BAS 3

#define TAILLE_TEXTE_CHARGEMENT (42 * Ecran::echelleMin())
#define INTERVALLE_DEFILEMENT 0.150f

#include <sys/types.h>

typedef size_t uindex_t;
typedef ssize_t index_t;

#endif
