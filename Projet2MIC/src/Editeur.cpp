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
#include "Menu.h"
#include <tr1/functional>
#include <algorithm>
#include <numeric>

#define TAILLE_MAX_NIVEAU 1000

static index_t choisirElement(std::vector<Unichar> const &elements, index_t selection, Unichar const &titre, Image *apercu = 0);

static char const *probaVierge = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";

Editeur *Editeur::_editeur = 0;
Rectangle Editeur::_cadreControles;
Rectangle Editeur::_cadreEditeur;
Rectangle Editeur::_cadreInventaire;
Rectangle Editeur::_cadreCarte;

Editeur *Editeur::editeur() {
	if(_editeur == 0) {
		_editeur = new Editeur;
	}
	
	return _editeur;
}

Editeur::Editeur() {
	_sauve = Image(Session::cheminRessources() + "save.png");
}

Editeur::~Editeur() {
	Editeur::_editeur = 0;
}

void Editeur::editerNiveau(std::string const &fichier) {
	_niveau = new NiveauEditeur(fichier);
	_continuer = true;
	_origine = -Coordonnees(0, Ecran::hauteur() / 2);
	_coucheEdition = static_cast<Niveau::couche_t>(Niveau::nbCouches - 1);
	_ancienRectangle = 0;
	_affichageSelection = Rectangle::aucun;
	_selection.clear();
	_modifie = false;
	
	std::vector<Unichar> elemMenus;
	elemMenus.push_back("Enregistrer le niveau");
	elemMenus.push_back("Revenir à la version enregistrée");	
	elemMenus.push_back("Quitter l'éditeur");
	
	Menu menuEditeur("Menu éditeur", elemMenus);
	
	while(Session::boucle(100, _continuer)) {
		Ecran::definirPointeurAffiche(true);
		Ecran::effacer();
		this->afficher();
		
		if(Session::evenement(Session::T_ESC)) {
			index_t retour = menuEditeur.afficher();
			if(retour < elemMenus.size()) {
				if(retour == 2) {
					_continuer = false;
				}
				else if(retour == 0) {
					this->enregistrer();
				}
				else if(retour == 1) {
					std::string f = _niveau->_fichier;
					delete _niveau;
					_niveau = new NiveauEditeur(f);
					_selection.clear();
					_modifie = false;
				}
			}
		}
		if(!Session::evenement(Session::B_GAUCHE)) {
			if(_ancienRectangle == &Editeur::cadreEditeur()) {
				for(selection_t::iterator i = _selection.begin(); i != _selection.end();) {
					if(i->_etat == es_sup)
						i = _selection.erase(i);
					else {
						i->_etat = es_ok;
						++i;
					}
				}
			}
			_ancienRectangle = 0;
			_affichageSelection = Rectangle::aucun;
		}
		else if(_ancienRectangle == 0) {
			if(Session::souris() < Editeur::cadreEditeur())
				_ancienRectangle = &Editeur::cadreEditeur();
			else if(Session::souris() < Editeur::cadreControles())
				_ancienRectangle = &Editeur::cadreControles();
			else if(Session::souris() < Editeur::cadreInventaire())
				_ancienRectangle = &Editeur::cadreInventaire();
			else if(Session::souris() < Editeur::cadreCarte())
				_ancienRectangle = &Editeur::cadreCarte();
		}
		
		if(_ancienRectangle == &Editeur::cadreInventaire())
			this->sourisInventaire();
		else if(_ancienRectangle == &Editeur::cadreControles())
			this->sourisControles();
		else if(_ancienRectangle == &Editeur::cadreEditeur())
			this->sourisEditeur();
		else if(_ancienRectangle == &Editeur::cadreCarte())
			this->sourisCarte();
		
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
			
			if(Session::evenement(Session::T_EFFACER)) {
				for(selection_t::iterator i = _selection.begin(); i != _selection.end(); ++i) {
					delete i->_e;
					_niveau->_elements[i->_posY][i->_posX][_coucheEdition] = 0;
				}
				
				if(!_selection.empty())
					this->modification();
				
				_selection.clear();
				_cadreSelection = Rectangle::aucun;
			}
		}
		
		if(!_continuer && _modifie) {
			std::vector<Unichar> txt;
			txt.push_back("Enregistrer");
			txt.push_back("Ne pas enregistrer");
			
			Menu m("Niveau modifié. Enregistrer ?", txt);
			index_t retour = m.afficher();
			if(retour == 0) {
				this->enregistrer();
			}
			else if(retour == 1) {
				_modifie = false;
			}
			else {
				Session::reinitialiserEvenements();
				_continuer = true;
			}
		}

		Ecran::maj();
	}
		
	delete _niveau;
}

