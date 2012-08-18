/*
 *  CaseACocher.cpp
 *  Jeu C++
 *
 *  Created by Rémi on 10/05/08.
 *  Copyright 2008 Rémi Saurel. All rights reserved.
 *
 */

#include "Bouton.h"
#include "Affichage.h"

static Image caseACocher;

CaseACocher::CaseACocher(glm::vec2 const &pos, std::string const &txt, bool enfonce, Couleur const &c) : Bouton(pos, txt) {
	this->definirValeurBool(enfonce);
	_texte.definir(txt, POLICE_NORMALE, 14);
	_texte.definir(c);
	this->definirCadre(Rectangle(pos, glm::vec2(24 + 4 + _texte.dimensions().x, 24)));
	this->definirComportement(Bouton::on_off);
}

CaseACocher::~CaseACocher() {

}

#pragma mark -
#pragma mark Accesseurs

#pragma mark -
#pragma mark Méthodes

void CaseACocher::dessiner() {
	Rectangle filtre;
	glm::vec2 d(0);
	
	Couleur t = Affichage::teinte();
	Affichage::ajouterOpacite(255);
	if(!_actif) {
		Affichage:: supprimerOpacite();
		Affichage::ajouterOpacite(128);
	}
	else if(_enfonce || (_clic && this->cadreAbsolu().contientPoint(Session::souris()))) { // clic
		Affichage::definirTeinte(Couleur(0, 100));
	}
	
	if(this->valeurIndeterminee()) {
		filtre.gauche = 0;
		filtre.haut = 0;
		filtre.hauteur = 22;
		filtre.largeur = 22;
		d.y = 2;
	}
	else if(_valeur) { // enfoncé
		filtre.gauche = 44;
		filtre.haut = 0;
		filtre.hauteur = 24;
		filtre.largeur = 23;
	}
	else { // état normal
		filtre.gauche = 22;
		filtre.haut = 0;
		filtre.hauteur = 22;
		filtre.largeur = 22;
		d.y = 2;
	}
		
	if(!caseACocher.chargee())
		caseACocher = Image(Session::cheminRessources() + "caseACocher.png");
	
	caseACocher.afficher(d, filtre);
	Affichage::definirTeinte(t);
	Affichage::supprimerOpacite();
	_texte.afficher(glm::vec2(filtre.largeur + 4 + !_valeur, (24 - _texte.dimensions().y) / 2));
}
