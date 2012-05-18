//
//  Joueur.cpp
//  Projet2MIC
//
//  Created by Rémi Saurel on 11/02/12.
//  Copyright (c) 2012 Rémi Saurel. All rights reserved.
//

#include "Joueur.h"
#include "Session.h"
#include "Partie.h"
#include "Marchand.h"
#include "UtilitaireNiveau.h"
#include "tinyxml.h"

Joueur::Joueur(bool decoupagePerspective, Niveau *n, uindex_t index, ElementNiveau::elementNiveau_t cat) : Personnage(decoupagePerspective, n, index, cat, new InventaireJoueur(*this)), _inventaireAffiche(false) {
	this->inventaire()->modifierMonnaie(123);
}

Joueur::~Joueur() {
	
}

void Joueur::afficher(index_t deltaY, Coordonnees const &decalage) const {
	this->Personnage::afficher(deltaY, decalage);
	//Ecran::afficherRectangle(Rectangle(referentielNiveauVersEcran(Coordonnees(this->pX(), this->pY()) * LARGEUR_CASE) - decalage - Coordonnees(5, 5), Coordonnees(10, 10)), Couleur(255, 255, 0));
}

void Joueur::animer() {
	this->Personnage::animer();
	
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
		if(Session::evenement(Parametres::evenementAction(Parametres::interagir))) {
			this->Personnage::interagir();
			Session::reinitialiser(Parametres::evenementAction(Parametres::interagir));
		}
		else if(Session::evenement(Session::T_m)) {
			this->modifierVieActuelle(-this->vieTotale());
		}
		else
			this->definirAction(EntiteMobile::a_immobile);
	}
	
	if(Session::evenement(Parametres::evenementAction(Parametres::ramasserObjet))) {
		index_t x = this->pX(), y = this->pY();
		this->inventaire()->modifierMonnaie(this->niveau()->monnaie(x, y));
		this->niveau()->modifierMonnaie(x, y, -this->niveau()->monnaie(x, y));
		
		Niveau::listeElements_t liste = this->niveau()->elements(this->pX(), this->pY(), Niveau::cn_objetsInventaire);
		for(Niveau::elements_t::iterator el = liste.first; el != liste.second;) {
			if(this->inventaire()->ajouterObjet(static_cast<ObjetInventaire *>(el->entite))) {
				el = this->niveau()->supprimerElement(el, Niveau::cn_objetsInventaire, false);
			}
			else {
				++el;
			}
		}
		Session::reinitialiser(Parametres::evenementAction(Parametres::ramasserObjet));
	}
}

bool Joueur::interagir(Personnage *p) {
	switch(p->categorieMobile()) {
		case EntiteMobile::em_ennemi:
			this->attaquer(p);
			return true;
		case EntiteMobile::em_marchand:
			Partie::partie()->definirMarchand(static_cast<Marchand *>(p));
		case EntiteMobile::em_joueur:
			break;
	}
	
	return false;
}

void Joueur::mourir() {
	Personnage::mourir();
}

bool Joueur::joueur() const {
	return true;
}

void Joueur::jeterObjets() {
	InventaireJoueur *e = static_cast<InventaireJoueur *>(this->inventaire());
	std::list<ObjetInventaire *> liste;
	index_t pX = this->pX(), pY = this->pY();
	for(InventaireJoueur::iterator i = e->debut(); i != e->fin(); ++i) {
		if(*i) {
			liste.push_back(*i);
			this->niveau()->ajouterElement(pX, pY, Niveau::cn_objetsInventaire, *i);
		}
	}
	e->vider();	
}

EntiteMobile::categorie_t Joueur::categorieMobile() const {
	return EntiteMobile::em_joueur;
}

bool Joueur::inventaireAffiche() const {
	return _inventaireAffiche;
}

void Joueur::definirInventaireAffiche(bool af) {
	_inventaireAffiche = af;
}

void Joueur::renaitre() {
	this->Personnage::renaitre();
}

TiXmlElement *Joueur::sauvegarde() const {
	TiXmlElement *el = new TiXmlElement("Joueur");
	
	el->SetAttribute("vie", (int)this->vieActuelle());
	
	TiXmlElement *inv = static_cast<InventaireJoueur const *>(this->inventaire())->sauvegarde();
	el->InsertEndChild(*inv);
	delete inv;
	
	TiXmlElement *comp = this->competences().sauvegarde();
	el->InsertEndChild(*comp);
	delete comp;
	
	for(positionTenue_t p = premierePositionTenue; p != nbPositionsTenue; ++p) {
		TiXmlElement *objet = new TiXmlElement("Tenue" + nombreVersTexte(p));
		if(this->tenue(p)) {
			TiXmlElement *comp = this->tenue(p)->competencesRequises().sauvegarde();
			objet->InsertEndChild(*comp);
			delete comp;
			objet->SetAttribute("index", this->tenue(p)->index());
		}
		el->InsertEndChild(*objet);
		delete objet;
	}
	
	return el;
}

void Joueur::restaurer(TiXmlElement *sauvegarde) {
	TiXmlElement *el = sauvegarde->FirstChildElement("Joueur");
	static_cast<InventaireJoueur *>(this->inventaire())->restaurer(el);
	
	int vie;
	el->Attribute("vie", &vie);
	this->modifierVieActuelle(-this->vieActuelle() + vie);
	
	Competences comp;
	comp.restaurer(el);
	this->definirCompetences(comp);
	
	for(positionTenue_t p = premierePositionTenue; p != nbPositionsTenue; ++p) {
		TiXmlElement *objet = el->FirstChildElement("Tenue" + nombreVersTexte(p));
		if(objet && objet->Attribute("index")) {
			index_t i;
			objet->Attribute("index", &i);
			ObjetInventaire *obj = ElementNiveau::elementNiveau<ObjetInventaire>(false, this->niveau(), i);
			Personnage::Competences c;
			c.restaurer(objet);
			
			obj->definirCompetencesRequises(c);
			this->definirTenue(p, obj);
		}
	}
}

