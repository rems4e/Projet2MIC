//
//  Editeur.cpp
//  Projet2MIC
//
//  Created by Rémi Saurel on 11/03/12.
//  Copyright (c) 2012 Rémi Saurel. All rights reserved.
//

#include "Editeur.h"
#include "Session.h"
#include "Ecran.h"
#include "ElementNiveau.h"
#include "tinyxml.h"
#include "UtilitaireNiveau.h"
#include <cstring>
#include "Texte.h"

Editeur *Editeur::_editeur = 0;
Rectangle Editeur::_cadreControles;
Rectangle Editeur::_cadreEditeur;
Rectangle Editeur::_cadreInventaire;

Editeur *Editeur::editeur() {
	if(_editeur == 0) {
		_editeur = new Editeur;
	}
	
	return _editeur;
}

Editeur::Editeur() {
	
}

Editeur::~Editeur() {
	Editeur::_editeur = 0;
}

void Editeur::editerNiveau(std::string const &fichier) {
	_niveau = new NiveauEditeur(fichier);
	_continuer = true;
	_origine = Coordonnees(0, 0);
	_coucheEdition = static_cast<Niveau::couche_t>(Niveau::nb_couches - 1);
	_ancienRectangle = 0;
	_selection = Rectangle::aucun;
	
	bool sauvegarder = true;
	while(Session::boucle(60, _continuer)) {
		Ecran::effacer();
		this->afficher();
		Ecran::maj();
		
		if(!Session::evenement(Session::B_GAUCHE)) {
			_ancienRectangle = 0;
			_selection = Rectangle::aucun;
		}
		else if(_ancienRectangle == 0) {
			if(Session::souris() < Editeur::cadreEditeur())
				_ancienRectangle = &Editeur::cadreEditeur();
			else if(Session::souris() < Editeur::cadreControles())
				_ancienRectangle = &Editeur::cadreControles();
			else if(Session::souris() < Editeur::cadreInventaire())
				_ancienRectangle = &Editeur::cadreInventaire();
		}
		
		if(_ancienRectangle == &Editeur::cadreInventaire())
			this->sourisInventaire();
		else if(_ancienRectangle == &Editeur::cadreControles())
			this->sourisControles();
		else if(_ancienRectangle == &Editeur::cadreEditeur())
			this->sourisEditeur();
		
		if(Session::evenement(Session::QUITTER)) {
			_continuer = false;
		}
		else {
			if(Session::evenement(Session::T_GAUCHE))
				_origine.x -= 10;
			else if(Session::evenement(Session::T_DROITE))
				_origine.x += 10;
			if(Session::evenement(Session::T_HAUT))
				_origine.y -= 10;
			else if(Session::evenement(Session::T_BAS))
				_origine.y += 10;
		}
	}
	
	if(sauvegarder)
		this->enregistrer();
	
	delete _niveau;
}

void Editeur::afficher() {
	for(Niveau::couche_t c = Niveau::premierCouche; c != Niveau::nb_couches; ++c) {
		if(c == _coucheEdition)
			continue;
		this->afficherCouche(c);
	}
	
	Ecran::afficherRectangle(Editeur::cadreEditeur(), Couleur(0, 0, 0, 128));
	
	this->afficherCouche(_coucheEdition);

	this->afficherGrille();
	
	this->afficherInterface();
}

void Editeur::afficherCouche(Niveau::couche_t couche) {
	Coordonnees pos;
	for(Ligne::iterator i = _niveau->_elements.begin(); i != _niveau->_elements.end(); ++i) {
		pos.x = 0;
		for(Colonne::iterator j = i->begin(); j != i->end(); ++j) {
			ElementEditeur const *elem = (*j)[couche];
			if(elem) {
				Coordonnees posAffichage = referentielNiveauVersEcran(pos) - elem->origine() - _origine + Coordonnees(Editeur::cadreEditeur()) + Coordonnees(0, Ecran::hauteur() / 2);
				elem->image().afficher(posAffichage, elem->cadre());
			}
			pos.x += LARGEUR_CASE;
		}
		pos.y += LARGEUR_CASE;
	}
}

void Editeur::afficherGrille() {
	Coordonnees decalage = -Coordonnees(0, Ecran::hauteur() / 2);
	for(index_t y = 0; y <= _niveau->_dimY; ++y) {
		Ecran::afficherLigne(referentielNiveauVersEcran(Coordonnees(0, y) * LARGEUR_CASE) - _origine + Coordonnees(Editeur::cadreEditeur()) - decalage, referentielNiveauVersEcran(Coordonnees(_niveau->_dimX, y) * LARGEUR_CASE) - _origine + Coordonnees(Editeur::cadreEditeur()) - decalage, Couleur::rouge, 1.0);
	}
	for(index_t x = 0; x <= _niveau->_dimX; ++x) {
		Ecran::afficherLigne(referentielNiveauVersEcran(Coordonnees(x, 0) * LARGEUR_CASE) - _origine + Coordonnees(Editeur::cadreEditeur()) - decalage, referentielNiveauVersEcran(Coordonnees(x, _niveau->_dimY) * LARGEUR_CASE) - _origine + Coordonnees(Editeur::cadreEditeur()) - decalage, Couleur::rouge, 1.0);
	}	
}

