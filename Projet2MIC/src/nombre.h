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
#include <cstdlib>

template <typename T>
inline T signe(T const &t) {
	return (t > 0)  ? 1 : ((t < 0) ? -1 : 0);
}

// Nombre aléatoire entre 0 et (nombreMax - 1)
//int nombreAleatoire(int nombreMax);
template <class T, size_t = sizeof(T) / sizeof(int)>
struct alea;

template <class T>
struct alea<T, 0> {
	T operator()(T nbMax) const {
		if(nbMax == 0)
			return 0;
		
		int nb = std::rand();
		nb %= nbMax;
		
		return nb;
	}
};

template <class T>
struct alea<T, 1> {
	T operator()(T nbMax) const {
		if(nbMax == 0)
			return 0;
		
		int nb = std::rand();
		nb %= nbMax;
		
		return nb;
	}
};


template <class T>
struct alea<T, 2> {
	T operator()(T nbMax) const {
		if(nbMax == 0)
			return 0;
		
		T nb = std::rand() + (std::rand() << (2 * 8));
		nb %= nbMax;
		
		return nb;
	}
};

template <class T>
T nombreAleatoire(T nbMax) {
	alea<T> nb;
	return nb(nbMax);
}

template<typename T1, typename T2, typename T3>
T1 clamp(T1 valeur, T2 min, T3 max) {
	return static_cast<T1>(valeur > max ? max : valeur < min ? min : valeur);
}

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
inline int nombreDecimales(Nombre) {
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

template<typename Retour>
inline Retour interpolationExponentielle(double borne1, double valeur1, double borne2, double valeur2, double position, int exposant) {
	return static_cast<Retour>(valeur1 + std::pow((position - borne1) / (borne2 - borne1), exposant) * valeur2);
}

template<typename T>
inline T interpolationLineaire(T borne1, T valeur1, T borne2, T valeur2, double position) {
	return valeur1 + (position - borne1) / (borne2 - borne1) * valeur2;
}

template<typename T>
inline T mix(T valeur1, T valeur2, double position) {
	return valeur1 * (1.0 - position) + valeur2 * position;
}

template<typename Retour>
inline Retour interpolationQuadratique(double borne1, double valeur1, double borne2, double valeur2, double position) {
	return interpolationExponentielle<Retour>(borne1, valeur1, borne2, valeur2, position, 2);
}

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
