/*
 *  fonctions.cpp
 *  Jeu C++
 *
 *  Created by Rémi on 23/07/07.
 *  Copyright 2007 Rémi Saurel. All rights reserved.
 *
 */

#include "fonctions.h"
#include "Session.h"
#include "Ecran.h"
#include "SDL/SDL.h"

#include "Image.h"
#include "Texte.h"

#include <cmath>
#include <cstring>
#include <cstdlib>
#include <cstdio>

#include "Constantes.h"
#include "Geometrie.h"

#include <algorithm>

int nombreAleatoire(int nombreMax) {
	if(nombreMax == 0)
		return 0;
	
	int nb = std::rand();
	nb %= nombreMax;
	
	return nb;
}

double arrondi(double n) {
	return n < 0.0 ? std::ceil(n - 0.5) : std::floor(n + 0.5);
}

float arrondi(float n) {
	return n < 0.0f ? std::ceil(n - 0.5f) : std::floor(n + 0.5f);
}

int caractereVersHexa(char c) {
	switch(c) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			return c - '0';
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
			return c - 'a' + 10;
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
			return c - 'A' + 10;
		default:
			return 0;
	}
}

char hexaVersCaractere(int c) {
	if(c < 10)
		return c + '0';
	else
		return c - 10 + 'A';
}

int caractereVersBase64(char c) {
	if(c >= 'A' && c <= 'Z')
		return c - 'A';
	if(c >= 'a' && c <= 'z')
		return c - 'a' + 26;
	if(c >= '0' && c <= '9')
		return c - '0' + 26 + 26;
	if(c == '+')
		return 26 + 26 + 10;
	if(c == '/')
		return 26 + 26 + 10 + 1;
	
	return 0;
}

char base64VersCaractere(int c) {
	if(c < 0)
		return 0;
	if(c < 26)
		return 'A' + c;
	if(c < 26 * 2)
		return 'a' + c;
	if(c < 26 * 2 + 10)
		return '0' + c;
	if(c == 26 * 2 + 10)
		return '+';
	if(c == 26 * 2 + 10 + 1)
		return '/';
	
	return 0;
}

double texteVersNombre(char const *texte) {
	long l = std::strlen(texte);
	char *txt = (char *)std::malloc(l + 1);
	std::strcpy(txt, texte);
	
	for(int i = 0; i < l; ++i) {
		if(txt[i] == ',')
			txt[i] = '.';
	}
	
	double d = std::strtod(txt, 0);
	std::free(txt);
	
	return d;
}

double texteVersNombre(std::string const &texte) { return texteVersNombre(texte.c_str()); }

std::string nombreVersTexte(double nombre, int decimales) {
	if(decimales == -1)
		decimales = nombreDecimales(nombre);
	
	char *texte;
	asprintf(&texte, "%.*lf", decimales, nombre);
	
	std::string retour(texte);
	std::free(texte);

	return retour;
}

int nombreDecimales(double nombre) {
	int nb = 0;
	while(nombre != trunc(nombre) && nb <= 6) {
		nombre *= 10;
		++nb;
	}
	
	return nb;
}