void Editeur::afficherInterface() {
	if(_selection != Rectangle::aucun) {
		Rectangle r = _selection;
		if(r.largeur < 0) {
			r.gauche += r.largeur;
			r.largeur *= -1;
		}
		if(r.hauteur < 0) {
			r.haut += r.hauteur;
			r.hauteur *= -1;
		}
		
		Coordonnees dec = Coordonnees(cadreEditeur()) + Coordonnees(0, Ecran::hauteur() / 2);
		
		Coordonnees p1 = dec + referentielNiveauVersEcran(Coordonnees(r.gauche, r.haut));
		Coordonnees p2 = dec + referentielNiveauVersEcran(Coordonnees(r.gauche + r.largeur, r.haut));
		Coordonnees p3 = dec + referentielNiveauVersEcran(Coordonnees(r.gauche + r.largeur, r.haut + r.hauteur));
		Coordonnees p4 = dec + referentielNiveauVersEcran(Coordonnees(r.gauche, r.haut + r.hauteur));
		
		//std::cout << _selection << std::endl;
		Ecran::afficherQuadrilatere(p1, p2, p3, p4, Couleur(200, 205, 220, 128));
	}
	
	this->afficherControles();
	this->afficherInventaire();
}

void Editeur::afficherInventaire() {
	Rectangle cadre = Editeur::cadreInventaire();
	Ecran::afficherRectangle(cadre, Couleur::noir);
	Ecran::afficherRectangle(Rectangle(cadre.gauche + 1, cadre.haut + 1, cadre.largeur - 2, cadre.hauteur - 2), Couleur(220, 225, 240));
}

void Editeur::afficherControles() {
	Rectangle cadre = Editeur::cadreControles();
	Ecran::afficherRectangle(cadre, Couleur::noir);
	Ecran::afficherRectangle(Rectangle(cadre.gauche + 1, cadre.haut + 1, cadre.largeur - 2, cadre.hauteur - 2), Couleur(220, 225, 240));
	
	Texte cc(Niveau::nomCouche(_coucheEdition));
	cc.definir(Couleur::noir);
	cc.afficher(Coordonnees(Editeur::cadreControles()) + Coordonnees(10, 10));
}

void Editeur::sourisEditeur() {
	Coordonnees point(Session::souris() - Editeur::cadreEditeur() - _origine - Coordonnees(0, Ecran::hauteur() / 2));
	Coordonnees pointNiveau(referentielEcranVersNiveau(point));
	//std::cout << pointNiveau << std::endl;
	if(_selection == Rectangle::aucun) {
		_selection = Rectangle(pointNiveau, Coordonnees());
	}
	else {
		_selection.largeur = pointNiveau.x - _selection.gauche;		
		_selection.hauteur = pointNiveau.y - _selection.haut;
	}
}

void Editeur::sourisControles() {
	
}

void Editeur::sourisInventaire() {
	
}

void Editeur::enregistrer() {
	
}

Rectangle const &Editeur::cadreEditeur() const {
	if(_cadreEditeur.estVide())
		Editeur::initCadres();
	
	return _cadreEditeur;
}

Rectangle const &Editeur::cadreControles() const {
	if(_cadreControles.estVide())
		Editeur::initCadres();
	
	return _cadreControles;
}

Rectangle const &Editeur::cadreInventaire() const {
	if(_cadreInventaire.estVide())
		Editeur::initCadres();
	
	return _cadreInventaire;
}

void Editeur::initCadres() {
	_cadreControles = Rectangle(0, 0, Ecran::largeur(), 100);
	_cadreInventaire = Rectangle(0, 0, 200, Ecran::hauteur() - _cadreControles.hauteur);
	_cadreControles.haut = _cadreInventaire.hauteur;
	_cadreEditeur = Rectangle(_cadreInventaire.largeur, 0, Ecran::largeur() - _cadreInventaire.largeur, Ecran::hauteur() - _cadreControles.hauteur);
}

