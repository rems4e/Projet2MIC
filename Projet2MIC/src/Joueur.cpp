//
//  Joueur.cpp
//  Projet2MIC
//
//  Created by Rémi Saurel on 11/02/12.
//  Copyright (c) 2012 Rémi Saurel. All rights reserved.
//

#include "Joueur.h"
#include "Session.h"

Joueur::Joueur(Niveau *n, uindex_t index, ElementNiveau::elementNiveau_t cat) : Personnage(n, index, cat) {
	
}

Joueur::~Joueur() {
	
}

void Joueur::afficher(Coordonnees const &decalage, double zoom) const {
	this->Personnage::afficher(decalage);
	Ecran::afficherRectangle(Rectangle((this->positionAffichage() + this->origine()) * zoom - decalage, Coordonnees(10, 10)), Couleur::rouge);
}

void Joueur::animer(horloge_t tempsEcoule) {
	Personnage::animer(tempsEcoule);
	
	Coordonnees dep;
	if(Session::evenement(Session::T_DROITE))
		dep += Coordonnees(1, 1);
	else if(Session::evenement(Session::T_GAUCHE))
		dep += Coordonnees(-1, -1);
	if(Session::evenement(Session::T_BAS))
		dep += Coordonnees(-1, 1);
	else if(Session::evenement(Session::T_HAUT))
		dep += Coordonnees(1, -1);
	
	if(!dep.vecteurNul()) {
		if(this->definirAction(EntiteMobile::a_deplacer)) {
			dep.normaliser();
			dep *= this->vitesse();
			this->deplacerPosition(dep);
		}
	}
	else {
		if(Session::evenement(Session::T_ESPACE)) {
			this->definirAction(EntiteMobile::a_attaquer);
			Session::reinitialiser(Session::T_ESPACE);
		}
		else if(Session::evenement(Session::T_m)) {
			this->definirAction(EntiteMobile::a_mourir);
		}
		else
			this->definirAction(EntiteMobile::a_immobile);
	}
}

void Joueur::interagir(Personnage *p) {
	
}

bool Joueur::joueur() const {
	return true;
}

