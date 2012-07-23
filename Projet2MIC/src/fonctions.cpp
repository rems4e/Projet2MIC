//
//  fonctions.cpp
//  Projet2MIC
//
//  Créé par Marc Promé et Rémi Saurel.
//  Ce fichier et son contenu sont librement distribuables, modifiables et utilisables pour toute œuvre non commerciale, à condition d'en citer les auteurs.
//


#include "fonctions.h"

void decouperChaine(std::string c, std::string const &separateur, std::vector<std::string> &sousChaines) {
	size_t pos = c.find_first_of(separateur);
	while(pos != std::string::npos) {
		if(pos > 0) {
			sousChaines.push_back(c.substr(0, pos));
		}
		c = c.substr(pos + 1);
		pos = c.find_first_of(separateur);
	}
	if(c.length() > 0) {
		sousChaines.push_back(c);
	}
}