Editeur::NiveauEditeur::NiveauEditeur(std::string const &fichier) : _fichier(fichier), _dimX(0), _dimY(0) {
	TiXmlDocument niveau(Session::cheminRessources() + fichier);
	if(!niveau.LoadFile()) {
		throw Exc_ChargementEditeur(std::string() + "Erreur de l'ouverture du fichier de niveau (" + (Session::cheminRessources() + fichier) + ".");
	}
	
	TiXmlElement *n = niveau.FirstChildElement("Niveau");
	n->Attribute("dimX", &_dimX);
	n->Attribute("dimY", &_dimY);
	
	_elements.resize(_dimY);
	for(Ligne::iterator i = _elements.begin(); i != _elements.end(); ++i) {
		i->resize(_dimX);
	}
	
	if(_dimX <= 0 || _dimY <= 0) {
		throw Exc_ChargementEditeur("Dimensions du niveau invalides !");
	}

	for(Niveau::couche_t couche = Niveau::premierCouche; couche < Niveau::nb_couches; ++couche) {
		TiXmlElement *cc = n->FirstChildElement(Niveau::nomCouche(couche));
		if(!cc)
			continue;
		char const *texte = cc->GetText();
		size_t tailleAttendue = _dimY * (_dimX * 4 + 1) - 1; // chaque ligne contient 4 chiffres hexa par colonne et un saut de ligne, sauf pour la dernière ligne.
		if(!texte || std::strlen(texte) != tailleAttendue) {
			throw Exc_ChargementEditeur(std::string() + "Erreur : dimensions de la couche " + nombreVersTexte(couche) + " invalides (" + nombreVersTexte(std::strlen(texte)) + " au lieu de " + nombreVersTexte(tailleAttendue) +  " attendus) !");
		}
		
		index_t pos = 0;
		for(index_t y = 0; y < _dimY; ++y) {
			for(index_t x = 0; x < _dimX; ++x, pos += 4) {
				uint16_t valeur = (caractereVersHexa(texte[pos]) << 12) + (caractereVersHexa(texte[pos + 1]) << 8) + (caractereVersHexa(texte[pos + 2]) << 4) + caractereVersHexa(texte[pos + 3]);
				ElementNiveau::elementNiveau_t categorie;
				index_t index;
				bool proba;
				index_t indexProba;
				obtenirInfosEntites(valeur, proba, indexProba, categorie, index);
				
				ElementEditeur *e = 0;
				if(!proba) { // Valeur déterminée de la case
					try {
						e = new ElementEditeur(index, categorie);
					}
					catch(ElementNiveau::Exc_EntiteIndefinie &e) {
						std::cerr << "L'entité (cat : " << categorie << " ; index : " << index << ") appelée dans le fichier de niveau \"" << fichier << "\" (couche : " << Niveau::nomCouche(couche) << " ; x : " << x << " ; y : " << y << ") n'est pas définie !" << std::endl;
						throw;
					}
				}
				else { // Bit de poids fort à 0 -> valeur probabiliste de la case
					if(indexProba == 0) { // Aucun entité présente dans la case
						
					}
					else { // choix d'une entité au hasard en fonction de la loi de probabilité indiquée
						--indexProba;
						e = new ElementEditeur(indexProba);
					}
				}
				
				if(e) {
					_elements[y][x][couche] = e;
				}
			}
			
			// Saut de ligne.
			pos += 1;
		}
	}
}

Editeur::NiveauEditeur::~NiveauEditeur() {
	for(Niveau::couche_t c = Niveau::premierCouche; c != Niveau::nb_couches; ++c) {
		for(Ligne::iterator i = _elements.begin(); i != _elements.end(); ++i) {
			for(Colonne::iterator j = i->begin(); j != i->end(); ++j) {
				delete (*j)[c];
			}
		}
	}
}


Editeur::Case::Case() {
	std::memset(_contenu, 0, sizeof(ElementEditeur *) * Niveau::nb_couches);
}

Editeur::ElementEditeur * &Editeur::Case::operator[](Niveau::couche_t c) {
	return _contenu[c];
}

Editeur::ElementEditeur::ElementEditeur(index_t index, ElementNiveau::elementNiveau_t cat) : _proba(false), _index(index), _categorie(cat), _image(), _cadre() {
	TiXmlElement *e = ElementNiveau::description(index, cat);
	_image = Image(Session::cheminRessources() + e->Attribute("image"));
	_cadre = Rectangle(Coordonnees(), _image.dimensions());
	
	if(e->Attribute("x"))
		e->Attribute("x", &_origine.x);
	if(e->Attribute("y"))
		e->Attribute("y", &_origine.y);
}

Editeur::ElementEditeur::ElementEditeur(index_t loiProba) : _proba(true), _indexProba(loiProba), _image(Session::cheminRessources() + "alea.png"), _cadre() {
	_cadre = Rectangle(Coordonnees(), _image.dimensions());
	_origine.x = 0;
	_origine.y = _cadre.hauteur / 2;
}

uint16_t Editeur::ElementEditeur::operator()() const {
	if(_proba) {
		return entite(_indexProba);
	}
	else {
		return entite(_categorie, _index);
	}
}

Image const &Editeur::ElementEditeur::image() const {
	return _image;
}

Rectangle Editeur::ElementEditeur::cadre() const {
	return _cadre;
}

Coordonnees Editeur::ElementEditeur::origine() const {
	return _origine;
}

