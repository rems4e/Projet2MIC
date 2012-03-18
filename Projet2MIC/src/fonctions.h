/*
 *  fonctions.h
 *  Jeu C++
 *
 *  Created by Rémi on 24/06/09.
 *  Copyright 2009 Rémi Saurel. All rights reserved.
 *
 */
 
#ifndef EN_TETE_FONCTIONS
#define EN_TETE_FONCTIONS

#include <string>

template <typename T>
T signe(T const &t) {
	return (t > 0)  ? 1 : ((t < 0) ? -1 : 0);
}

// Nombre aléatoire entre 0 et (nombreMax - 1)
int nombreAleatoire(int nombreMax);

template <typename T> 
T nombreAleatoire(T nombreMax) {
	return static_cast<T>(nombreAleatoire(static_cast<int>(nombreMax)));
}

int caractereVersHexa(char c);
int caractereVersBase64(char c);
double texteVersNombre(char const *texte);
double texteVersNombre(std::string const &texte);
std::string nombreVersTexte(double nombre, int decimales);

inline std::string nombreVersTexte(float nombre, int decimales) { return nombreVersTexte(static_cast<double>(nombre), decimales); }
inline std::string nombreVersTexte(int nombre) { return nombreVersTexte(static_cast<double>(nombre), 0); }
inline std::string nombreVersTexte(unsigned int nombre) { return nombreVersTexte(static_cast<double>(nombre), 0); }
inline std::string nombreVersTexte(long nombre) { return nombreVersTexte(static_cast<double>(nombre), 0); }
inline std::string nombreVersTexte(unsigned long nombre) { return nombreVersTexte(static_cast<double>(nombre), 0); }

int nombreDecimales(double nombre);

void quitter(int code);

#endif
