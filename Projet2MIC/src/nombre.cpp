//
//  fonctions.cpp
//  Projet2MIC
//
//  Créé par Marc Promé et Rémi Saurel.
//  Ce fichier et son contenu sont librement distribuables, modifiables et utilisables pour toute œuvre non commerciale, à condition d'en citer les auteurs.
//


#include "nombre.h"

#include <cstdlib>
#include <cstring>

#include <algorithm>

int nombreAleatoire(int nombreMax) {
	if(nombreMax == 0)
		return 0;
	
	int nb = std::rand();
	nb %= nombreMax;
	
	return nb;
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
		return 'a' + c - 26;
	if(c < 26 * 2 + 10)
		return '0' + c - 26 - 26;
	if(c == 26 * 2 + 10)
		return '+';
	if(c == 26 * 2 + 10 + 1)
		return '/' ;
	
	return 0;
}

double texteVersNombre(std::string const &s) {
	std::string::size_type l = s.size();
	char *txt = (char *)std::malloc(l + 1);
	std::strcpy(txt, s.c_str());
	
	for(int i = 0; i < l; ++i) {
		if(txt[i] == ',')
			txt[i] = '.';
	}
	
	double d = std::strtod(txt, 0);
	std::free(txt);
	
	return d;
}
