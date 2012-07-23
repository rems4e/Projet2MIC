/*
 *  Unichar.h
 *
 *  Created by Rémi on 19/07/12.
 *  Copyright 2011 Rémi Saurel. All rights reserved.
 *
 */

#ifndef EN_TETE_NOMBRE
#define EN_TETE_NOMBRE

#include <string>
#include <sstream>
#include <float.h>
#include <cmath>

template <typename T>
inline T signe(T const &t) {
	return (t > 0)  ? 1 : ((t < 0) ? -1 : 0);
}

// Nombre aléatoire entre 0 et (nombreMax - 1)
int nombreAleatoire(int nombreMax);

template <typename T>
inline bool comprisEntre(T valeur, T borneInf, T borneSup) {
	return valeur >= borneInf && valeur <= borneSup;
}

int caractereVersHexa(char c);
char hexaVersCaractere(int c);
int caractereVersBase64(char c);
char base64VersCaractere(int c);

double texteVersNombre(std::string const &texte);

template<typename Nombre>
inline int nombreDecimales(Nombre nombre) {
	return 0;
}

template<>
inline int nombreDecimales(double nombre) {
	if(std::abs(nombre - std::trunc(nombre)) < DBL_EPSILON)
		return 0;
	return 1 + nombreDecimales(nombre * 10.0);
}

template<>
inline int nombreDecimales(float nombre) {
	if(std::abs(nombre - std::trunc(nombre)) < FLT_EPSILON)
		return 0;
	return 1 + nombreDecimales(nombre * 10.0f);
}

template<typename Nombre>
inline std::string nombreVersTexte(Nombre nombre, int decimales) {	
	std::stringstream s;
	s.precision(decimales);
	s << std::fixed << nombre;
	
	return s.str();
}

template<typename Nombre>
inline std::string nombreVersTexte(Nombre nombre) {
	std::stringstream s;
	s << nombre;
	
	return s.str();
}

template <unsigned int val>
struct nombreChiffres {
	enum {nb = nombreChiffres<val / 10 + 1>::nb + 1};
};

template <>
struct nombreChiffres<1> {
	enum {nb = 1};
};

template <>
struct nombreChiffres<2> {
	enum {nb = 1};
};
template <>
struct nombreChiffres<3> {
	enum {nb = 1};
};
template <>
struct nombreChiffres<4> {
	enum {nb = 1};
};
template <>
struct nombreChiffres<5> {
	enum {nb = 1};
};
template <>
struct nombreChiffres<6> {
	enum {nb = 1};
};
template <>
struct nombreChiffres<7> {
	enum {nb = 1};
};
template <>
struct nombreChiffres<8> {
	enum {nb = 1};
};
template <>
struct nombreChiffres<9> {
	enum {nb = 1};
};
template <>
struct nombreChiffres<0> {
	enum {nb = 1};
};

#endif