void Editeur::afficher() {
	Ecran::afficherRectangle(Ecran::ecran(), Couleur::grisClair);
	for(Niveau::couche_t c = Niveau::premiereCouche; c != Niveau::nbCouches; ++c) {
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
	static float teinteSelection = 0;
	static int sensTeinte = 1;
	teinteSelection += 1.0f / 400.0f * (60.0f / Ecran::frequenceInstantanee()) * sensTeinte;
	if(teinteSelection > 1) {
		teinteSelection = 1;
		sensTeinte = -1;
	}
	else if(teinteSelection < 0) {
		teinteSelection = 0;
		sensTeinte = 1;
	}

	Coordonnees pos;
	for(Ligne::iterator i = _niveau->_elements.begin(); i != _niveau->_elements.end(); ++i) {
		pos.x = (_niveau->_dimX - 1) * LARGEUR_CASE;
		for(Colonne::reverse_iterator j = i->rbegin(); j != i->rend(); ++j) {
			ElementEditeur const *elem = (*j)[couche];
			if(elem) {
				Coordonnees posAffichage = referentielNiveauVersEcran(pos) - elem->origine() - _origine + Editeur::cadreEditeur().origine();
				Image::definirTeinte(elem->teinte());
				elem->image().afficher(posAffichage, elem->cadre());
			}
			pos.x -= LARGEUR_CASE;
		}
		pos.y += LARGEUR_CASE;
	}
	Image::definirTeinte(Couleur::blanc);
	
	if(couche == _coucheEdition) {
		for(selection_t::iterator i = _selection.begin(); i != _selection.end(); ++i) {
			if(i->_etat == es_sup)
				continue;
			dimension_t dimX = LARGEUR_CASE, dimY = LARGEUR_CASE;
			if(i->_e) {
				dimX = i->_e->dimensions().x;
				dimY = i->_e->dimensions().y;
			}
			Coordonnees pos(i->_posX * LARGEUR_CASE, i->_posY * LARGEUR_CASE);
			Coordonnees p1 = referentielNiveauVersEcran(pos) - _origine + Editeur::cadreEditeur().origine();
			Coordonnees p2 = referentielNiveauVersEcran(pos + Coordonnees(dimX, 0)) - _origine + Editeur::cadreEditeur().origine();
			Coordonnees p3 = referentielNiveauVersEcran(pos + Coordonnees(dimX, dimY)) - _origine + Editeur::cadreEditeur().origine();
			Coordonnees p4 = referentielNiveauVersEcran(pos + Coordonnees(0, dimY)) - _origine + Editeur::cadreEditeur().origine();
			Ecran::afficherQuadrilatere(p1, p2, p3, p4, Couleur(teinteSelection * 255, teinteSelection * 255, teinteSelection * 255, 128));
		}
	}
}

void Editeur::afficherGrille() {
	for(index_t y = 0; y <= _niveau->_dimY; ++y) {
		Ecran::afficherLigne(referentielNiveauVersEcran(Coordonnees(0, y) * LARGEUR_CASE) - _origine + Editeur::cadreEditeur().origine(), referentielNiveauVersEcran(Coordonnees(_niveau->_dimX, y) * LARGEUR_CASE) - _origine + Editeur::cadreEditeur().origine(), Couleur::rouge, 1.0);
	}
	for(index_t x = 0; x <= _niveau->_dimX; ++x) {
		Ecran::afficherLigne(referentielNiveauVersEcran(Coordonnees(x, 0) * LARGEUR_CASE) - _origine + Editeur::cadreEditeur().origine(), referentielNiveauVersEcran(Coordonnees(x, _niveau->_dimY) * LARGEUR_CASE) - _origine + Editeur::cadreEditeur().origine(), Couleur::rouge, 1.0);
	}
}

void Editeur::afficherInterface() {
	if(_affichageSelection != Rectangle::aucun) {
		Rectangle r = _affichageSelection;
		if(r.largeur < 0) {
			r.gauche += r.largeur;
			r.largeur *= -1;
		}
		if(r.hauteur < 0) {
			r.haut += r.hauteur;
			r.hauteur *= -1;
		}
		
		Coordonnees dec = cadreEditeur().origine() - _origine;

		Coordonnees p1 = dec + referentielNiveauVersEcran(Coordonnees(r.gauche, r.haut));
		Coordonnees p2 = dec + referentielNiveauVersEcran(Coordonnees(r.gauche + r.largeur, r.haut));
		Coordonnees p3 = dec + referentielNiveauVersEcran(Coordonnees(r.gauche + r.largeur, r.haut + r.hauteur));
		Coordonnees p4 = dec + referentielNiveauVersEcran(Coordonnees(r.gauche, r.haut + r.hauteur));
		
		Ecran::afficherQuadrilatere(p1, p2, p3, p4, Couleur(200, 205, 220, 128));
	}
	
	this->afficherControles();
	this->afficherInventaire();
	this->afficherCarte();
}

void Editeur::afficherInventaire() {
	_fonctionsInventaire.clear();

	Rectangle const &cadre = Editeur::cadreInventaire();
	Ecran::afficherRectangle(cadre, Couleur::noir);
	Ecran::afficherRectangle(Rectangle(cadre.gauche + 1, cadre.haut + 1, cadre.largeur - 2, cadre.hauteur - 2), Couleur(220, 225, 240));
	
	Rectangle rectCat(Editeur::cadreInventaire().origine() + Coordonnees(10, 10), Coordonnees());
	
	Texte cc;
	cc.definir(Couleur::noir);
		
	size_t tailleSelection = 0;
	if(_ancienRectangle == &Editeur::cadreEditeur()) {
		for(selection_t::iterator i = _selection.begin(); i != _selection.end(); ++i) {
			if(i->_etat != es_sup)
				++tailleSelection;
		}
	}
	else {
		tailleSelection = _selection.size();
	}

	if(tailleSelection > 1) {
		cc.definir("Entités sélectionnées :");
	}
	else if(tailleSelection == 1) {
		cc.definir("Entité sélectionnée :");
	}
	else {
		cc.definir("Aucune sélection");
	}
	rectCat.definirDimensions(cc.dimensions());
	cc.afficher(rectCat.origine());

	
	if(tailleSelection) {
		if(tailleSelection == 1) {
			rectCat.haut += rectCat.dimensions().y + 10;
			cc.definir("Sélectionner les entités\nidentiques");
			rectCat.definirDimensions(cc.dimensions());
			cc.afficher(rectCat.origine());
			
			_fonctionsInventaire.push_back(std::make_pair(rectCat, &Editeur::selectionnerSemblables));
		}
		
		selection_t::iterator i = _selection.begin();
		for(; i != _selection.end(); ++i) {
			if(i->_etat != es_sup)
				break;
		}
		bool diffProba = !i->_e, diffIndex = false, diffCategorie = false, diffIndexProba = false;
		bool proba;
		index_t index, indexProba;
		ElementNiveau::elementNiveau_t cat;
		uint16_t idEntite;
		if(i->_e) {
			idEntite = i->_e->operator()();
			obtenirInfosEntites(idEntite, proba, indexProba, cat, index);
			++i;
		}
		else
			i = _selection.end();
		
		for(; i != _selection.end(); ++i) {
			if(i->_etat == es_sup)
				continue;
			
			if(!i->_e) {
				diffProba = true;
				break;
			}
			idEntite = i->_e->operator()();
			bool proba1;
			index_t index1, indexProba1;
			ElementNiveau::elementNiveau_t cat1;
			obtenirInfosEntites(idEntite, proba1, indexProba1, cat1, index1);
			if(proba1 != proba) {
				diffProba = true;
				break;
			}
			else if(proba) {
				if(indexProba1 != indexProba) {
					diffIndexProba = true;
				}
			}
			else {
				if(index1 != index) {
					diffIndex = true;
				}
				if(cat1 != cat) {
					diffCategorie = true;
				}				
			}
		}
		
		if(diffProba) {
			rectCat.haut += rectCat.dimensions().y + 10;
			cc.definir("Sélection hétérogène");
			rectCat.definirDimensions(cc.dimensions());
			cc.afficher(rectCat.origine());
			
			_fonctionsInventaire.push_back(std::make_pair(rectCat, &Editeur::modifProba));
		}
		else {
			if(proba) {
				rectCat.haut += rectCat.dimensions().y + 10;
				cc.definir("Défini par :\nLoi de probabilité");
				rectCat.definirDimensions(cc.dimensions());
				cc.afficher(rectCat.origine());
				_fonctionsInventaire.push_back(std::make_pair(rectCat, &Editeur::modifProba));
				
				if(diffIndexProba) {
					rectCat.haut += rectCat.dimensions().y + 10;
					cc.definir("Lois différentes");
					rectCat.definirDimensions(cc.dimensions());
					cc.afficher(rectCat.origine());
				}
				else {
					rectCat.haut += rectCat.dimensions().y + 10;
					cc.definir("Loi n°" + nombreVersTexte(indexProba) + " (" + _niveau->_probas[indexProba]._nom + ")");
					rectCat.definirDimensions(cc.dimensions());
					cc.afficher(rectCat.origine());
				}
				_fonctionsInventaire.push_back(std::make_pair(rectCat, &Editeur::modifIndexProba));
			}
			else {
				rectCat.haut += rectCat.dimensions().y + 10;
				cc.definir("Défini par :\nCatégorie + index");
				rectCat.definirDimensions(cc.dimensions());
				cc.afficher(rectCat.origine());
				_fonctionsInventaire.push_back(std::make_pair(rectCat, &Editeur::modifProba));

				if(diffCategorie) {
					rectCat.haut += rectCat.dimensions().y + 10;
					cc.definir("Catégories différentes ");
					rectCat.definirDimensions(cc.dimensions());
					cc.afficher(rectCat.origine());
				}
				else {
					rectCat.haut += rectCat.dimensions().y + 10;
					cc.definir("Catégorie " + nombreVersTexte(cat));
					rectCat.definirDimensions(cc.dimensions());
					cc.afficher(rectCat.origine());
				}
				_fonctionsInventaire.push_back(std::make_pair(rectCat, &Editeur::modifCategorie));
				
				if(diffIndex) {
					rectCat.haut += rectCat.dimensions().y + 10;
					cc.definir("Index différents");
					rectCat.definirDimensions(cc.dimensions());
					cc.afficher(rectCat.origine());
				}
				else {
					rectCat.haut += rectCat.dimensions().y + 10;
					cc.definir("Index " + nombreVersTexte(index));
					rectCat.definirDimensions(cc.dimensions());
					cc.afficher(rectCat.origine());
				}
				_fonctionsInventaire.push_back(std::make_pair(rectCat, &Editeur::modifIndex));
			}
		}
	}
}

void Editeur::afficherControles() {
	_fonctionsControles.clear();
	Rectangle cadre = Editeur::cadreControles();
	Ecran::afficherRectangle(cadre, Couleur::noir);
	Ecran::afficherRectangle(Rectangle(cadre.gauche + 1, cadre.haut + 1, cadre.largeur - 2, cadre.hauteur - 2), Couleur(220, 225, 240));
	
	Rectangle rect(Editeur::cadreControles().origine() + Coordonnees(10, 10), Coordonnees());
	
	_sauve.afficher(rect.origine());
	rect.definirDimensions(_sauve.dimensions());
	_fonctionsControles.push_back(std::make_pair(rect, &Editeur::enregistrer));
	
	rect.haut += rect.hauteur + 10;
	
	Texte cc(std::string("Afficher la couche : \n") + Niveau::nomCouche(_coucheEdition));
	cc.definir(Couleur::noir);
	rect.definirDimensions(cc.dimensions());
	cc.afficher(rect.origine());
	_fonctionsControles.push_back(std::make_pair(rect, &Editeur::modifCouche));
	
	rect.haut += rect.hauteur + 10;
	cc.definir("Éditer les lois de proba");
	rect.definirDimensions(cc.dimensions());
	cc.afficher(rect.origine());
	_fonctionsControles.push_back(std::make_pair(rect, &Editeur::modifLoisProbas));

	rect.haut += rect.hauteur + 10;
	cc.definir("Éditer les dimensions\ndu niveau");
	rect.definirDimensions(cc.dimensions());
	cc.afficher(rect.origine());
	_fonctionsControles.push_back(std::make_pair(rect, &Editeur::modifDimensions));
}

void Editeur::afficherCarte() {
	Rectangle cadre = Editeur::cadreCarte();
	Ecran::afficherRectangle(cadre, Couleur::noir);
	Ecran::afficherRectangle(Rectangle(cadre.gauche + 1, cadre.haut + 1, cadre.largeur - 2, cadre.hauteur - 2), Couleur(220, 225, 240));

	dimension_t max = std::max(_niveau->_dimX, _niveau->_dimY);
	
	dimension_t dimX = cadre.largeur * _niveau->_dimX / max;
	dimension_t dimY = cadre.largeur * _niveau->_dimY / max;
	Coordonnees p1;
	Coordonnees p2(dimX, 0);
	Coordonnees p3(dimX, dimY);
	Coordonnees p4(0, dimY);
	p1 = referentielNiveauVersEcran(p1) + Coordonnees(cadre.gauche, cadre.haut + dimX / 4);
	p2 = referentielNiveauVersEcran(p2) + Coordonnees(cadre.gauche, cadre.haut + dimX / 4);
	p3 = referentielNiveauVersEcran(p3) + Coordonnees(cadre.gauche, cadre.haut + dimX / 4);
	p4 = referentielNiveauVersEcran(p4) + Coordonnees(cadre.gauche, cadre.haut + dimX / 4);
	
	Coordonnees dimensions(p3.x - p1.x, p4.y - p2.y);
	Coordonnees decalage = (cadre.dimensions() - dimensions) / 2;
	p1 += decalage;
	p2 += decalage;
	p3 += decalage;
	p4 += decalage;
	
	Ecran::afficherQuadrilatere(p1, p2, p3, p4, Couleur::gris);
	
	Rectangle aff;
	aff.definirOrigine(decalage + Coordonnees(cadre.gauche, cadre.haut + dimX / 4) + Coordonnees(_origine.x / (max * LARGEUR_CASE), _origine.y / (max * LARGEUR_CASE)) * cadre.largeur);
	aff.definirDimensions(Coordonnees(Editeur::cadreEditeur().largeur / (max * LARGEUR_CASE) * cadre.largeur, Editeur::cadreEditeur().hauteur / (max * LARGEUR_CASE) * cadre.largeur));
	aff = cadre.intersection(aff);
	
	Ecran::afficherRectangle(aff, Couleur(255, 0, 0, 128));
}

void Editeur::sourisEditeur() {
	Coordonnees point(Session::souris() - Editeur::cadreEditeur().origine() + _origine);
	Coordonnees pointNiveau(referentielEcranVersNiveau(point));

	if(_affichageSelection == Rectangle::aucun) {
		_affichageSelection = Rectangle(pointNiveau, Coordonnees());
	}
	else {
		_affichageSelection.largeur = pointNiveau.x - _affichageSelection.gauche;		
		_affichageSelection.hauteur = pointNiveau.y - _affichageSelection.haut;
	}
	_cadreSelection = _affichageSelection;
	if(_cadreSelection.largeur < 0) {
		_cadreSelection.gauche += _cadreSelection.largeur;
		_cadreSelection.largeur *= -1;
	}
	if(_cadreSelection.hauteur < 0) {
		_cadreSelection.haut += _cadreSelection.hauteur;
		_cadreSelection.hauteur *= -1;
	}
	
	if(!Session::modificateurTouches(Session::M_MAJ) && !Session::modificateurTouches(Session::M_COMMANDE) && !Session::modificateurTouches(Session::M_CONTROLE)) {
		_selection.clear();
	}
	for(selection_t::iterator i = _selection.begin(); i != _selection.end();) {
		if(i->_etat == es_aj)
			i = _selection.erase(i);
		else {
			if(i->_etat == es_sup)
				i->_etat = es_ok;
			++i;
		}
	}
	
	Coordonnees pos;
	for(Ligne::iterator i = _niveau->_elements.begin(); i != _niveau->_elements.end(); ++i) {
		pos.x = 0;
		for(Colonne::iterator j = i->begin(); j != i->end(); ++j) {
			ElementEditeur const *elem = (*j)[_coucheEdition];
			Coordonnees dim(elem ? elem->dimensions().x : LARGEUR_CASE, elem ? elem->dimensions().y : LARGEUR_CASE);
			if(Rectangle(pos, (Coordonnees(dim.x, dim.y))).superposition(_cadreSelection)) {
				ElementSelection elemS(elem, std::distance(i->begin(), j), std::distance(_niveau->_elements.begin(), i), es_aj);
				selection_t::iterator fnd = std::find(_selection.begin(), _selection.end(), elemS);
				if(fnd == _selection.end()) {
					_selection.push_back(elemS);
				}
				else
					fnd->_etat = es_sup;
			}
			pos.x += LARGEUR_CASE;
		}
		pos.y += LARGEUR_CASE;
	}
}

void Editeur::sourisControles() {
	for(listeFonctions_t::iterator i = _fonctionsControles.begin(); i != _fonctionsControles.end(); ++i) {
		if(Session::souris() < i->first) {
			(this->*(i->second))();
			Session::reinitialiser(Session::B_GAUCHE);
			break;
		}
	}
}

void Editeur::sourisInventaire() {
	for(listeFonctions_t::iterator i = _fonctionsInventaire.begin(); i != _fonctionsInventaire.end(); ++i) {
		if(Session::souris() < i->first) {
			(this->*(i->second))();
			Session::reinitialiser(Session::B_GAUCHE);
			break;
		}
	}
}

void Editeur::sourisCarte() {
	Rectangle cadre = Editeur::cadreCarte();
	dimension_t max = std::max(_niveau->_dimX, _niveau->_dimY);
	
	dimension_t dimX = cadre.largeur * _niveau->_dimX / max;
	dimension_t dimY = cadre.largeur * _niveau->_dimY / max;
	Coordonnees p1;
	Coordonnees p2(dimX, 0);
	Coordonnees p3(dimX, dimY);
	Coordonnees p4(0, dimY);
	p1 = referentielNiveauVersEcran(p1) + Coordonnees(cadre.gauche, cadre.haut + dimX / 4);
	p2 = referentielNiveauVersEcran(p2) + Coordonnees(cadre.gauche, cadre.haut + dimX / 4);
	p3 = referentielNiveauVersEcran(p3) + Coordonnees(cadre.gauche, cadre.haut + dimX / 4);
	p4 = referentielNiveauVersEcran(p4) + Coordonnees(cadre.gauche, cadre.haut + dimX / 4);
	
	Coordonnees dimensions(p3.x - p1.x, p4.y - p2.y);
	Coordonnees decalage = (cadre.dimensions() - dimensions) / 2;

	Coordonnees pos = Session::souris() - cadre.origine() - decalage - Coordonnees(0, (p1.y - p2.y));
	_origine = pos / cadre.largeur * max * LARGEUR_CASE;
}

void Editeur::enregistrer() {
	std::string valeurTexte;
	char const *documentBase = 
	"<?xml version=\"1.0\" standalone='no' >\n"
	"<Niveau>\n"
	"<proba>\n"
	"</proba>\n"
	"</Niveau>";
	TiXmlDocument *document = new TiXmlDocument(Session::cheminRessources() + _niveau->_fichier);
	document->Parse(documentBase);
	
	TiXmlElement* n = document->FirstChildElement("Niveau");// niveau
	
	n->SetAttribute("dimX", _niveau->_dimX);
	n->SetAttribute("dimY", _niveau->_dimY);
		
	
	TiXmlElement *probas = n->FirstChildElement("proba");
		
	for(std::vector<LoiProba>::iterator i = _niveau->_probas.begin(); i != _niveau->_probas.end(); ++i) {
		TiXmlElement p("proba");
		p.SetAttribute("nom", i->_nom);
		std::string valeur;
		for(ElementNiveau::elementNiveau_t cat = ElementNiveau::premierTypeElement; cat < ElementNiveau::nbTypesElement; ++cat) {
			valeur += base64VersCaractere(i->operator[](cat) / 64);
			valeur += base64VersCaractere(i->operator[](cat) % 64);
		}
		p.SetAttribute("valeur", valeur);
		
		probas->InsertEndChild(p);
	}
	
	for(Niveau::couche_t couche = Niveau::premiereCouche; couche < Niveau::nbCouches; ++couche) {
		TiXmlElement cc(Niveau::nomCouche(couche));
		std::string texte;
		texte.reserve(_niveau->_dimY * (_niveau->_dimX * 4 + 1) - 1);
		for(Ligne::iterator i = _niveau->_elements.begin(); i != _niveau->_elements.end(); ++i) {
			for(Colonne::iterator j = i->begin(); j != i->end(); ++j) {
				uint16_t val = 0;
				if(j->operator[](couche)) {
					val = j->operator[](couche)->operator()();
					bool proba;
					index_t indexProba, index;
					ElementNiveau::elementNiveau_t cat;
					obtenirInfosEntites(val, proba, indexProba, cat, index);
					if(proba)
						++val;
				}
				texte.push_back(hexaVersCaractere(val >> 12));
				texte.push_back(hexaVersCaractere((val >> 8) % 16));
				texte.push_back(hexaVersCaractere((val >> 4) % 16));
				texte.push_back(hexaVersCaractere(val % 16));
			}
			if(i != _niveau->_elements.end() - 1) {
				texte.push_back('\n');
			}
		}

		TiXmlText t(texte);
		cc.InsertEndChild(t);
		n->InsertEndChild(cc);
	}
	
	if(!document->SaveFile(Session::cheminRessources() + _niveau->_fichier))
		std::cout << "La sauvegarde du fichier de niveau " << Session::cheminRessources() + _niveau->_fichier << " a échoué. Vérifiez le chemin du fichier." << std::endl;
	delete document;
	
	_modifie = false;
}

void Editeur::modifIndexProba() {
	if(_selection.empty())
		return;
	
	std::vector<Unichar> cat;
	for(std::vector<LoiProba>::iterator i = _niveau->_probas.begin(); i != _niveau->_probas.end(); ++i) {
		cat.push_back(i->_nom);
	}
	
	index_t elem = choisirElement(cat, 0, "Choisissez une loi de probabilité :");
	
	if(elem != cat.size()) {
		bool modif = false;
		
		bool proba;
		index_t indexProba, index;
		ElementNiveau::elementNiveau_t cat;
		for(selection_t::iterator i = _selection.begin(); i != _selection.end(); ++i) {
			if(!i->_e)
				indexProba = elem + 1;
			else
				obtenirInfosEntites(i->_e->operator()(), proba, indexProba, cat, index);
			if(indexProba != elem) {
				ElementEditeur *nouveau = new ElementEditeur(elem);
				_niveau->_elements[i->_posY][i->_posX][_coucheEdition] = nouveau;
				delete i->_e;
				i->_e = nouveau;
				modif = true;
			}
		}
		
		if(modif)
			this->modification();
	}
}
void Editeur::modifLoisProbas() {
	std::vector<Unichar> lois;
	for(std::vector<LoiProba>::iterator i = _niveau->_probas.begin(); i != _niveau->_probas.end(); ++i) {
		lois.push_back("Loi " + nombreVersTexte(std::distance(_niveau->_probas.begin(), i)) + " (" + i->_nom + ")");
	}
	lois.push_back("Ajouter une loi");
	
	Image *fond = Ecran::apercu();
	index_t elem = choisirElement(lois, 0, "Choisissez une loi à modifier :", fond);
	
	if(elem == lois.size() - 1) {
		_niveau->_probas.push_back(LoiProba("proba" + nombreVersTexte(lois.size()), probaVierge));
		this->modification();
		this->editerLoiProba(_niveau->_probas.size() - 1, fond);
	}
	else if(elem != lois.size()) {
		this->editerLoiProba(elem, fond);
	}
	
	delete fond;
}

struct hauteurTexte_t {
	hauteurTexte_t(dimension_t ecart) : _ecart(ecart) {
		
	}
	
	dimension_t operator()(dimension_t somme, Texte const &t) {
		return somme + t.dimensions().y + _ecart;
	}
	
	dimension_t _ecart;
};

struct largeurTexte_t {
	bool operator()(Texte const &t1, Texte const &t2) {
		return t1.dimensions().x < t2.dimensions().x;
	}
};

struct afficheurTexte_t {
	afficheurTexte_t(Rectangle &cadre, dimension_t ecart) : _cadre(cadre), _ecart(ecart) {
		
	}
	
	void operator()(Texte const &t) {
		t.afficher(_cadre.origine());
		_cadre.hauteur = t.dimensions().y;
		_cadre.haut += _cadre.hauteur + _ecart;
	}
	
	Rectangle &_cadre;
	dimension_t _ecart;
};

struct initCadres_t {
	initCadres_t(std::vector<Rectangle> &cadres, Rectangle &cadre, dimension_t ecart) : _cadres(cadres), _cadre(cadre), _ecart(ecart) {
		
	}
	
	void operator()(Texte const &t) {
		_cadre.hauteur = t.dimensions().y;
		_cadres.push_back(_cadre);
		_cadre.haut += _cadre.hauteur + _ecart;
	}
	
	std::vector<Rectangle> &_cadres;
	Rectangle &_cadre;
	dimension_t _ecart;
};

struct trouveSouris_t {
	bool operator()(Rectangle const &r) {
		return Session::souris() < r;
	}
};

void Editeur::editerLoiProba(index_t loi, Image *fond) {
	bool continuer = true, supprimer = false;
	
	horloge_t ancienDefilement = 0;
	
	Texte titre("Édition de la loi " + nombreVersTexte(loi) + " (" + _niveau->_probas[loi]._nom + ")", POLICE_NORMALE, 20);
	Texte sup("Supprimer la loi", POLICE_NORMALE, 16);
	std::vector<Texte> categories;
	categories.reserve(ElementNiveau::nbTypesElement);
	for(ElementNiveau::elementNiveau_t i = ElementNiveau::premierTypeElement; i != ElementNiveau::nbTypesElement; ++i) {
		char const *cat = ElementNiveau::nomCategorie(i);
		if(!cat)
			break;
		categories.push_back(Texte(cat + std::string(" : "), POLICE_NORMALE, 16));
	}
	
	std::vector<Texte> probas;
	probas.reserve(categories.size());
	for(ElementNiveau::elementNiveau_t i = ElementNiveau::premierTypeElement; i != categories.size(); ++i) {
		int proba = _niveau->_probas[loi][i];
		probas.push_back(Texte(nombreVersTexte(proba), POLICE_NORMALE, 16));
	}
	
	std::vector<Rectangle> cadres;
	cadres.reserve(categories.size());

	ElementNiveau::elementNiveau_t selection = ElementNiveau::premierTypeElement;
	index_t premierAffiche = 0;
	size_t nbAffiches = std::min(size_t(10), categories.size());
	
	Image flou(fond->flou(1));

	float teinteSelection = 0;
	int sensTeinte = 1;

	while(Session::boucle(100.0f, continuer)) {
		teinteSelection += 1.0f / 50.0f * (60.0f / Ecran::frequenceInstantanee()) * sensTeinte;
		if(teinteSelection > 0.7f) {
			teinteSelection = 0.7f;
			sensTeinte = -1;
		}
		else if(teinteSelection < 0) {
			teinteSelection = 0;
			sensTeinte = 1;
		}
		
		
		categories[selection].definir(Couleur(teinteSelection * 255));
		probas[selection].definir(Couleur(teinteSelection * 255));

		Ecran::effacer();
		flou.afficher(Coordonnees());
		Ecran::afficherRectangle(Ecran::ecran(), Couleur(255, 255, 255, 200));
		
		Rectangle cadre(Coordonnees(80, 80), titre.dimensions()), cadreSup(Coordonnees(), sup.dimensions());
		titre.afficher(cadre.origine());
		
		cadre.haut += cadre.hauteur + 20;
		cadre.gauche += 20;
		
		dimension_t largeurCat = std::max_element(categories.begin() + premierAffiche, categories.begin() + premierAffiche + nbAffiches, largeurTexte_t())->dimensions().x;
		dimension_t largeurProba = std::max_element(probas.begin() + premierAffiche, probas.begin() + premierAffiche + nbAffiches, largeurTexte_t())->dimensions().x;

		dimension_t hauteur = std::accumulate(categories.begin() + premierAffiche, categories.begin() + premierAffiche + nbAffiches, dimension_t(0), hauteurTexte_t(10)) - 10;
		
		Ecran::afficherRectangle(Rectangle(cadre.gauche, cadre.haut, largeurCat + largeurProba + 20, hauteur + 20), Couleur(200, 205, 220, 128));
		
		cadre += Coordonnees(10, 10);
		coordonnee_t sauveHaut = cadre.haut;
		afficheurTexte_t afficheur(cadre, 10);
		for_each(categories.begin() + premierAffiche, categories.begin() + premierAffiche + nbAffiches, afficheur);
		cadre.haut = sauveHaut;
		cadre.gauche += largeurCat;
		for_each(probas.begin() + premierAffiche, probas.begin() + premierAffiche + nbAffiches, afficheur);
		categories[selection].definir(Couleur::noir);
		probas[selection].definir(Couleur::noir);
	
		cadres.clear();
		cadre.gauche -= largeurCat;
		cadre.largeur = largeurCat + largeurProba;
		cadre.haut = sauveHaut;
		initCadres_t initCadres(cadres, cadre, 10);
		for_each(categories.begin() + premierAffiche, categories.begin() + premierAffiche + nbAffiches, initCadres);

		cadre -= Coordonnees(10, -10);
		
		sup.afficher(cadre.origine());
		cadreSup.definirOrigine(cadre.origine());

		if(Session::evenement(Session::T_ESC) || Session::evenement(Session::QUITTER) || Session::evenement(Session::T_ENTREE)) {
			continuer = false;
		}
		else {
			if(Session::evenement(Session::T_HAUT) && horloge() - ancienDefilement > INTERVALLE_DEFILEMENT) {
				if(selection > 0) {
					selection = static_cast<ElementNiveau::elementNiveau_t>(selection - 1);
					if(selection < premierAffiche)
						--premierAffiche;
				}
				else {
					selection = static_cast<ElementNiveau::elementNiveau_t>(categories.size() - 1);
					if(categories.size() > nbAffiches)
						premierAffiche = categories.size() - nbAffiches;
				}
				
				ancienDefilement = horloge();
			}
			else if(Session::evenement(Session::T_BAS) && horloge() - ancienDefilement > INTERVALLE_DEFILEMENT) {
				if(selection < categories.size() - 1) {
					++selection;
					if(selection >= premierAffiche + nbAffiches)
						++premierAffiche;
				}
				else {
					selection = ElementNiveau::premierTypeElement;
					premierAffiche = 0;
				}
				
				ancienDefilement = horloge();
			}
			if(Session::evenement(Session::B_GAUCHE)) {
				std::vector<Rectangle>::iterator souris = std::find_if(cadres.begin(), cadres.end(), trouveSouris_t());
				if(souris != cadres.end()) {
					selection = static_cast<ElementNiveau::elementNiveau_t>(std::distance(cadres.begin(), souris));
				}
				else if(Session::souris() < cadreSup) {
					continuer = false;
					supprimer = true;
				}
			}
			if(Session::evenement(Session::T_EFFACER)) {
				_niveau->_probas[loi][selection] /= 10;
				probas[selection].definir(nombreVersTexte(_niveau->_probas[loi][selection]));
				Session::reinitialiser(Session::T_EFFACER);
				this->modification();
			}
			else {
				int nb = -1;
				for(Session::evenement_t e = Session::T_0; e <= Session::T_9; ++e) {
					if(Session::evenement(e)) {
						nb = e - Session::T_0;
						Session::reinitialiser(e);
						break;
					}
				}
				if(nb != -1) {
					int &p = _niveau->_probas[loi][selection];
					int p1 = p;
					p1 = p1 * 10 + nb;
					if(p1 < NB_VALEURS_PROBA_ENTITES) {
						p = p1;
						probas[selection].definir(nombreVersTexte(p));
						this->modification();
					}
				}
			}
		}
		
		Ecran::maj();
	}
	
	if(supprimer) {
		uint16_t idProba = entite(loi);
		for(Niveau::couche_t c = Niveau::premiereCouche; c != Niveau::nbCouches; ++c) {
			for(Ligne::iterator i = _niveau->_elements.begin(); i != _niveau->_elements.end(); ++i) {
				for(Colonne::iterator j = i->begin(); j != i->end(); ++j) {
					if(j->_contenu[c] && j->_contenu[c]->operator()() == idProba) {
						supprimer = false;
						goto finRecherche;
					}
				}
			}
		}
	finRecherche:
		if(supprimer) {
			_niveau->_probas.erase(_niveau->_probas.begin() + loi);
			this->modification();
		}
		else {
			std::vector<Unichar> elem;
			elem.push_back("OK");
			Menu m("La loi de probabilité est utilisée", elem);
			m.afficher(fond);
		}
	}
}

void Editeur::modifDimensions() {
	bool continuer = true;
	
	Texte titre("Redimensionnement du niveau", POLICE_NORMALE, 20);
	Texte ok("OK", POLICE_NORMALE, 16);
	std::vector<Texte> enTetes;
	enTetes.push_back(Texte("Largeur (X) : ", POLICE_NORMALE, 16));
	enTetes.push_back(Texte("Hauteur (Y) : ", POLICE_NORMALE, 16));
	
	std::vector<Texte> valeurs;
	valeurs.push_back(Texte(nombreVersTexte(_niveau->_dimX), POLICE_NORMALE, 16));
	valeurs.push_back(Texte(nombreVersTexte(_niveau->_dimY), POLICE_NORMALE, 16));
	
	std::vector<Rectangle> cadres;
	
	index_t selection = 0;
	
	Image *ap = Ecran::apercu();
	Image flou(ap->flou(1));
	
	float teinteSelection = 0;
	int sensTeinte = 1;
	
	size_t dimX = _niveau->_dimX, dimY = _niveau->_dimY;
	
	while(Session::boucle(100.0f, continuer)) {
		teinteSelection += 1.0f / 50.0f * (60.0f / Ecran::frequenceInstantanee()) * sensTeinte;
		if(teinteSelection > 0.7f) {
			teinteSelection = 0.7f;
			sensTeinte = -1;
		}
		else if(teinteSelection < 0) {
			teinteSelection = 0;
			sensTeinte = 1;
		}
		
		
		enTetes[selection].definir(Couleur(teinteSelection * 255));
		valeurs[selection].definir(Couleur(teinteSelection * 255));
		
		Ecran::effacer();
		flou.afficher(Coordonnees());
		Ecran::afficherRectangle(Ecran::ecran(), Couleur(255, 255, 255, 200));
		
		Rectangle cadre(Coordonnees(80, 80), titre.dimensions()), cadreOk(Coordonnees(), ok.dimensions());
		titre.afficher(cadre.origine());
		
		cadre.haut += cadre.hauteur + 20;
		cadre.gauche += 20;
		
		dimension_t largeurCat = std::max_element(enTetes.begin(), enTetes.end(), largeurTexte_t())->dimensions().x;
		dimension_t largeurProba = std::max_element(valeurs.begin(), valeurs.end(), largeurTexte_t())->dimensions().x;
		
		dimension_t hauteur = std::accumulate(enTetes.begin(), enTetes.end(), dimension_t(0), hauteurTexte_t(10)) - 10;
		
		Ecran::afficherRectangle(Rectangle(cadre.gauche, cadre.haut, largeurCat + largeurProba + 20, hauteur + 20), Couleur(200, 205, 220, 128));
		
		cadre += Coordonnees(10, 10);
		coordonnee_t sauveHaut = cadre.haut;
		afficheurTexte_t afficheur(cadre, 10);
		for_each(enTetes.begin(), enTetes.end(), afficheur);
		cadre.haut = sauveHaut;
		cadre.gauche += largeurCat;
		for_each(valeurs.begin(), valeurs.end(), afficheur);
		
		cadres.clear();
		cadre.gauche -= largeurCat;
		cadre.largeur = largeurCat + largeurProba;
		cadre.haut = sauveHaut;
		initCadres_t initCadres(cadres, cadre, 10);
		for_each(enTetes.begin(), enTetes.end(), initCadres);
		
		cadre -= Coordonnees(10, -10);
		
		ok.afficher(cadre.origine());
		cadreOk.definirOrigine(cadre.origine());
		
		if(Session::evenement(Session::T_ESC) || Session::evenement(Session::QUITTER) || Session::evenement(Session::T_ENTREE)) {
			continuer = false;
		}
		else {
			if(Session::evenement(Session::B_GAUCHE)) {
				std::vector<Rectangle>::iterator souris = std::find_if(cadres.begin(), cadres.end(), trouveSouris_t());
				if(souris != cadres.end()) {
					enTetes[selection].definir(Couleur::noir);
					valeurs[selection].definir(Couleur::noir);
					selection = static_cast<ElementNiveau::elementNiveau_t>(std::distance(cadres.begin(), souris));
				}
				else if(Session::souris() < cadreOk) {
					continuer = false;
				}
			}
			if(Session::evenement(Session::T_EFFACER)) {
				switch(selection) {
					case 0:
						dimX /= 10;
						valeurs[selection].definir(nombreVersTexte(dimX));
						break;
					case 1:	
						dimY /= 10;
						valeurs[selection].definir(nombreVersTexte(dimY));
						break;
					default:
						break;
				}
				Session::reinitialiser(Session::T_EFFACER);
			}
			else {
				int nb = -1;
				for(Session::evenement_t e = Session::T_0; e <= Session::T_9; ++e) {
					if(Session::evenement(e)) {
						nb = e - Session::T_0;
						Session::reinitialiser(e);
						break;
					}
				}
				if(nb != -1) {
					size_t &p = selection ? dimY : dimX;
					size_t p1 = p;
					p1 = p1 * 10 + nb;
					if(p1 <= TAILLE_MAX_NIVEAU) {
						p = p1;
						valeurs[selection].definir(nombreVersTexte(p));
					}
				}
			}
		}
		
		Ecran::maj();
	}
	
	if(dimX != _niveau->_dimX || dimY != _niveau->_dimY) {
		bool redim = true;
		if(dimX < _niveau->_dimX || dimY < _niveau->_dimY) {
			std::vector<Unichar> elem;
			elem.push_back("Continuer");
			Menu m("Des éléments seront supprimés", elem);
			index_t reponse = m.afficher(ap);
			if(reponse != 0)
				redim = false;
		}
		
		if(redim) {
			for(Ligne::iterator i = _niveau->_elements.begin(); i != _niveau->_elements.end(); ++i) {
				for(Colonne::iterator j = i->begin(); j != i->end(); ++j) {
					for(Niveau::couche_t c = Niveau::premiereCouche; c != Niveau::nbCouches; ++c) {
						if(std::distance(_niveau->_elements.begin(), i) >= dimY || std::distance(i->begin(), j) >= dimX) {
							delete j->operator[](c);
						}
					}
				}
			}
			_niveau->_elements.resize(dimY);
			for(Ligne::iterator i = _niveau->_elements.begin(); i != _niveau->_elements.end(); ++i) {
				i->resize(dimX);
			}
			_niveau->_dimX = dimX;
			_niveau->_dimY = dimY;
			
			this->modification();
		}
	}
	
	delete ap;
}

void Editeur::modifIndex() {
	if(_selection.empty())
		return;

	bool continuer = true;
	
	horloge_t ancienDefilement = 0;
	
	index_t selection = 0;
	ElementNiveau::elementNiveau_t catSel;
	{
		bool proba;
		index_t indexProba;
		selection_t::iterator deb = _selection.begin();
		while(deb != _selection.end()) {
			if(deb->_e) {
				obtenirInfosEntites(deb->_e->operator()(), proba, indexProba, catSel, selection);
				break;
			}
		}
	}
	
	Texte titre("Choisissez un index :", POLICE_NORMALE, 20);
	std::vector<Texte> index;
	index.reserve(ElementNiveau::nbTypesElement);
	for(index_t i = 0; i != ElementNiveau::nombreEntites(catSel); ++i) {
		index.push_back(Texte(nombreVersTexte(i), POLICE_NORMALE, 16));
	}
	
	std::vector<Rectangle> cadres;
	cadres.reserve(index.size());
	
	index_t premierAffiche = 0;
	size_t nbAffiches = std::min(size_t(10), index.size());
	
	Image *ap = Ecran::apercu();
	Image flou(ap->flou(1));
	
	float teinteSelection = 0;
	int sensTeinte = 1;
	
	while(Session::boucle(100.0f, continuer)) {
		teinteSelection += 1.0f / 50.0f * (60.0f / Ecran::frequenceInstantanee()) * sensTeinte;
		if(teinteSelection > 0.7f) {
			teinteSelection = 0.7f;
			sensTeinte = -1;
		}
		else if(teinteSelection < 0) {
			teinteSelection = 0;
			sensTeinte = 1;
		}
		
		
		index[selection].definir(Couleur(teinteSelection * 255));
		
		Ecran::effacer();
		flou.afficher(Coordonnees());
		Ecran::afficherRectangle(Ecran::ecran(), Couleur(255, 255, 255, 200));
		
		Rectangle cadre(Coordonnees(80, 80), titre.dimensions());
		titre.afficher(cadre.origine());
		
		cadre.haut += cadre.hauteur + 20;
		cadre.gauche += 20;
		
		dimension_t largeur = std::max_element(index.begin() + premierAffiche, index.begin() + premierAffiche + nbAffiches, largeurTexte_t())->dimensions().x;
		
		dimension_t hauteur = std::accumulate(index.begin() + premierAffiche, index.begin() + premierAffiche + nbAffiches, dimension_t(0), hauteurTexte_t(10)) - 10;
		
		Coordonnees pCadre(cadre.gauche + cadre.largeur + 20, cadre.haut);
		Ecran::afficherRectangle(Rectangle(cadre.gauche, cadre.haut, largeur + 20, hauteur + 20), Couleur(200, 205, 220, 128));
		
		cadre += Coordonnees(10, 10);
		coordonnee_t sauveHaut = cadre.haut;
		afficheurTexte_t afficheur(cadre, 10);
		for_each(index.begin() + premierAffiche, index.begin() + premierAffiche + nbAffiches, afficheur);
		cadre.haut = sauveHaut;
		index[selection].definir(Couleur::noir);
		
		cadres.clear();
		cadre.largeur = largeur;
		cadre.haut = sauveHaut;
		initCadres_t initCadres(cadres, cadre, 10);
		for_each(index.begin() + premierAffiche, index.begin() + premierAffiche + nbAffiches, initCadres);
		
		cadre -= Coordonnees(10, -10);
		
		ElementEditeur *apercu = new ElementEditeur(catSel, selection);
		apercu->image().afficher(pCadre, apercu->cadre());
		delete apercu;
		
				
		if(Session::evenement(Session::T_ESC) || Session::evenement(Session::QUITTER) || Session::evenement(Session::T_ENTREE)) {
			continuer = false;
		}
		else {
			if(Session::evenement(Session::T_HAUT) && horloge() - ancienDefilement > INTERVALLE_DEFILEMENT) {
				if(selection > 0) {
					--selection;
					if(selection < premierAffiche)
						--premierAffiche;
				}
				else {
					selection = index.size() - 1;
					if(index.size() > nbAffiches)
						premierAffiche = index.size() - nbAffiches;
				}
				
				ancienDefilement = horloge();
			}
			else if(Session::evenement(Session::T_BAS) && horloge() - ancienDefilement > INTERVALLE_DEFILEMENT) {
				if(selection < index.size() - 1) {
					++selection;
					if(selection >= premierAffiche + nbAffiches)
						++premierAffiche;
				}
				else {
					selection = ElementNiveau::premierTypeElement;
					premierAffiche = 0;
				}
				
				ancienDefilement = horloge();
			}
			if(Session::evenement(Session::B_GAUCHE)) {
				std::vector<Rectangle>::iterator souris = std::find_if(cadres.begin(), cadres.end(), trouveSouris_t());
				if(souris != cadres.end()) {
					selection = std::distance(cadres.begin(), souris);
				}
			}
		}
		
		Ecran::maj();
	}
		
	bool modif = false;
	
	bool proba;
	index_t indexProba, idx;
	ElementNiveau::elementNiveau_t cat;
	for(selection_t::iterator i = _selection.begin(); i != _selection.end(); ++i) {
		if(!i->_e)
			idx = selection + 1;
		else
			obtenirInfosEntites(i->_e->operator()(), proba, indexProba, cat, idx);
		if(idx != selection) {
			ElementEditeur *nouveau = new ElementEditeur(catSel, selection);
			_niveau->_elements[i->_posY][i->_posX][_coucheEdition] = nouveau;
			delete i->_e;
			i->_e = nouveau;
			modif = true;
		}
	}
		
	if(modif)
		this->modification();

		
	delete ap;
}

static index_t choisirElement(std::vector<Unichar> const &el, index_t sel, Unichar const &tt, Image *apercu) {
	bool continuer = true;
	
	horloge_t ancienDefilement = 0;
	
	index_t selection = sel;
	
	Texte titre(tt, POLICE_NORMALE, 20);
	std::vector<Texte> elements;
	elements.reserve(el.size());
	for(index_t i = 0; i != el.size(); ++i) {
		elements.push_back(Texte(el[i], POLICE_NORMALE, 16));
	}
	
	std::vector<Rectangle> cadres;
	cadres.reserve(elements.size());
	
	index_t premierAffiche = 0;
	size_t nbAffiches = std::min(size_t(10), elements.size());
	
	Image *ap = apercu ? apercu : Ecran::apercu();
	Image flou(ap->flou(1));
	
	float teinteSelection = 0;
	int sensTeinte = 1;
	
	while(Session::boucle(100.0f, continuer)) {
		teinteSelection += 1.0f / 50.0f * (60.0f / Ecran::frequenceInstantanee()) * sensTeinte;
		if(teinteSelection > 0.7f) {
			teinteSelection = 0.7f;
			sensTeinte = -1;
		}
		else if(teinteSelection < 0) {
			teinteSelection = 0;
			sensTeinte = 1;
		}
		
		
		elements[selection].definir(Couleur(teinteSelection * 255));
		
		Ecran::effacer();
		flou.afficher(Coordonnees());
		Ecran::afficherRectangle(Ecran::ecran(), Couleur(255, 255, 255, 200));
		
		Rectangle cadre(Coordonnees(80, 80), titre.dimensions());
		titre.afficher(cadre.origine());
		
		cadre.haut += cadre.hauteur + 20;
		cadre.gauche += 20;
		
		dimension_t largeur = std::max_element(elements.begin() + premierAffiche, elements.begin() + premierAffiche + nbAffiches, largeurTexte_t())->dimensions().x;
		
		dimension_t hauteur = std::accumulate(elements.begin() + premierAffiche, elements.begin() + premierAffiche + nbAffiches, dimension_t(0), hauteurTexte_t(10)) - 10;
		
		Coordonnees pCadre(cadre.gauche + cadre.largeur + 20, cadre.haut);
		Ecran::afficherRectangle(Rectangle(cadre.gauche, cadre.haut, largeur + 20, hauteur + 20), Couleur(200, 205, 220, 128));
		
		cadre += Coordonnees(10, 10);
		coordonnee_t sauveHaut = cadre.haut;
		afficheurTexte_t afficheur(cadre, 10);
		for_each(elements.begin() + premierAffiche, elements.begin() + premierAffiche + nbAffiches, afficheur);
		cadre.haut = sauveHaut;
		elements[selection].definir(Couleur::noir);
		
		cadres.clear();
		cadre.largeur = largeur;
		cadre.haut = sauveHaut;
		initCadres_t initCadres(cadres, cadre, 10);
		for_each(elements.begin() + premierAffiche, elements.begin() + premierAffiche + nbAffiches, initCadres);
		
		cadre -= Coordonnees(10, -10);
		
		if(Session::evenement(Session::T_ESC) || Session::evenement(Session::QUITTER)) {
			continuer = false;
			selection = elements.size();
		}
		else if(Session::evenement(Session::T_ENTREE)) {
			continuer = false;
		}
		else {
			if(Session::evenement(Session::T_HAUT) && horloge() - ancienDefilement > INTERVALLE_DEFILEMENT) {
				if(selection > 0) {
					--selection;
					if(selection < premierAffiche)
						--premierAffiche;
				}
				else {
					selection = elements.size() - 1;
					if(elements.size() > nbAffiches)
						premierAffiche = elements.size() - nbAffiches;
				}
				
				ancienDefilement = horloge();
			}
			else if(Session::evenement(Session::T_BAS) && horloge() - ancienDefilement > INTERVALLE_DEFILEMENT) {
				if(selection < elements.size() - 1) {
					++selection;
					if(selection >= premierAffiche + nbAffiches)
						++premierAffiche;
				}
				else {
					selection = ElementNiveau::premierTypeElement;
					premierAffiche = 0;
				}
				
				ancienDefilement = horloge();
			}
			if(Session::evenement(Session::B_GAUCHE)) {
				std::vector<Rectangle>::iterator souris = std::find_if(cadres.begin(), cadres.end(), trouveSouris_t());
				if(souris != cadres.end()) {
					selection = std::distance(cadres.begin(), souris);
					continuer = false;
				}
			}
		}
		
		Ecran::maj();
	}
	
	if(!apercu)
		delete ap;
	
	return selection;
}

void Editeur::modifCouche() {	
	std::vector<Unichar> couches;
	for(Niveau::couche_t c = Niveau::premiereCouche; c != Niveau::nbCouches; ++c) {
		couches.push_back(Niveau::nomCouche(c));
	}
	
	index_t selection = choisirElement(couches, _coucheEdition, "Choisissez une couche :");
	if(selection != couches.size()) {
		_coucheEdition = static_cast<Niveau::couche_t>(selection);
	}
	
	_selection.clear();
	_cadreSelection = Rectangle::aucun;
}

void Editeur::modifProba() {
	if(_selection.empty())
		return;
	
	std::vector<Unichar> txt;
	txt.push_back("Loi de probabilité");
	txt.push_back("Catégorie + index");
		
	char const *titre = 0;
	if(_selection.size() == 1)
		titre = "L'entité est définie par:";
	else
		titre = "Les entités sont définies par :";

	index_t selection = choisirElement(txt, 0, titre);

	if(selection != txt.size()) {
		bool modif = false;
		
		if(selection == 0) { // Proba
			bool proba;
			index_t indexProba, index;
			ElementNiveau::elementNiveau_t cat;
			for(selection_t::iterator i = _selection.begin(); i != _selection.end(); ++i) {
				if(!i->_e)
					proba = false;
				else
					obtenirInfosEntites(i->_e->operator()(), proba, indexProba, cat, index);
				if(!proba) {
					ElementEditeur *nouveau = new ElementEditeur(0); // loi de proba 0
					_niveau->_elements[i->_posY][i->_posX][_coucheEdition] = nouveau;
					delete i->_e;
					i->_e = nouveau;
					modif = true;
				}
			}
		}
		else if(selection == 1) { // Catégorie + index
			bool proba;
			index_t indexProba, index;
			ElementNiveau::elementNiveau_t cat;
			for(selection_t::iterator i = _selection.begin(); i != _selection.end(); ++i) {
				if(!i->_e)
					proba = true;
				else
					obtenirInfosEntites(i->_e->operator()(), proba, indexProba, cat, index);
				if(proba) {
					ElementEditeur *nouveau = new ElementEditeur(ElementNiveau::premierTypeElement, 0); // catégorie 1, index 1
					_niveau->_elements[i->_posY][i->_posX][_coucheEdition] = nouveau;
					delete i->_e;
					i->_e = nouveau;
					modif = true;
				}
			}
		}
		if(modif)
			this->modification();
		
	}
}

void Editeur::modifCategorie() {
	if(_selection.empty())
		return;
	
	std::vector<Unichar> cat;
	for(ElementNiveau::elementNiveau_t c = ElementNiveau::premierTypeElement; c != ElementNiveau::nbTypesElement; ++c) {
		if(ElementNiveau::nombreEntites(c))
			cat.push_back(ElementNiveau::nomCategorie(c));
	}
	
	index_t selection = choisirElement(cat, 0, "Choisissez une catégorie :");
		
	if(selection != cat.size()) {
		bool modif = false;
		
		ElementNiveau::elementNiveau_t nCat = static_cast<ElementNiveau::elementNiveau_t>(selection);
		bool proba;
		index_t indexProba, index;
		ElementNiveau::elementNiveau_t cat;
		for(selection_t::iterator i = _selection.begin(); i != _selection.end(); ++i) {
			if(!i->_e)
				cat = nCat + 1;
			else
				obtenirInfosEntites(i->_e->operator()(), proba, indexProba, cat, index);
			if(cat != nCat) {
				ElementEditeur *nouveau = new ElementEditeur(nCat, 0); // index 1
				_niveau->_elements[i->_posY][i->_posX][_coucheEdition] = nouveau;
				delete i->_e;
				i->_e = nouveau;
				modif = true;
			}
		}
		
		if(modif)
			this->modification();
	}
}

void Editeur::selectionnerSemblables() {
	ElementEditeur const *modele = _selection.front()._e;
	index_t pX = _selection.front()._posX, pY = _selection.front()._posY;
	Coordonnees pos;
	for(Ligne::iterator i = _niveau->_elements.begin(); i != _niveau->_elements.end(); ++i) {
		pos.x = 0;
		for(Colonne::iterator j = i->begin(); j != i->end(); ++j) {
			if((modele == 0 && j->operator[](_coucheEdition) == 0 && (std::distance(_niveau->_elements.begin(), i) != pY || std::distance(i->begin(), j) != pX)) || (modele && j->operator[](_coucheEdition) && modele != j->operator[](_coucheEdition) && j->operator[](_coucheEdition)->operator()() == modele->operator()())) {
				_selection.push_back(ElementSelection(j->operator[](_coucheEdition), std::distance(i->begin(), j), std::distance(_niveau->_elements.begin(), i), es_ok));
			}
			pos.x += LARGEUR_CASE;
		}
		pos.y += LARGEUR_CASE;
	}
}

void Editeur::modification() {
	_modifie = true;
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

Rectangle const &Editeur::cadreCarte() const {
	if(_cadreCarte.estVide())
		Editeur::initCadres();
	
	return _cadreCarte;
}

void Editeur::initCadres() {
	_cadreControles = Rectangle(0, 0, 200, 160);
	_cadreCarte = Rectangle(0, 0, _cadreControles.largeur * 3 / 2, _cadreControles.largeur * 3 / 4);
	_cadreInventaire = Rectangle(0, _cadreControles.haut + _cadreControles.hauteur - 1, _cadreControles.largeur, Ecran::hauteur() - _cadreControles.hauteur - _cadreCarte.hauteur + 2);
	_cadreEditeur = Rectangle(_cadreControles.gauche + _cadreControles.largeur, 0, Ecran::largeur() - _cadreControles.largeur, Ecran::hauteur());
	_cadreCarte.haut = _cadreInventaire.haut + _cadreInventaire.hauteur - 1;
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


	{
		TiXmlElement *probas = n->FirstChildElement("proba");

		for(TiXmlElement *proba = probas->FirstChildElement(); proba; proba = proba->NextSiblingElement()) {			
			std::string valeur = proba->Attribute("valeur");
			std::string nom;
			if(proba->Attribute("nom"))
				nom = proba->Attribute("nom");
			_probas.push_back(LoiProba(nom, valeur));
		}
	}

	for(Niveau::couche_t couche = Niveau::premiereCouche; couche < Niveau::nbCouches; ++couche) {
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
						e = new ElementEditeur(categorie, index);
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
	for(Niveau::couche_t c = Niveau::premiereCouche; c != Niveau::nbCouches; ++c) {
		for(Ligne::iterator i = _elements.begin(); i != _elements.end(); ++i) {
			for(Colonne::iterator j = i->begin(); j != i->end(); ++j) {
				delete (*j)[c];
			}
		}
	}
}


Editeur::Case::Case() {
	std::memset(_contenu, 0, sizeof(ElementEditeur *) * Niveau::nbCouches);
}

Editeur::ElementEditeur * &Editeur::Case::operator[](Niveau::couche_t c) {
	return _contenu[c];
}

int &Editeur::LoiProba::operator[](ElementNiveau::elementNiveau_t c) {
	return _proba[c];
}

Editeur::ElementEditeur::ElementEditeur(ElementNiveau::elementNiveau_t cat, index_t index) : _proba(false), _index(index), _categorie(cat), _image(), _cadre(), _teinte(Couleur::blanc) {
	TiXmlElement *e = ElementNiveau::description(index, cat);
	_image = Image(Session::cheminRessources() + e->Attribute("image"));
	_dimensions.x = _dimensions.y = LARGEUR_CASE;
	
	_cadre = Rectangle(Coordonnees(), _image.dimensions());

	if(e->Attribute("x"))
		e->Attribute("x", &_origine.x);
	if(e->Attribute("y"))
		e->Attribute("y", &_origine.y);
	
	bool multi = false;
	TiXmlElement *el = e->FirstChildElement();
	if(el) {
		if(el->Attribute("nbPoses"))
			multi = true;
	}
	if(multi) {
		if(e->Attribute("dimX")) {
			e->Attribute("dimX", &_dimensions.x);
		}
		if(e->Attribute("dimY")) {
			e->Attribute("dimY", &_dimensions.y);
		}
		_cadre.definirDimensions(_dimensions);
		_dimensions = Coordonnees(LARGEUR_CASE, LARGEUR_CASE);

		_origine = Coordonnees(_dimensions.x / 2, _cadre.hauteur * 3 / 4);
	}
	else {
		if(e->Attribute("dimX")) {
			e->Attribute("dimX", &_dimensions.x);
			_dimensions.x *= LARGEUR_CASE;
		}
		if(e->Attribute("dimY")) {
			e->Attribute("dimY", &_dimensions.y);
			_dimensions.y *= LARGEUR_CASE;
		}
		
		if(e->Attribute("centrage")) {
			int tmp;
			e->Attribute("centrage", &tmp);
			if(tmp)
				_origine -= Coordonnees(LARGEUR_CASE, 0) / 2;
		}
		if(e->Attribute("nbPoses")) {
			int tmp;
			e->Attribute("nbPoses", &tmp);
			_cadre.definirDimensions(Coordonnees(_cadre.largeur / tmp, _cadre.hauteur));
		}
	}
}

Editeur::ElementEditeur::ElementEditeur(index_t loiProba) : _proba(true), _indexProba(loiProba), _image(Session::cheminRessources() + "alea.png"), _cadre(), _teinte() {
	_cadre = Rectangle(Coordonnees(), _image.dimensions());
	_dimensions = Coordonnees(1, 1) * LARGEUR_CASE;
	_origine.x = 0;
	_origine.y = _image.dimensions().y / 2;

	std::tr1::hash<double> h;
	_teinte = Couleur(h(_indexProba + 20) % 255, h(_indexProba * 100 + 20) % 255, h(_indexProba * 100000 + 20) % 255, 160);
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

Coordonnees Editeur::ElementEditeur::dimensions() const {
	return _dimensions;
}

Coordonnees Editeur::ElementEditeur::origine() const {
	return _origine;
}

Couleur Editeur::ElementEditeur::teinte() const {
	return _teinte;
}

Editeur::LoiProba::LoiProba(std::string const &nom, std::string const &proba) : _nom(nom), _proba() {
	for(ElementNiveau::elementNiveau_t e = ElementNiveau::premierTypeElement; e != ElementNiveau::nbTypesElement; ++e) {
		int val = caractereVersBase64(proba[e * CHIFFRES_VALEURS_PROBA_ENTITES]) * BASE_VALEURS_PROBA_ENTITES + caractereVersBase64(proba[e * CHIFFRES_VALEURS_PROBA_ENTITES + 1]);
		_proba[e] = val;
	}
}
