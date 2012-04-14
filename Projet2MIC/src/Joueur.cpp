//
//  Joueur.cpp
//  Projet2MIC
//
//  Created by Rémi Saurel on 11/02/12.
//  Copyright (c) 2012 Rémi Saurel. All rights reserved.
//

#include "Joueur.h"
#include "Session.h"

Joueur::Joueur(Niveau *n, uindex_t index, ElementNiveau::elementNiveau_t cat) : Personnage(n, index, cat, new InventaireJoueur(*this)), _inventaireAffiche(false) {
	
}

Joueur::~Joueur() {
	
}

void Joueur::afficher(index_t deltaX, index_t deltaY, Coordonnees const &decalage, double zoom) const {
	this->Personnage::afficher(deltaX, deltaY, decalage);
	Ecran::afficherRectangle(Rectangle((this->positionAffichage() + this->origine()) * zoom - decalage - Coordonnees(5, 5), Coordonnees(10, 10)), Couleur(255, 255, 0));
}

void Joueur::animer(horloge_t tempsEcoule) {
	Personnage::animer(tempsEcoule);
	
	Coordonnees dep;
	if(Session::evenement(Parametres::evenementAction(Parametres::depDroite)))
		dep += Coordonnees(1, 1);
	else if(Session::evenement(Parametres::evenementAction(Parametres::depGauche)))
		dep += Coordonnees(-1, -1);
	if(Session::evenement(Parametres::evenementAction(Parametres::depBas)))
		dep += Coordonnees(-1, 1);
	else if(Session::evenement(Parametres::evenementAction(Parametres::depHaut)))
		dep += Coordonnees(1, -1);
	
	if(!dep.vecteurNul()) {
		dep.normaliser();
		dep *= this->vitesse();
		if(this->definirAction(EntiteMobile::a_deplacer)) {
			this->deplacerPosition(dep);
		}
	}
	if(dep.vecteurNul()) {
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
	
	if(Session::evenement(Session::T_r)) {
		Niveau::listeElements_t liste = this->niveau()->elements(this->pX(), this->pY(), Niveau::cn_objetsInventaire);
		for(Niveau::elements_t::iterator el = liste.first; el != liste.second; ++el) {
			if(this->inventaire()->ajouterObjet(static_cast<ObjetInventaire *>(el->first))) {
				this->niveau()->supprimerElement(this->pX(), this->pY(), Niveau::cn_objetsInventaire, el, false);
			}
		}
		Session::reinitialiser(Session::T_r);
	}
}

void Joueur::interagir(Personnage *p) {
	
}

bool Joueur::joueur() const {
	return true;
}

EntiteMobile::categorie_t Joueur::type() const {
	return EntiteMobile::em_joueur;
}

bool Joueur::inventaireAffiche() const {
	return _inventaireAffiche;
}

void Joueur::definirInventaireAffiche(bool af) {
	_inventaireAffiche = af;
}
