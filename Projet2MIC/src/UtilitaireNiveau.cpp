//
//  UtilitaireNiveau.cpp
//  Projet2MIC
//
//  Créé par Marc Promé et Rémi Saurel.
//  Ce fichier et son contenu sont librement distribuables, modifiables et utilisables pour toute œuvre non commerciale, à condition d'en citer les auteurs.
//

#include "UtilitaireNiveau.h"
#include "Niveau.h"

uint16_t entite(ElementNiveau::elementNiveau_t cat, index_t index) {
	uint16_t retour = 0;
	retour |= (cat << 9);
	retour |= index;
	retour |= 0x8000;
	
	return retour;
}

uint16_t entite(index_t loiProbabilite) {
	uint16_t retour = 0;
	retour |= loiProbabilite;
	
	return retour;
}

void obtenirInfosEntites(uint16_t valeur, bool &proba, index_t &indexProba, ElementNiveau::elementNiveau_t &cat, index_t &index) {
	proba = false;
	indexProba = 0;
	cat = ElementNiveau::nbTypesElement;
	index = 0;
	
	proba = !(valeur & 0x8000);
	if(proba) {
		indexProba = valeur;
	}
	else {
		cat = static_cast<ElementNiveau::elementNiveau_t>((valeur ^ 0x8000) >> 9);
		index = valeur & 0x01FF;
	}
}

Coordonnees referentielNiveauVersEcran(Coordonnees const &pos) {
	return Coordonnees(pos.x + pos.y, (pos.y - pos.x) / 2) / 2;
}

Coordonnees referentielEcranVersNiveau(Coordonnees const &pos) {
	return Coordonnees(pos.x - 2 * pos.y, pos.x + 2 * pos.y);
}
