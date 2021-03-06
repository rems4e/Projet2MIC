//
//  Editeur.cpp
//  Projet2MIC
//
//  Créé par Marc Promé et Rémi Saurel.
//  Ce fichier et son contenu sont librement distribuables, modifiables et utilisables pour toute œuvre non commerciale, à condition d'en citer les auteurs.
//

#include "Editeur.h"
#include "Session.h"
#include "Affichage.h"
#include "ElementNiveau.h"
#include "tinyxml.h"
#include "UtilitaireNiveau.h"
#include <cstring>
#include <cmath>
#include "Texte.h"
#include "Menu.h"
#include <functional>
#include <algorithm>
#include <numeric>
#include "NavigateurFichiers.h"
#include "EntiteMobile.h"
#include "ObjetInterface.h"

#define TAILLE_MAX_NIVEAU 1000

static void afficheRien(Rectangle const &dejaAffiche, index_t selection, Image const &apercu);

template<class Afficheur>
static index_t choisirElement(std::vector<Unichar> const &elements, index_t selection, Unichar const &titre, Image const &apercu, Afficheur afficheurSup);

Editeur *Editeur::_editeur = 0;
Rectangle Editeur::_cadreControles;
Rectangle Editeur::_cadreEditeur;
Rectangle Editeur::_cadreInventaire;
Rectangle Editeur::_cadreCarte;

class VueEditeur : public VueInterface {
public:
	VueEditeur(Editeur &e) : VueInterface(Rectangle(0, 0, Ecran::largeur(), Ecran::hauteur())), _e(e) {
		
	}
	
protected:
	void preparationDessin() override;
	void dessiner() override;
	void gestionClavier() override;
	
private:
	Editeur &_e;
};

class EditeurLoiProba : public VueInterface {
public:
	EditeurLoiProba(Editeur &e, index_t loi, Image &fond) : VueInterface(Rectangle(0, 0, Ecran::largeur(), Ecran::hauteur())), _e(e), _loi(loi), _fond(fond), _nouvelleLoi(_e._niveau->_probas[loi]) {
		_titre = Texte(TRAD("ed Édition de la loi %1 (%2) :", _loi + 1, _e._niveau->_probas[_loi]._nom), POLICE_NORMALE, 20);
		_sup = Texte(TRAD("ed Supprimer la loi"), POLICE_NORMALE, 16);
		_cadreSup = Rectangle(glm::vec2(0), _sup.dimensions());
		_categories.reserve(ElementNiveau::nbTypesElement);
		for(ElementNiveau::elementNiveau_t i = ElementNiveau::premierTypeElement; i != ElementNiveau::nbTypesElement; ++i) {
			Unichar cat = ElementNiveau::nomCategorie(i);
			if(cat == Unichar::uninull)
				break;
			_categories.push_back(Texte(cat + std::string(" : "), POLICE_NORMALE, 16));
		}
		_probas.reserve(_categories.size());
		for(ElementNiveau::elementNiveau_t i = ElementNiveau::premierTypeElement; i != _categories.size(); ++i) {
			int proba = _e._niveau->_probas[loi][i];
			_probas.push_back(Texte(nombreVersTexte(proba), POLICE_NORMALE, 16));
		}
		_cadres.reserve(_categories.size());

		_nbAffiches = std::min(size_t(10), _categories.size());
	}
	
	bool &supprimer() {
		return _supprimer;
	}
	bool &modif() {
		return _modif;
	}
	Editeur::LoiProba &nouvelleLoi() {
		return _nouvelleLoi;
	}
	
protected:
	void preparationDessin() override;
	void dessiner() override;
	void gestionClavier() override;
	
private:
	Editeur &_e;
	Image &_fond;
	
	index_t _loi;
	bool _continuer = true, _supprimer = false;
	
	horloge_t _ancienDefilement = 0;
	
	Texte _titre;
	Texte _sup;
	Rectangle _cadreSup;
	
	std::vector<Texte> _categories;
	
	std::vector<Texte> _probas;
	
	std::vector<Rectangle> _cadres;
	
	ElementNiveau::elementNiveau_t _selection = ElementNiveau::premierTypeElement;
	index_t _premierAffiche = 0;
	size_t _nbAffiches;
	
	float _teinteSelection = 0;
	int _sensTeinte = 1;
	
	Editeur::LoiProba _nouvelleLoi;
	bool _modif = false;
};

class EditeurDimensions : public VueInterface {
public:
	EditeurDimensions(Editeur &e, Image const &ap) : VueInterface(Ecran::ecran()), _e(e), _titre(TRAD("ed Redimensionnement du niveau"), POLICE_NORMALE, 20), _ok(TRAD("ed redimNiveau OK"), POLICE_NORMALE, 16), _dimX(e._niveau->_dimX), _dimY(e._niveau->_dimY), _ap(ap) {
		_enTetes.push_back(Texte(TRAD("ed Largeur (X) : "), POLICE_NORMALE, 16));
		_enTetes.push_back(Texte(TRAD("ed Hauteur (Y) : "), POLICE_NORMALE, 16));
		_valeurs.push_back(Texte(nombreVersTexte(_e._niveau->_dimX), POLICE_NORMALE, 16));
		_valeurs.push_back(Texte(nombreVersTexte(_e._niveau->_dimY), POLICE_NORMALE, 16));
		
		_cadreOk = Rectangle(glm::vec2(0), _ok.dimensions());
	}
	
	size_t dimX() const {
		return _dimX;
	}
	
	size_t dimY() const {
		return _dimY;
	}
	
protected:
	void preparationDessin() override;
	void dessiner() override;
	void gestionClavier() override;
	
private:
	Editeur &_e;

	bool _continuer = true;
	
	Texte _titre;
	Texte _ok;
	std::vector<Texte> _enTetes;
	std::vector<Texte> _valeurs;
	
	std::vector<Rectangle> _cadres;
	
	index_t _selection = 0;
	
	Image const &_ap;
	
	float _teinteSelection = 0;
	int _sensTeinte = 1;
	
	size_t _dimX, _dimY;
	Rectangle _cadreOk;
};

template<typename Afficheur>
class SelecteurElement : public VueInterface {
public:
	SelecteurElement(Afficheur aff, index_t sel, Unichar const &titre, std::vector<Unichar> el, Image const &apercu) : VueInterface(Ecran::ecran()), _afficheur(aff), _selection(sel), _titre(titre, POLICE_NORMALE, 20), _valider(TRAD("ed Valider"),  POLICE_GRANDE, 20 * Ecran::echelleMin(), Couleur::noir), _annuler(TRAD("ed Annuler"),  POLICE_GRANDE, 20 * Ecran::echelleMin(), Couleur::noir), _apercu(apercu) {
		_elements.reserve(el.size());
		for(index_t i = 0; i != el.size(); ++i) {
			_elements.push_back(Texte(el[i], POLICE_NORMALE, 16));
		}
		_cadres.reserve(_elements.size());
		_nbAffiches = std::min(size_t(15), _elements.size());
	}
	
	index_t selection() const {
		return _selection;
	}
	
protected:
	void dessiner() override;
	void gestionClavier() override;

private:
	Afficheur _afficheur;
	Image const &_apercu;

	bool _continuer = true;
	
	horloge_t _ancienDefilement = 0;
	
	index_t _selection;
	
	Texte _titre;
	std::vector<Texte> _elements;
	std::vector<Rectangle> _cadres;
	
	index_t _premierAffiche = 0;
	size_t _nbAffiches;
	
	float _teinteSelection = 0;
	int _sensTeinte = 1;
	
	Texte _valider;
	Texte _annuler;
	
	Rectangle _cadreAnnuler, _cadreValider;
};

Editeur *Editeur::editeur() {
	if(_editeur == 0) {
		_editeur = new Editeur;
	}
	
	return _editeur;
}

Editeur::Editeur() : _sauve(Session::cheminRessources() + "save.png"), _select(Session::cheminRessources() + "select.png"), _coller(Session::cheminRessources() + "coller.png"), _copier(Session::cheminRessources() + "copier.png"), _recharger(Session::cheminRessources() + "recharger.png"), _annuler(Session::cheminRessources() + "annuler.png"), _retablir(Session::cheminRessources() + "retablir.png"), _reglages(Session::cheminRessources() + "reglages.png") {
	
}

Editeur::~Editeur() {
	Editeur::_editeur = 0;
}

void Editeur::ouvrirEditeur(Image &fond, Shader &s) {	
	index_t selection;
	
	do {
		std::vector<Unichar> el;
		el.push_back(TRAD("ed Ouvrir un niveau"));
		el.push_back(TRAD("ed Créer un niveau"));
		el.push_back(TRAD("ed Quitter"));
		Menu m(TRAD("ed Éditeur de niveaux"), el, "");

		selection = m.afficher(0, fond, s);
		// Ouvrir niveau
		if(selection == 0) {
			std::vector<Unichar> el;
			std::vector<std::string> f = NavigateurFichiers::listeFichiers(Session::cheminRessources(), std::vector<std::string>(1, "xml"));
			
			for(int i = 1; ; ++i) {
				std::vector<std::string>::iterator pos = std::find(f.begin(), f.end(), "niveau" + nombreVersTexte(i) + ".xml");
				if(pos == f.end())
					break;
				else
					el.push_back(TRAD("ed Niveau %1", i));
			}
			
			if(el.size()) {
				Menu choix(TRAD("ed Ouvrir un niveau :"), el);
				index_t selection = choix.afficher(0, fond, s);
				if(selection != el.size()) {
					this->editerNiveau("niveau" + nombreVersTexte(selection + 1) + ".xml");
				}
			}
		}
		// Créer niveau
		else if(selection == 1) {
			std::vector<std::string> f = NavigateurFichiers::listeFichiers(Session::cheminRessources(), std::vector<std::string>(1, "xml"));
			
			int nb = 1;
			for(; ; ++nb) {
				std::vector<std::string>::iterator pos = std::find(f.begin(), f.end(), "niveau" + nombreVersTexte(nb) + ".xml");
				if(pos == f.end())
					break;
			}
			
			char const *documentBase = 
			"<?xml version=\"1.0\" standalone=\"no\" ?>"
			"<Niveau dimX=\"16\" dimY=\"16\" musique=\"musique1.mp3\">"
			"</Niveau>";
			TiXmlDocument *document = new TiXmlDocument(Session::cheminRessources() + "niveau" + nombreVersTexte(nb) + ".xml");
			document->Parse(documentBase);
			document->SaveFile();
			delete document;
			
			this->editerNiveau("niveau" + nombreVersTexte(nb) + ".xml");
		}
	} while(selection != 2);
}

void Editeur::editerNiveau(std::string const &fichier) {
	_niveau = new NiveauEditeur(fichier);
	_continuer = true;
	_origine = -glm::vec2(0, Ecran::hauteur() / 2);
	_coucheEdition = static_cast<Niveau::couche_t>(Niveau::nbCouches - 1);
	_ancienRectangle = 0;
	_affichageSelection = Rectangle::aucun;
	_selection.clear();
	_modifie = false;
	_outil = o_selection;
	
	_aIndex = -1;
	
	VueEditeur vueEditeur(*this);
	
	Session::ajouterVueFenetre(&vueEditeur);
	
	for(std::vector<std::vector<ElementEditeur *> >::iterator i = _pressePapier.begin(); i != _pressePapier.end(); ++i) {
		for(std::vector<ElementEditeur *>::iterator j = i->begin(); j != i->end(); ++j) {
			delete *j;
		}
		i->clear();
	}
	_pressePapier.clear();
	
	this->reinitialiserActions();
	
	delete _niveau;
}

void Editeur::demandeEnregistrement(Image const &fond) {
	std::vector<Unichar> txt;
	txt.push_back(TRAD("ed Enregistrer"));
	txt.push_back(TRAD("ed Ne pas enregistrer"));
	
	Menu m(TRAD("ed Niveau modifié. Enregistrer ?"), txt);
	index_t retour = m.afficher(0, fond);
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

void VueEditeur::preparationDessin() {
	Editeur::initCadres();
	this->definirCadre(Ecran::ecran());
}

void VueEditeur::dessiner() {
	Ecran::definirPointeurAffiche(true);
	Affichage::afficherRectangle(Ecran::ecran(), Couleur::grisClair);
	for(Niveau::couche_t c = Niveau::premiereCouche; c != Niveau::nbCouches; ++c) {
		if(c == _e._coucheEdition)
			continue;
		_e.afficherCouche(c);
	}
	
	Affichage::afficherRectangle(Editeur::cadreEditeur(), Couleur(0, 0, 0, 128));
	
	_e.afficherGrille(255);
	_e.afficherCouche(_e._coucheEdition);
	_e.afficherGrille(128);
	
	
	_e.afficherInterface();
}

void VueEditeur::gestionClavier() {
	if(Session::evenement(Session::T_ESC)) {
		Image *fond = Ecran::apercu();
		
		std::vector<Unichar> elemMenus;
		elemMenus.push_back(TRAD("ed Enregistrer le niveau"));
		elemMenus.push_back(TRAD("ed Revenir à la version enregistrée"));
		elemMenus.push_back(TRAD("ed Réglages"));
		elemMenus.push_back(TRAD("ed Quitter l'éditeur"));
		
		Menu menuEditeur(TRAD("ed Menu éditeur"), elemMenus);
		
		index_t retour = menuEditeur.afficher(0, *fond);
		if(retour < elemMenus.size()) {
			if(retour == 3) {
				if(_e._modifie)
					_e.demandeEnregistrement(*fond);
				_e._continuer = false;
			}
			else if(retour == 0) {
				_e.enregistrer();
			}
			else if(retour == 1) {
				_e.recharger();
			}
			else if(retour == 2) {
				Parametres::editerParametres(*fond);
			}
		}
	}
	if(!Session::evenement(Session::B_GAUCHE)) {
		if(_e._ancienRectangle == &Editeur::cadreEditeur()) {
			for(Editeur::selection_t::iterator i = _e._selection.begin(); i != _e._selection.end();) {
				if(i->_etat == Editeur::es_sup)
					i = _e._selection.erase(i);
				else {
					i->_etat = Editeur::es_ok;
					++i;
				}
			}
		}
		_e._ancienRectangle = 0;
		_e._affichageSelection = Rectangle::aucun;
	}
	else {
		if(Editeur::cadreEditeur().contientPoint(Session::souris()))
			_e._ancienRectangle = &Editeur::cadreEditeur();
		else if(Editeur::cadreControles().contientPoint(Session::souris()))
			_e._ancienRectangle = &Editeur::cadreControles();
		else if(Editeur::cadreInventaire().contientPoint(Session::souris()))
			_e._ancienRectangle = &Editeur::cadreInventaire();
		else if(Editeur::cadreCarte().contientPoint(Session::souris()))
			_e._ancienRectangle = &Editeur::cadreCarte();
	}
	
	if(_e._ancienRectangle == &Editeur::cadreInventaire())
		_e.sourisInventaire();
	else if(_e._ancienRectangle == &Editeur::cadreControles())
		_e.sourisControles();
	else if(_e._ancienRectangle == &Editeur::cadreEditeur())
		_e.sourisEditeur();
	else if(_e._ancienRectangle == &Editeur::cadreCarte())
		_e.sourisCarte();
	
	if(Session::evenement(Session::QUITTER)) {
		_e._continuer = false;
		if(_e._modifie) {
			Image *f = Ecran::apercu();
			_e.demandeEnregistrement(*f);
			delete f;
		}
	}
	else {
		if(Session::evenement(Session::T_GAUCHE))
			_e._origine.x -= 10;
		else if(Session::evenement(Session::T_DROITE))
			_e._origine.x += 10;
		if(Session::evenement(Session::T_HAUT))
			_e._origine.y -= 10;
		else if(Session::evenement(Session::T_BAS))
			_e._origine.y += 10;
		
		if(Session::evenement(Session::T_EFFACER)) {
			std::list<Editeur::ActionEditeur *> l;
			for(Editeur::selection_t::iterator i = _e._selection.begin(); i != _e._selection.end(); ++i) {
				Editeur::ActionEditeur *a = new Editeur::RemplacerEntite(0, i->_e, i->_posX, i->_posY, _e._coucheEdition);
				l.push_back(a);
			}
			Editeur::ActionEditeur *a = new Editeur::ActionsEditeur(l);
			_e.posterAction(a);
			
			_e._selection.clear();
			_e._cadreSelection = Rectangle::aucun;
		}
	}
	
	if(!_e._continuer) {
		Session::supprimerVueFenetre();
	}
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

	glm::vec2 pos;
	for(Ligne::iterator i = _niveau->_elements.begin(); i != _niveau->_elements.end(); ++i) {
		pos.x = (_niveau->_dimX - 1) * LARGEUR_CASE;
		for(Colonne::reverse_iterator j = i->rbegin(); j != i->rend(); ++j) {
			ElementEditeur const *elem = (*j)[couche];
			if(elem) {
				glm::vec2 posAffichage = referentielNiveauVersEcran(pos) - elem->origine() - _origine + Editeur::cadreEditeur().origine();
				Affichage::definirTeinte(elem->teinte());
				elem->image().afficher(posAffichage, elem->cadre());
			}
			pos.x -= LARGEUR_CASE;
		}
		pos.y += LARGEUR_CASE;
	}
	Affichage::definirTeinte(Couleur::blanc);
	
	if(couche == _coucheEdition) {
		for(selection_t::iterator i = _selection.begin(); i != _selection.end(); ++i) {
			if(i->_etat == es_sup)
				continue;
			dimension_t dimX = LARGEUR_CASE, dimY = LARGEUR_CASE;
			if(i->_e) {
				dimX = i->_e->dimensions().x;
				dimY = i->_e->dimensions().y;
			}
			glm::vec2 pos(i->_posX * LARGEUR_CASE, i->_posY * LARGEUR_CASE);
			glm::vec2 p1 = referentielNiveauVersEcran(pos) - _origine + Editeur::cadreEditeur().origine();
			glm::vec2 p2 = referentielNiveauVersEcran(pos + glm::vec2(dimX, 0)) - _origine + Editeur::cadreEditeur().origine();
			glm::vec2 p3 = referentielNiveauVersEcran(pos + glm::vec2(dimX, dimY)) - _origine + Editeur::cadreEditeur().origine();
			glm::vec2 p4 = referentielNiveauVersEcran(pos + glm::vec2(0, dimY)) - _origine + Editeur::cadreEditeur().origine();
			Affichage::afficherQuadrilatere(p1, p2, p3, p4, Couleur(teinteSelection * 255, teinteSelection * 255, teinteSelection * 255, 128));
		}
		
		if(_outil == o_coller) {
			glm::vec2 point(Session::souris() - Editeur::cadreEditeur().origine() + _origine);
			glm::vec2 pointNiveau(referentielEcranVersNiveau(point));
			index_t pX = std::floor(pointNiveau.x / LARGEUR_CASE), pY = std::floor(pointNiveau.y / LARGEUR_CASE);
			
			Affichage::ajouterOpacite(128);
			for(std::vector<std::vector<ElementEditeur *> >::iterator i = _pressePapier.begin(); i != _pressePapier.end(); ++i) {
				for(std::vector<ElementEditeur *>::iterator j = i->begin(); j != i->end(); ++j) {
					if(*j) {
						glm::vec2 p = referentielNiveauVersEcran(glm::vec2(pX + std::distance(i->begin(), j), pY + std::distance(_pressePapier.begin(), i)) * static_cast<coordonnee_t>(LARGEUR_CASE)) - (*j)->origine() - _origine + Editeur::cadreEditeur().origine();
						Affichage::definirTeinte((*j)->teinte());
						(*j)->image().afficher(p);
					}
				}
			}
			Affichage::supprimerOpacite();
			Affichage::definirTeinte(Couleur::blanc);
		}
	}
}

void Editeur::afficherGrille(Couleur::composante_t opacite) {
	Couleur c(Couleur::rouge, opacite);
	for(index_t y = 0; y <= _niveau->_dimY; ++y) {
		Affichage::afficherLigne(referentielNiveauVersEcran(glm::vec2(0, y) * static_cast<coordonnee_t>(LARGEUR_CASE)) - _origine + Editeur::cadreEditeur().origine(), referentielNiveauVersEcran(glm::vec2(_niveau->_dimX, y) * static_cast<coordonnee_t>(LARGEUR_CASE)) - _origine + Editeur::cadreEditeur().origine(), c, 1.0);
	}
	for(index_t x = 0; x <= _niveau->_dimX; ++x) {
		Affichage::afficherLigne(referentielNiveauVersEcran(glm::vec2(x, 0) * static_cast<coordonnee_t>(LARGEUR_CASE)) - _origine + Editeur::cadreEditeur().origine(), referentielNiveauVersEcran(glm::vec2(x, _niveau->_dimY) * static_cast<coordonnee_t>(LARGEUR_CASE)) - _origine + Editeur::cadreEditeur().origine(), c, 1.0);
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
		
		glm::vec2 dec = cadreEditeur().origine() - _origine;

		glm::vec2 p1 = dec + referentielNiveauVersEcran(glm::vec2(r.gauche, r.haut));
		glm::vec2 p2 = dec + referentielNiveauVersEcran(glm::vec2(r.gauche + r.largeur, r.haut));
		glm::vec2 p3 = dec + referentielNiveauVersEcran(glm::vec2(r.gauche + r.largeur, r.haut + r.hauteur));
		glm::vec2 p4 = dec + referentielNiveauVersEcran(glm::vec2(r.gauche, r.haut + r.hauteur));
		
		Affichage::afficherQuadrilatere(p1, p2, p3, p4, Couleur(200, 205, 220, 128));
	}
	
	this->afficherControles();
	this->afficherInventaire();
	this->afficherCarte();
}

void Editeur::afficherInventaire() {
	_fonctionsInventaire.clear();

	Rectangle const &cadre = Editeur::cadreInventaire();
	Affichage::afficherRectangle(cadre, Couleur::noir);
	Affichage::afficherRectangle(Rectangle(cadre.gauche + 1, cadre.haut + 1, cadre.largeur - 2, cadre.hauteur - 2), Couleur(220, 225, 240));
	
	Rectangle rectCat(Editeur::cadreInventaire().origine() + glm::vec2(10, 10), glm::vec2(0));
	
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
		cc.definir(TRAD("ed Entités sélectionnées :"));
	}
	else if(tailleSelection == 1) {
		cc.definir(TRAD("ed Entité sélectionnée :"));
	}
	else {
		cc.definir(TRAD("ed Aucune sélection"));
	}
	rectCat.definirDimensions(cc.dimensions());
	cc.afficher(rectCat.origine());

	
	if(tailleSelection) {		
		selection_t::iterator i = _selection.begin();
		for(; i != _selection.end(); ++i) {
			if(i->_etat != es_sup)
				break;
		}
		bool diffProba = !i->_e, diffIndex = false, diffCategorie = false, diffIndexProba = false, vide = !i->_e;
		bool proba = false;
		index_t index = 0, indexProba, nbVide = 0;
		ElementNiveau::elementNiveau_t cat = ElementNiveau::nbTypesElement;
		uint16_t idEntite;
		if(i->_e) {
			idEntite = i->_e->operator()();
			obtenirInfosEntites(idEntite, proba, indexProba, cat, index);
			++i;		
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
		}
		else {
			for(selection_t::iterator i = _selection.begin(); i != _selection.end(); ++i) {
				if(i->_etat == es_sup)
					continue;
				if(i->_e) {
					vide = false;
					if(nbVide > 1)
						break;
				}
				else
					++nbVide;
			}
		}
		
		if(vide || (!diffProba && !diffIndexProba && !diffIndex && !diffCategorie)) {
			rectCat.haut += rectCat.dimensions().y + 10;
			cc.definir(TRAD("ed Sélectionner les entités identiques"));
			rectCat.definirDimensions(cc.dimensions());
			cc.afficher(rectCat.origine());
			
			_fonctionsInventaire.push_back(std::make_pair(rectCat, &Editeur::selectionnerSemblables));
		}
		
		rectCat.haut += rectCat.dimensions().y + 10;
		cc.definir(TRAD("ed Remplissage grille…"));
		rectCat.definirDimensions(cc.dimensions());
		cc.afficher(rectCat.origine());
		
		_fonctionsInventaire.push_back(std::make_pair(rectCat, &Editeur::remplissageGrille));

		if(vide) {
			rectCat.haut += rectCat.dimensions().y + 10;
			if(nbVide > 1)
				cc.definir(TRAD("ed Cases vides"));
			else
				cc.definir(TRAD("ed Case vide"));
			rectCat.definirDimensions(cc.dimensions());
			cc.afficher(rectCat.origine());
			
			_fonctionsInventaire.push_back(std::make_pair(rectCat, &Editeur::modifProba));
		}
		else if(diffProba) {
			rectCat.haut += rectCat.dimensions().y + 10;
			cc.definir(TRAD("ed Sélection hétérogène"));
			rectCat.definirDimensions(cc.dimensions());
			cc.afficher(rectCat.origine());
			
			_fonctionsInventaire.push_back(std::make_pair(rectCat, &Editeur::modifProba));
		}
		else {
			if(proba) {
				rectCat.haut += rectCat.dimensions().y + 10;
				cc.definir(TRAD("ed Défini par : Loi de probabilité"));
				rectCat.definirDimensions(cc.dimensions());
				cc.afficher(rectCat.origine());
				_fonctionsInventaire.push_back(std::make_pair(rectCat, &Editeur::modifProba));
				
				if(diffIndexProba) {
					rectCat.haut += rectCat.dimensions().y + 10;
					cc.definir(TRAD("ed Lois différentes"));
					rectCat.definirDimensions(cc.dimensions());
					cc.afficher(rectCat.origine());
				}
				else {
					rectCat.haut += rectCat.dimensions().y + 10;
					cc.definir(TRAD("ed Loi n°%1 (%2)", indexProba + 1, _niveau->_probas[indexProba]._nom));
					rectCat.definirDimensions(cc.dimensions());
					cc.afficher(rectCat.origine());
				}
				_fonctionsInventaire.push_back(std::make_pair(rectCat, &Editeur::modifIndexProba));
			}
			else {
				rectCat.haut += rectCat.dimensions().y + 10;
				cc.definir(TRAD("ed Défini par : Catégorie + index"));
				rectCat.definirDimensions(cc.dimensions());
				cc.afficher(rectCat.origine());
				_fonctionsInventaire.push_back(std::make_pair(rectCat, &Editeur::modifProba));

				if(diffCategorie) {
					rectCat.haut += rectCat.dimensions().y + 10;
					cc.definir(TRAD("ed Catégories différentes "));
					rectCat.definirDimensions(cc.dimensions());
					cc.afficher(rectCat.origine());
				}
				else {
					rectCat.haut += rectCat.dimensions().y + 10;
					cc.definir(TRAD("ed Catégorie : %1", ElementNiveau::nomCategorie(cat)));
					rectCat.definirDimensions(cc.dimensions());
					cc.afficher(rectCat.origine());
				}
				_fonctionsInventaire.push_back(std::make_pair(rectCat, &Editeur::modifCategorie));
				
				if(diffIndex) {
					rectCat.haut += rectCat.dimensions().y + 10;
					cc.definir(TRAD("ed Index différents"));
					rectCat.definirDimensions(cc.dimensions());
					cc.afficher(rectCat.origine());
				}
				else {
					rectCat.haut += rectCat.dimensions().y + 10;
					cc.definir(TRAD("ed Index : %1", index));
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
	Affichage::afficherRectangle(cadre, Couleur::noir);
	Affichage::afficherRectangle(Rectangle(cadre.gauche + 1, cadre.haut + 1, cadre.largeur - 2, cadre.hauteur - 2), Couleur(220, 225, 240));
	
	Rectangle rect(Editeur::cadreControles().origine() + glm::vec2(10, 10), glm::vec2(0));
	
	_sauve.afficher(rect.origine());
	rect.definirDimensions(_sauve.dimensions());
	_fonctionsControles.push_back(std::make_pair(rect, &Editeur::enregistrer));
	
	rect.gauche += _sauve.dimensions().x;
	_recharger.afficher(rect.origine());
	rect.definirDimensions(_recharger.dimensions());
	_fonctionsControles.push_back(std::make_pair(rect, &Editeur::outilRecharger));

	rect.gauche += _select.dimensions().x;
	_annuler.afficher(rect.origine());
	rect.definirDimensions(_annuler.dimensions());
	_fonctionsControles.push_back(std::make_pair(rect, &Editeur::outilAnnuler));

	rect.gauche += _annuler.dimensions().x;
	_retablir.afficher(rect.origine());
	rect.definirDimensions(_retablir.dimensions());
	_fonctionsControles.push_back(std::make_pair(rect, &Editeur::outilRetablir));
	
	rect.gauche += _retablir.dimensions().x;
	_reglages.afficher(rect.origine());
	rect.definirDimensions(_reglages.dimensions());
	_fonctionsControles.push_back(std::make_pair(rect, &Editeur::outilReglages));

	rect.gauche = Editeur::cadreControles().origine().x + 10;
	rect.haut += rect.hauteur + 10;

	_copier.afficher(rect.origine());
	rect.definirDimensions(_copier.dimensions());
	_fonctionsControles.push_back(std::make_pair(rect, &Editeur::outilCopier));

	rect.gauche += _copier.dimensions().x;
	_select.afficher(rect.origine());
	rect.definirDimensions(_select.dimensions());
	_fonctionsControles.push_back(std::make_pair(rect, &Editeur::outilSelection));
	
	rect.gauche += _select.dimensions().x;
	_coller.afficher(rect.origine());
	rect.definirDimensions(_coller.dimensions());
	_fonctionsControles.push_back(std::make_pair(rect, &Editeur::outilColler));

	rect.gauche = Editeur::cadreControles().origine().x + 10;
	rect.haut += rect.hauteur + 10;
	
	Texte cc(TRAD("ed Afficher la couche : %1", Niveau::nomCouche(_coucheEdition)));
	cc.definir(Couleur::noir);
	rect.definirDimensions(cc.dimensions());
	cc.afficher(rect.origine());
	_fonctionsControles.push_back(std::make_pair(rect, &Editeur::modifCouche));
	
	rect.haut += rect.hauteur + 10;
	cc.definir(TRAD("ed Éditer les lois de proba"));
	rect.definirDimensions(cc.dimensions());
	cc.afficher(rect.origine());
	_fonctionsControles.push_back(std::make_pair(rect, &Editeur::modifLoisProbas));

	rect.haut += rect.hauteur + 10;
	cc.definir(TRAD("ed Éditer les dimensions du niveau"));
	rect.definirDimensions(cc.dimensions());
	cc.afficher(rect.origine());
	_fonctionsControles.push_back(std::make_pair(rect, &Editeur::modifDimensions));
}

void Editeur::afficherCarte() {
	Rectangle cadre = Editeur::cadreCarte();
	Affichage::afficherRectangle(cadre, Couleur::noir);
	Affichage::afficherRectangle(Rectangle(cadre.gauche + 1, cadre.haut + 1, cadre.largeur - 2, cadre.hauteur - 2), Couleur(220, 225, 240));

	dimension_t max = std::max(_niveau->_dimX, _niveau->_dimY);
	
	dimension_t dimX = cadre.largeur * _niveau->_dimX / max;
	dimension_t dimY = cadre.largeur * _niveau->_dimY / max;
	glm::vec2 p1;
	glm::vec2 p2(dimX, 0);
	glm::vec2 p3(dimX, dimY);
	glm::vec2 p4(0, dimY);
	p1 = referentielNiveauVersEcran(p1) + glm::vec2(cadre.gauche, cadre.haut + dimX / 4);
	p2 = referentielNiveauVersEcran(p2) + glm::vec2(cadre.gauche, cadre.haut + dimX / 4);
	p3 = referentielNiveauVersEcran(p3) + glm::vec2(cadre.gauche, cadre.haut + dimX / 4);
	p4 = referentielNiveauVersEcran(p4) + glm::vec2(cadre.gauche, cadre.haut + dimX / 4);
	
	glm::vec2 dimensions(p3.x - p1.x, p4.y - p2.y);
	glm::vec2 decalage = (cadre.dimensions() - dimensions) / 2.0f;
	p1 += decalage;
	p2 += decalage;
	p3 += decalage;
	p4 += decalage;
	
	Affichage::afficherQuadrilatere(p1, p2, p3, p4, Couleur::gris);
	
	Rectangle aff;
	aff.definirOrigine(decalage + glm::vec2(cadre.gauche, cadre.haut + dimX / 4) + glm::vec2(_origine.x / (max * LARGEUR_CASE), _origine.y / (max * LARGEUR_CASE)) * cadre.largeur);
	aff.definirDimensions(glm::vec2(Editeur::cadreEditeur().largeur / (max * LARGEUR_CASE) * cadre.largeur, Editeur::cadreEditeur().hauteur / (max * LARGEUR_CASE) * cadre.largeur));
	aff = cadre.intersection(aff);
	
	Affichage::afficherRectangle(aff, Couleur(255, 0, 0, 128));
}

void Editeur::sourisEditeur() {
	glm::vec2 point(Session::souris() - Editeur::cadreEditeur().origine() + _origine);
	glm::vec2 pointNiveau(referentielEcranVersNiveau(point));
	
	if(_outil == o_selection) {
		if(_affichageSelection == Rectangle::aucun) {
			_affichageSelection = Rectangle(pointNiveau, glm::vec2(0));
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

		glm::vec2 pos(0);
		for(Ligne::iterator i = _niveau->_elements.begin(); i != _niveau->_elements.end(); ++i) {
			pos.x = 0;
			for(Colonne::iterator j = i->begin(); j != i->end(); ++j) {
				ElementEditeur const *elem = (*j)[_coucheEdition];
				glm::vec2 dim(elem ? elem->dimensions().x : LARGEUR_CASE, elem ? elem->dimensions().y : LARGEUR_CASE);
				if(Rectangle(pos, (glm::vec2(dim.x, dim.y))).superposition(_cadreSelection)) {
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
	else if(_outil == o_coller && _pressePapier.size()) {
		index_t pX = std::floor(pointNiveau.x / LARGEUR_CASE), pY = std::floor(pointNiveau.y / LARGEUR_CASE);
		this->coller(pX, pY);
		Session::reinitialiser(Session::B_GAUCHE);
	}
}

void Editeur::sourisControles() {
	for(listeFonctions_t::iterator i = _fonctionsControles.begin(); i != _fonctionsControles.end(); ++i) {
		if(i->first.contientPoint(Session::souris())) {
			(this->*(i->second))();
			Session::reinitialiser(Session::B_GAUCHE);
			break;
		}
	}
}

void Editeur::sourisInventaire() {
	for(listeFonctions_t::iterator i = _fonctionsInventaire.begin(); i != _fonctionsInventaire.end(); ++i) {
		if(i->first.contientPoint(Session::souris())) {
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
	glm::vec2 p1;
	glm::vec2 p2(dimX, 0);
	glm::vec2 p3(dimX, dimY);
	glm::vec2 p4(0, dimY);
	p1 = referentielNiveauVersEcran(p1) + glm::vec2(cadre.gauche, cadre.haut + dimX / 4);
	p2 = referentielNiveauVersEcran(p2) + glm::vec2(cadre.gauche, cadre.haut + dimX / 4);
	p3 = referentielNiveauVersEcran(p3) + glm::vec2(cadre.gauche, cadre.haut + dimX / 4);
	p4 = referentielNiveauVersEcran(p4) + glm::vec2(cadre.gauche, cadre.haut + dimX / 4);
	
	glm::vec2 dimensions(p3.x - p1.x, p4.y - p2.y);
	glm::vec2 decalage = (cadre.dimensions() - dimensions) / 2.0f;

	glm::vec2 pos = Session::souris() - cadre.origine() - decalage - glm::vec2(0, (p1.y - p2.y));
	_origine = pos / cadre.largeur * max * static_cast<coordonnee_t>(LARGEUR_CASE);
}

void Editeur::outilAnnuler() {
	this->annuler();
}

void Editeur::outilRetablir() {
	this->retablir();
}

void Editeur::outilCopier() {
	this->copier();
}

void Editeur::outilColler() {
	_outil = o_coller;
}

void Editeur::outilSelection() {
	_outil = o_selection;
}

void Editeur::outilRecharger() {
	this->recharger();
}

void Editeur::outilReglages() {
	Image *fond = Ecran::apercu();
	this->reglages(*fond);
	delete fond;
}

void Editeur::presenter(index_t x, index_t y) {
	if(x != -1 && y != -1) {
		//_origine = -Coordonnees(0, Ecran::hauteur() / 2) + referentielNiveauVersEcran(Coordonnees(x, y) * LARGEUR_CASE);
	}
}

void Editeur::posterAction(ActionEditeur *action) {
	if(!action)
		return;
	
	ActionsEditeur *a = dynamic_cast<ActionsEditeur *>(action);
	if(a && !a->taille()) {
		return;
	}
	
	while(_pileRetablissements.size()) {
		delete _pileRetablissements.top();
		_pileRetablissements.pop();
	}
	_pileAnnulations.push(action);
	(*action)();
	
	_modifie = true;
}

void Editeur::reinitialiserActions() {
	while(_pileRetablissements.size()) {
		delete _pileRetablissements.top();
		_pileRetablissements.pop();
	}
	while(_pileAnnulations.size()) {
		delete _pileAnnulations.top();
		_pileAnnulations.pop();
	}
}

void Editeur::annuler() {
	if(_pileAnnulations.empty())
		return;
	
	ActionEditeur *action = _pileAnnulations.top()->oppose();
	delete _pileAnnulations.top();
	_pileAnnulations.pop();
	_pileRetablissements.push(action);
	(*action)();
	this->presenter(action->pX(), action->pY());
		
	if(!_pileAnnulations.size()) {
		_modifie = false;
	}
	
	_selection.clear();
}

void Editeur::retablir() {
	if(!_pileRetablissements.size())
		return;
	
	ActionEditeur *action = _pileRetablissements.top()->oppose();
	delete _pileRetablissements.top();
	_pileRetablissements.pop();
	_pileAnnulations.push(action);
	(*action)();
	this->presenter(action->pX(), action->pY());
		
	_modifie = true;

	_selection.clear();
}

void Editeur::copier() {
	for(std::vector<std::vector<ElementEditeur *> >::iterator i = _pressePapier.begin(); i != _pressePapier.end(); ++i) {
		for(std::vector<ElementEditeur *>::iterator j = i->begin(); j != i->end(); ++j) {
			delete *j;
		}
		i->clear();
	}
	_pressePapier.clear();
	
	ssize_t dimX = 0, dimY = 0;
	index_t pX = std::numeric_limits<index_t>::max(), pY = std::numeric_limits<index_t>::max();
	
	for(selection_t::iterator i = _selection.begin(); i != _selection.end(); ++i) {
		if(i->_e) {
			dimX = std::max<ssize_t>(dimX, i->_posX);
			dimY = std::max<ssize_t>(dimY, i->_posY);
			
			pX = std::min<ssize_t>(pX, i->_posX);
			pY = std::min<ssize_t>(pY, i->_posY);
		}
	}
	
	_pressePapier.resize(dimY - pY + 1);
	for(std::vector<std::vector<ElementEditeur *> >::iterator i = _pressePapier.begin(); i != _pressePapier.end(); ++i) {
		i->resize(dimX - pX + 1);
	}
	
	for(selection_t::iterator i = _selection.begin(); i != _selection.end(); ++i) {
		if(i->_e) {
			_pressePapier[i->_posY - pY][i->_posX - pX] = new ElementEditeur(*(i->_e));
		}
	}
}

void Editeur::coller(index_t pX, index_t pY) {
	if(_pressePapier.empty() || pX < 0 || pY < 0)
		return;
	
	_selection.clear();
	ssize_t dimX = _pressePapier[0].size(), dimY = _pressePapier.size();
	std::list<ActionEditeur *> l;
	for(index_t y = pY; y != std::min<ssize_t>(pY + dimY, _niveau->_dimY); ++y) {
		for(index_t x = pX; x != std::min<ssize_t>(pX + dimX, _niveau->_dimX); ++x) {			
			ActionEditeur *a = new RemplacerEntite(_pressePapier[y - pY][x - pX], _niveau->_elements[y][x]._contenu[_coucheEdition], x, y, _coucheEdition);
			l.push_back(a);
		}
	}
	
	ActionEditeur *a = new ActionsEditeur(l);
	this->posterAction(a);
}

// FIXME: au boulot
void Editeur::reglages(Image &fond) {
	/*bool continuer = true;
	
	Texte titre(TRAD("ed Réglages du niveau"), POLICE_GRANDE, 20);
	
	Rectangle cadreValider, cadreAnnuler;
	Texte valider(TRAD("ed Valider"), POLICE_GRANDE, 20 * Ecran::echelleMin());
	Texte annuler(TRAD("ed Annuler"), POLICE_GRANDE, 20 * Ecran::echelleMin());
	while(Session::boucle(100.0f, continuer)) {		
		Ecran::effacer();
		
		Shader::flou(1).activer();
		fond.afficher(Coordonnees());
		Shader::desactiver();
		
		Affichage::afficherRectangle(Ecran::ecran(), Couleur(255, 255, 255, 200));

		titre.definir(POLICE_GRANDE, 20 * Ecran::echelleMin());
		valider.definir(POLICE_GRANDE, 20 * Ecran::echelleMin());
		annuler.definir(POLICE_GRANDE, 20 * Ecran::echelleMin());
		
		Rectangle pos(Coordonnees(80, 80), titre.dimensions());
		
		titre.afficher(pos.origine());
		
		pos.definirOrigine(pos.origine() + Coordonnees(0, pos.dimensions().y + 10 * Ecran::echelleMin()));
		
		cadreValider.definirDimensions(valider.dimensions()); 
		cadreAnnuler.definirDimensions(annuler.dimensions()); 
		cadreValider.definirOrigine(pos.origine() + Coordonnees(20 * Ecran::echelleMin(), 20 * Ecran::echelleMin())); 
		cadreAnnuler.definirOrigine(cadreValider.origine() + Coordonnees(80 * Ecran::echelleMin() + cadreValider.largeur, 0));
		
		valider.afficher(cadreValider.origine());
		annuler.afficher(cadreAnnuler.origine());
		
		//Ecran::finaliser();
		
		if(Session::evenement(Session::B_GAUCHE)) {
			if(Session::souris() < cadreValider) {
				continuer = false;
			}
			else if(Session::souris() < cadreAnnuler) {
				continuer = false;
			}
		}
		
		//Ecran::maj();
	}*/
}

void Editeur::recharger() {
	std::string f = _niveau->_fichier;
	delete _niveau;
	_niveau = new NiveauEditeur(f);
	_selection.clear();
	_modifie = false;
	this->reinitialiserActions();
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
	
	TiXmlElement *n = document->FirstChildElement("Niveau");// niveau
	
	n->SetAttribute("dimX", _niveau->_dimX);
	n->SetAttribute("dimY", _niveau->_dimY);
	
	if(_niveau->_musique.size())
		n->SetAttribute("musique", _niveau->_musique);
	
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
		TiXmlElement cc(Niveau::nomBaliseCouche(couche));
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
	
	if(!document->SaveFile())
		std::cerr << "La sauvegarde du fichier de niveau " << document->Value() << " a échoué. Vérifiez le chemin du fichier." << std::endl;
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
	
	index_t sel = 0;
	{
		bool proba;
		index_t index;
		ElementNiveau::elementNiveau_t cat;
		obtenirInfosEntites(_selection.front()._e->operator()(), proba, sel, cat, index);
	}
	
	Image *ap = Ecran::apercu();
	index_t elem = choisirElement(cat, sel, TRAD("ed Choisissez une loi de probabilité :"), *ap, &afficheRien);
	delete ap;
	
	_aIndexProba = elem;
	
	if(elem != cat.size()) {		
		bool proba;
		index_t indexProba, index;
		ElementNiveau::elementNiveau_t cat;
		std::list<ActionEditeur *> l;
		for(selection_t::iterator i = _selection.begin(); i != _selection.end(); ++i) {
			if(!i->_e)
				indexProba = elem + 1;
			else
				obtenirInfosEntites(i->_e->operator()(), proba, indexProba, cat, index);
			if(indexProba != elem) {
				ElementEditeur *nouveau = new ElementEditeur(elem);
				ActionEditeur *a = new RemplacerEntite(nouveau, i->_e, i->_posX, i->_posY, _coucheEdition);
				l.push_back(a);
				
				i->_e = nouveau;
			}
		}
		
		ActionEditeur *a = new ActionsEditeur(l);
		this->posterAction(a);
	}
}
void Editeur::modifLoisProbas() {
	std::vector<Unichar> lois;
	for(std::vector<LoiProba>::iterator i = _niveau->_probas.begin(); i != _niveau->_probas.end(); ++i) {
		lois.push_back(TRAD("ed Loi %1 (%2)", std::distance(_niveau->_probas.begin(), i) + 1, i->_nom));
	}
	lois.push_back(TRAD("ed Ajouter une loi"));
	
	Image *fond = Ecran::apercu();
	index_t elem = choisirElement(lois, 0, TRAD("ed Choisissez une loi à modifier :"), *fond, &afficheRien);
	
	if(elem == lois.size() - 1) {
		LoiProba l("proba" + nombreVersTexte(Editeur::editeur()->_niveau->_probas.size()));
		ActionEditeur *a = new AjouterLoiProba(Editeur::editeur()->_niveau->_probas.size(), l);
		this->posterAction(a);
		this->editerLoiProba(_niveau->_probas.size() - 1, *fond);
	}
	else if(elem != lois.size()) {
		this->editerLoiProba(elem, *fond);
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
		return r.contientPoint(Session::souris());
	}
};

void EditeurLoiProba::preparationDessin() {
	
}

void EditeurLoiProba::dessiner() {
	_teinteSelection += 1.0f / 50.0f * (60.0f / Ecran::frequenceInstantanee()) * _sensTeinte;
	if(_teinteSelection > 0.7f) {
		_teinteSelection = 0.7f;
		_sensTeinte = -1;
	}
	else if(_teinteSelection < 0) {
		_teinteSelection = 0;
		_sensTeinte = 1;
	}
	
	
	_categories[_selection].definir(Couleur(_teinteSelection * 255));
	_probas[_selection].definir(Couleur(_teinteSelection * 255));
	
	
	Shader::flou(1).activer();
	_fond.afficher(glm::vec2(0));
	Shader::desactiver();
	
	Affichage::afficherRectangle(Ecran::ecran(), Couleur(255, 255, 255, 200));
	
	Rectangle cadre(glm::vec2(80, 80), _titre.dimensions());
	_titre.afficher(cadre.origine());
	
	cadre.haut += cadre.hauteur + 20;
	cadre.gauche += 20;
	
	dimension_t largeurCat = std::max_element(_categories.begin() + _premierAffiche, _categories.begin() + _premierAffiche + _nbAffiches, largeurTexte_t())->dimensions().x;
	dimension_t largeurProba = std::max_element(_probas.begin() + _premierAffiche, _probas.begin() + _premierAffiche + _nbAffiches, largeurTexte_t())->dimensions().x;
	
	dimension_t hauteur = std::accumulate(_categories.begin() + _premierAffiche, _categories.begin() + _premierAffiche + _nbAffiches, dimension_t(0), hauteurTexte_t(10)) - 10;
	
	Affichage::afficherRectangle(Rectangle(cadre.gauche, cadre.haut, largeurCat + largeurProba + 20, hauteur + 20), Couleur(200, 205, 220, 128));
	
	cadre.decaler(glm::vec2(10, 10));
	coordonnee_t sauveHaut = cadre.haut;
	afficheurTexte_t afficheur(cadre, 10);
	for_each(_categories.begin() + _premierAffiche, _categories.begin() + _premierAffiche + _nbAffiches, afficheur);
	cadre.haut = sauveHaut;
	cadre.gauche += largeurCat;
	for_each(_probas.begin() + _premierAffiche, _probas.begin() + _premierAffiche + _nbAffiches, afficheur);
	_categories[_selection].definir(Couleur::noir);
	_probas[_selection].definir(Couleur::noir);
	
	_cadres.clear();
	cadre.gauche -= largeurCat;
	cadre.largeur = largeurCat + largeurProba;
	cadre.haut = sauveHaut;
	initCadres_t initCadres(_cadres, cadre, 10);
	for_each(_categories.begin() + _premierAffiche, _categories.begin() + _premierAffiche + _nbAffiches, initCadres);
	
	cadre.decaler(-glm::vec2(10, -10));
	
	_sup.afficher(cadre.origine());
	_cadreSup.definirOrigine(cadre.origine());
}

void EditeurLoiProba::gestionClavier() {
	if(Session::evenement(Session::T_ESC) || Session::evenement(Session::QUITTER) || Session::evenement(Session::T_ENTREE)) {
		_continuer = false;
	}
	else {
		if(Session::evenement(Session::T_HAUT) && horloge() - _ancienDefilement > INTERVALLE_DEFILEMENT) {
			if(_selection > 0) {
				_selection = static_cast<ElementNiveau::elementNiveau_t>(_selection - 1);
				if(_selection < _premierAffiche)
					--_premierAffiche;
			}
			else {
				_selection = static_cast<ElementNiveau::elementNiveau_t>(_categories.size() - 1);
				if(_categories.size() > _nbAffiches)
					_premierAffiche = _categories.size() - _nbAffiches;
			}
			
			_ancienDefilement = horloge();
		}
		else if(Session::evenement(Session::T_BAS) && horloge() - _ancienDefilement > INTERVALLE_DEFILEMENT) {
			if(_selection < _categories.size() - 1) {
				++_selection;
				if(_selection >= _premierAffiche + _nbAffiches)
					++_premierAffiche;
			}
			else {
				_selection = ElementNiveau::premierTypeElement;
				_premierAffiche = 0;
			}
			
			_ancienDefilement = horloge();
		}
		if(Session::evenement(Session::B_GAUCHE)) {
			std::vector<Rectangle>::iterator souris = std::find_if(_cadres.begin(), _cadres.end(), trouveSouris_t());
			if(souris != _cadres.end()) {
				_selection = static_cast<ElementNiveau::elementNiveau_t>(std::distance(_cadres.begin(), souris));
			}
			else if(_cadreSup.contientPoint(Session::souris())) {
				_continuer = false;
				_supprimer = true;
			}
		}
		if(Session::evenement(Session::T_EFFACER)) {
			_nouvelleLoi[_selection] /= 10;
			_probas[_selection].definir(nombreVersTexte(_e._niveau->_probas[_loi][_selection]));
			Session::reinitialiser(Session::T_EFFACER);
			_modif = true;
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
				int &p = _nouvelleLoi[_selection];
				int p1 = p;
				p1 = p1 * 10 + nb;
				if(p1 < NB_VALEURS_PROBA_ENTITES) {
					p = p1;
					_probas[_selection].definir(nombreVersTexte(p));
					_modif = true;
				}
			}
		}
	}
	
	if(!_continuer) {
		Session::supprimerVueFenetre();
	}
}

void Editeur::editerLoiProba(index_t loi, Image &fond) {
	EditeurLoiProba e(*this, loi, fond);
	Session::ajouterVueFenetre(&e);
	
	if(e.supprimer()) {
		uint16_t idProba = entite(loi);
		for(Niveau::couche_t c = Niveau::premiereCouche; c != Niveau::nbCouches; ++c) {
			for(Ligne::iterator i = _niveau->_elements.begin(); i != _niveau->_elements.end(); ++i) {
				for(Colonne::iterator j = i->begin(); j != i->end(); ++j) {
					if(j->_contenu[c] && j->_contenu[c]->operator()() == idProba) {
						e.supprimer() = false;
						goto finRecherche;
					}
				}
			}
		}
	finRecherche:
		if(e.supprimer()) {
			_niveau->_probas.erase(_niveau->_probas.begin() + loi);
			ActionEditeur *a = new SupprimerLoiProba(loi, _niveau->_probas[loi]);
			this->posterAction(a);
		}
		else {
			std::vector<Unichar> elem;
			elem.push_back(TRAD("ed annulerSupLoi OK"));
			Menu m(TRAD("La loi de probabilité est utilisée !"), elem);
			m.afficher(0, fond);
		}
	}
	else if(e.modif()) {
		ActionEditeur *a = new ModifierLoiProba(loi, _niveau->_probas[loi], e.nouvelleLoi());
		this->posterAction(a);
	}
}

void EditeurDimensions::preparationDessin() {
	
}

void EditeurDimensions::dessiner() {
	_teinteSelection += 1.0f / 50.0f * (60.0f / Ecran::frequenceInstantanee()) * _sensTeinte;
	if(_teinteSelection > 0.7f) {
		_teinteSelection = 0.7f;
		_sensTeinte = -1;
	}
	else if(_teinteSelection < 0) {
		_teinteSelection = 0;
		_sensTeinte = 1;
	}
	
	
	_enTetes[_selection].definir(Couleur(_teinteSelection * 255));
	_valeurs[_selection].definir(Couleur(_teinteSelection * 255));
		
	Shader::flou(1).activer();
	_ap.afficher(glm::vec2(0));
	Shader::desactiver();
	
	Affichage::afficherRectangle(Ecran::ecran(), Couleur(255, 255, 255, 200));
	
	Rectangle cadre(glm::vec2(80, 80), _titre.dimensions());
	_titre.afficher(cadre.origine());
	
	cadre.haut += cadre.hauteur + 20;
	cadre.gauche += 20;
	
	dimension_t largeurCat = std::max_element(_enTetes.begin(), _enTetes.end(), largeurTexte_t())->dimensions().x;
	dimension_t largeurProba = std::max_element(_valeurs.begin(), _valeurs.end(), largeurTexte_t())->dimensions().x;
	
	dimension_t hauteur = std::accumulate(_enTetes.begin(), _enTetes.end(), dimension_t(0), hauteurTexte_t(10)) - 10;
	
	Affichage::afficherRectangle(Rectangle(cadre.gauche, cadre.haut, largeurCat + largeurProba + 20, hauteur + 20), Couleur(200, 205, 220, 128));
	
	cadre.decaler(glm::vec2(10, 10));
	coordonnee_t sauveHaut = cadre.haut;
	afficheurTexte_t afficheur(cadre, 10);
	for_each(_enTetes.begin(), _enTetes.end(), afficheur);
	cadre.haut = sauveHaut;
	cadre.gauche += largeurCat;
	for_each(_valeurs.begin(), _valeurs.end(), afficheur);
	
	_cadres.clear();
	cadre.gauche -= largeurCat;
	cadre.largeur = largeurCat + largeurProba;
	cadre.haut = sauveHaut;
	initCadres_t initCadres(_cadres, cadre, 10);
	for_each(_enTetes.begin(), _enTetes.end(), initCadres);
	
	cadre.decaler(-glm::vec2(10, -10));
	
	_ok.afficher(cadre.origine());
	_cadreOk.definirOrigine(cadre.origine());
}

void EditeurDimensions::gestionClavier() {
	if(Session::evenement(Session::T_ESC) || Session::evenement(Session::QUITTER) || Session::evenement(Session::T_ENTREE)) {
		_continuer = false;
	}
	else {
		if(Session::evenement(Session::B_GAUCHE)) {
			std::vector<Rectangle>::iterator souris = std::find_if(_cadres.begin(), _cadres.end(), trouveSouris_t());
			if(souris != _cadres.end()) {
				_enTetes[_selection].definir(Couleur::noir);
				_valeurs[_selection].definir(Couleur::noir);
				_selection = static_cast<ElementNiveau::elementNiveau_t>(std::distance(_cadres.begin(), souris));
			}
			else if(_cadreOk.contientPoint(Session::souris())) {
				_continuer = false;
			}
		}
		if(Session::evenement(Session::T_EFFACER)) {
			switch(_selection) {
				case 0:
					_dimX /= 10;
					_valeurs[_selection].definir(nombreVersTexte(_dimX));
					break;
				case 1:
					_dimY /= 10;
					_valeurs[_selection].definir(nombreVersTexte(_dimY));
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
				size_t &p = _selection ? _dimY : _dimX;
				size_t p1 = p;
				p1 = p1 * 10 + nb;
				if(p1 <= TAILLE_MAX_NIVEAU) {
					p = p1;
					_valeurs[_selection].definir(nombreVersTexte(p));
				}
			}
		}
	}
	
	if(!_continuer) {
		Session::supprimerVueFenetre();
	}
}

void Editeur::modifDimensions() {
	Image *ap = Ecran::apercu();
	EditeurDimensions e(*this, *ap);
	
	Session::ajouterVueFenetre(&e);
	
	if(e.dimX() != _niveau->_dimX || e.dimY() != _niveau->_dimY) {
		bool redim = true;
		if(e.dimX() < _niveau->_dimX || e.dimY() < _niveau->_dimY) {
			std::vector<Unichar> elem;
			elem.push_back(TRAD("ed redimNiveauConfirm Continuer"));
			Menu m(TRAD("ed Des éléments seront supprimés"), elem);
			index_t reponse = m.afficher(0, *ap);
			if(reponse != 0)
				redim = false;
		}
		if(redim) {
			ActionEditeur *a = new RedimensionnerNiveau(e.dimX(), e.dimY());
			this->posterAction(a);
		}
	}
	
	delete ap;
}

void Editeur::modifIndex() {
	if(_selection.empty())
		return;

	index_t selection = 0;
	ElementNiveau::elementNiveau_t catSel = ElementNiveau::nbTypesElement;
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
	
	std::vector<Unichar> index;
	index.reserve(ElementNiveau::nbTypesElement);
	for(index_t i = 0; i != ElementNiveau::nombreEntites(catSel); ++i) {
		index.push_back(nombreVersTexte(i));
	}

	Image *ap = Ecran::apercu();
	selection = choisirElement(index, selection, TRAD("ed Choisissez un index :"), *ap, Editeur::ApercuEntite(catSel));
	delete ap;
		
	if(selection != index.size()) {
		_aIndex = selection;
		
		bool proba;
		index_t indexProba, idx;
		ElementNiveau::elementNiveau_t cat;
		std::list<ActionEditeur *> l;
		for(selection_t::iterator i = _selection.begin(); i != _selection.end(); ++i) {
			if(!i->_e)
				idx = selection + 1;
			else
				obtenirInfosEntites(i->_e->operator()(), proba, indexProba, cat, idx);
			if(idx != selection) {
				ElementEditeur *nouveau = new ElementEditeur(catSel, selection);
				ActionEditeur *a = new RemplacerEntite(nouveau, i->_e, i->_posX, i->_posY, _coucheEdition);
				l.push_back(a);
				i->_e = nouveau;
			}
		}
		ActionEditeur *a = new ActionsEditeur(l);
		this->posterAction(a);
	}
}

static void afficheRien(Rectangle const &, index_t, Image const &apercu) {
	
}

void Editeur::ApercuEntite::operator()(Rectangle const &dejaAffiche, index_t selection, Image const &apercu) {
	if(selection != _aSel) {
		delete _elem;
		_elem = new ElementEditeur(_cat, selection);
		_aSel = selection;
	}
	
	_elem->image().afficher(dejaAffiche.origine() + glm::vec2(dejaAffiche.largeur + 40, 0), _elem->cadre());
}

template<typename Afficheur>
void SelecteurElement<Afficheur>::dessiner() {
	_teinteSelection += 1.0f / 50.0f * (60.0f / Ecran::frequenceInstantanee()) * _sensTeinte;
	if(_teinteSelection > 0.7f) {
		_teinteSelection = 0.7f;
		_sensTeinte = -1;
	}
	else if(_teinteSelection < 0) {
		_teinteSelection = 0;
		_sensTeinte = 1;
	}
	
	
	_elements[_selection].definir(Couleur(_teinteSelection * 255));
	
	Shader::flou(1).activer();
	_apercu.afficher(glm::vec2(0));
	Shader::desactiver();
	
	Affichage::afficherRectangle(Ecran::ecran(), Couleur(255, 255, 255, 200));
	
	Rectangle cadre(glm::vec2(80, 80), _titre.dimensions());
	_titre.afficher(cadre.origine());
	
	cadre.haut += cadre.hauteur + 20;
	cadre.gauche += 20;
	
	dimension_t largeur = std::max_element(_elements.begin() + _premierAffiche, _elements.begin() + _premierAffiche + _nbAffiches, largeurTexte_t())->dimensions().x;
	
	dimension_t hauteur = std::accumulate(_elements.begin() + _premierAffiche, _elements.begin() + _premierAffiche + _nbAffiches, dimension_t(0), hauteurTexte_t(10)) - 10;
	
	Rectangle dejaAffiche = cadre.cadreDecale(glm::vec2(largeur, 0));
	
	glm::vec2 pCadre(cadre.gauche + cadre.largeur + 20, cadre.haut);
	Affichage::afficherRectangle(Rectangle(cadre.gauche, cadre.haut, largeur + 20, hauteur + 20), Couleur(200, 205, 220, 128));
	
	cadre.decaler(glm::vec2(10, 10));
	coordonnee_t sauveHaut = cadre.haut;
	afficheurTexte_t afficheur(cadre, 10);
	for_each(_elements.begin() + _premierAffiche, _elements.begin() + _premierAffiche + _nbAffiches, afficheur);
	cadre.haut = sauveHaut;
	_elements[_selection].definir(Couleur::noir);
	
	_cadres.clear();
	cadre.largeur = largeur;
	cadre.haut = sauveHaut;
	initCadres_t initCadres(_cadres, cadre, 10);
	for_each(_elements.begin() + _premierAffiche, _elements.begin() + _premierAffiche + _nbAffiches, initCadres);
	
	cadre.decaler(-glm::vec2(10, -10));
	
	_valider.definir(POLICE_GRANDE, 20 * Ecran::echelleMin());
	_annuler.definir(POLICE_GRANDE, 20 * Ecran::echelleMin());
	
	_cadreValider.definirDimensions(_valider.dimensions());
	_cadreAnnuler.definirDimensions(_annuler.dimensions());
	_cadreValider.definirOrigine(cadre.origine() + glm::vec2(20 * Ecran::echelleMin(), 20 * Ecran::echelleMin() + cadre.hauteur));
	_cadreAnnuler.definirOrigine(_cadreValider.origine() + glm::vec2(80 * Ecran::echelleMin() + _cadreValider.largeur, 0));
	
	_valider.afficher(_cadreValider.origine());
	_annuler.afficher(_cadreAnnuler.origine());
	
	dejaAffiche.definirDimensions(glm::vec2(std::max(dejaAffiche.largeur, _cadreAnnuler.gauche + _cadreAnnuler.largeur) - dejaAffiche.gauche, dejaAffiche.hauteur + _cadreAnnuler.haut + _cadreAnnuler.hauteur));
	
	_afficheur(dejaAffiche, _selection, _apercu);
}

template<typename Afficheur>
void SelecteurElement<Afficheur>::gestionClavier() {
	if(Session::evenement(Session::T_ESC) || Session::evenement(Session::QUITTER)) {
		_continuer = false;
		_selection = _elements.size();
	}
	else if(Session::evenement(Session::T_ENTREE)) {
		_continuer = false;
	}
	else {
		if(Session::evenement(Session::T_HAUT) && horloge() - _ancienDefilement > INTERVALLE_DEFILEMENT) {
			if(_selection > 0) {
				--_selection;
				if(_selection < _premierAffiche)
					--_premierAffiche;
			}
			else {
				_selection = _elements.size() - 1;
				if(_elements.size() > _nbAffiches)
					_premierAffiche = _elements.size() - _nbAffiches;
			}
			
			_ancienDefilement = horloge();
		}
		else if(Session::evenement(Session::T_BAS) && horloge() - _ancienDefilement > INTERVALLE_DEFILEMENT) {
			if(_selection < _elements.size() - 1) {
				++_selection;
				if(_selection >= _premierAffiche + _nbAffiches)
					++_premierAffiche;
			}
			else {
				_selection = ElementNiveau::premierTypeElement;
				_premierAffiche = 0;
			}
			
			_ancienDefilement = horloge();
		}
		if(Session::evenement(Session::B_GAUCHE)) {
			std::vector<Rectangle>::iterator souris = std::find_if(_cadres.begin(), _cadres.end(), trouveSouris_t());
			if(souris != _cadres.end()) {
				_selection = std::distance(_cadres.begin(), souris);
			}
			else if(_cadreAnnuler.contientPoint(Session::souris())) {
				_selection = _elements.size();
				_continuer = false;
			}
			else if(_cadreValider.contientPoint(Session::souris())) {
				_continuer = false;
			}
		}
	}
	
	if(!_continuer) {
		Session::supprimerVueFenetre();
	}
}

template<class Afficheur>
static index_t choisirElement(std::vector<Unichar> const &el, index_t sel, Unichar const &tt, Image const &apercu, Afficheur afficheurSup) {
	SelecteurElement<Afficheur> selecteur(afficheurSup, sel, tt, el, apercu);
	Session::ajouterVueFenetre(&selecteur);
	
	return selecteur.selection();
}

void Editeur::modifCouche() {
	std::vector<Unichar> couches;
	for(Niveau::couche_t c = Niveau::premiereCouche; c != Niveau::nbCouches; ++c) {
		couches.push_back(Niveau::nomCouche(c));
	}
	
	Image *ap = Ecran::apercu();

	index_t selection = choisirElement(couches, _coucheEdition, TRAD("ed Choisissez une couche :"), *ap, &afficheRien);
	delete ap;
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
	txt.push_back(TRAD("ed Loi de probabilité"));
	txt.push_back(TRAD("ed Catégorie + index"));
	if(_aIndex != -1)
		txt.push_back(TRAD("ed Dernière entité utilisée"));
		
	Unichar titre;
	if(_selection.size() == 1)
		titre = TRAD("ed L'entité est définie par :");
	else
		titre = TRAD("ed Les entités sont définies par :");
	
	Image *ap = Ecran::apercu();
	index_t selection = choisirElement(txt, 0, titre, *ap, &afficheRien);
	delete ap;

	if(selection != txt.size()) {
		std::list<ActionEditeur *> l;
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
					_aProba = true;
					_aIndexProba = 0;
					_aIndex = 0;

					ActionEditeur *a = new RemplacerEntite(nouveau, i->_e, i->_posX, i->_posY, _coucheEdition);
					l.push_back(a);

					i->_e = nouveau;
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
					_aProba = false;
					_aIndex = 0;
					_aCat = ElementNiveau::premierTypeElement;
					
					ActionEditeur *a = new RemplacerEntite(nouveau, i->_e, i->_posX, i->_posY, _coucheEdition);
					l.push_back(a);
					i->_e = nouveau;
				}
			}
		}
		else if(selection == 2) {
			for(selection_t::iterator i = _selection.begin(); i != _selection.end(); ++i) {
				ElementEditeur *nouveau = 0;
				if(_aProba)
					nouveau = new ElementEditeur(_aIndexProba);
				else
					nouveau = new ElementEditeur(_aCat, _aIndex);

				ActionEditeur *a = new RemplacerEntite(nouveau, i->_e, i->_posX, i->_posY, _coucheEdition);
				l.push_back(a);
				i->_e = nouveau;
			}
		}		
		ActionEditeur *a = new ActionsEditeur(l);
		this->posterAction(a);
	}
}

void Editeur::modifCategorie() {
	if(_selection.empty())
		return;
	
	ElementNiveau::elementNiveau_t catSel = ElementNiveau::nbTypesElement;
	{
		index_t index;
		bool proba;
		index_t indexProba;
		selection_t::iterator deb = _selection.begin();
		while(deb != _selection.end()) {
			if(deb->_e) {
				obtenirInfosEntites(deb->_e->operator()(), proba, indexProba, catSel, index);
				break;
			}
		}
	}

	std::vector<Unichar> cat;
	for(ElementNiveau::elementNiveau_t c = ElementNiveau::premierTypeElement; c != ElementNiveau::nbTypesElement; ++c) {
		if(ElementNiveau::nombreEntites(c))
			cat.push_back(ElementNiveau::nomCategorie(c));
	}
	
	Image *ap = Ecran::apercu();
	index_t selection = choisirElement(cat, catSel, TRAD("ed Choisissez une catégorie :"), *ap, &afficheRien);
	delete ap;
		
	if(selection != cat.size()) {		
		ElementNiveau::elementNiveau_t nCat = static_cast<ElementNiveau::elementNiveau_t>(selection);
		_aCat = nCat;
		bool proba;
		index_t indexProba, index;
		ElementNiveau::elementNiveau_t cat;
		std::list<ActionEditeur *> l;
		for(selection_t::iterator i = _selection.begin(); i != _selection.end(); ++i) {
			if(!i->_e)
				cat = nCat + 1;
			else
				obtenirInfosEntites(i->_e->operator()(), proba, indexProba, cat, index);
			if(cat != nCat) {
				ElementEditeur *nouveau = new ElementEditeur(nCat, 0); // index 1
				ActionEditeur *a = new RemplacerEntite(nouveau, i->_e, i->_posX, i->_posY, _coucheEdition);
				l.push_back(a);
				i->_e = nouveau;
			}
		}
		ActionEditeur *a = new ActionsEditeur(l);
		this->posterAction(a);
	}
}

void Editeur::remplissageGrille() {
	Image *ap = Ecran::apercu();

	std::vector<Unichar> cat;
	for(ElementNiveau::elementNiveau_t c = ElementNiveau::premierTypeElement; c != ElementNiveau::nbTypesElement; ++c) {
		if(ElementNiveau::nombreEntites(c))
			cat.push_back(ElementNiveau::nomCategorie(c));
	}
	
	index_t selection = choisirElement(cat, 0, TRAD("ed Choisissez une catégorie :"), *ap, &afficheRien);
	
	if(selection != cat.size()) {
		ElementNiveau::elementNiveau_t categorie = static_cast<ElementNiveau::elementNiveau_t>(selection);
		std::vector<Unichar> index;
		index.reserve(ElementNiveau::nbTypesElement);
		for(index_t i = 0; i != ElementNiveau::nombreEntites(categorie); ++i) {
			index.push_back(nombreVersTexte(i));
		}

		index_t selection = choisirElement(index, 0, TRAD("Choisissez un index :"), *ap, Editeur::ApercuEntite(categorie));
		if(selection != index.size()) {
			ElementEditeur el(categorie, selection);
			size_t dimX = el.dimensions().x / LARGEUR_CASE, dimY = el.dimensions().y / LARGEUR_CASE;
			
			index_t xMin = std::numeric_limits<index_t>::max(), yMin = std::numeric_limits<index_t>::max();
			index_t xMax = std::numeric_limits<index_t>::min(), yMax = std::numeric_limits<index_t>::min();
			for(selection_t::iterator i = _selection.begin(); i != _selection.end(); ++i) {
				xMin = std::min(xMin, i->_posX);
				yMin = std::min(yMin, i->_posY);
				xMax = std::max(xMax, i->_posX);
				yMax = std::max(yMax, i->_posY);
			}
						
			std::list<ActionEditeur *> l;
			for(selection_t::iterator i = _selection.begin(); i != _selection.end(); ++i) {
				if((i->_posX - xMin) % dimX == 0 && (i->_posY - yMin) % dimY == 0) {
					ActionEditeur *a = new RemplacerEntite(&el, _niveau->_elements[i->_posY][i->_posX][_coucheEdition], i->_posX, i->_posY, _coucheEdition);
					l.push_back(a);
				}
			}
			ActionEditeur *a = new ActionsEditeur(l);
			this->posterAction(a);
		}
	}
	
	delete ap;
}

void Editeur::selectionnerSemblables() {
	ElementEditeur const *modele = _selection.front()._e;
	_selection.clear();
	
	glm::vec2 pos;
	for(Ligne::iterator i = _niveau->_elements.begin(); i != _niveau->_elements.end(); ++i) {
		pos.x = 0;
		for(Colonne::iterator j = i->begin(); j != i->end(); ++j) {
			if((modele == 0 && j->operator[](_coucheEdition) == 0) || (modele && j->operator[](_coucheEdition) && j->operator[](_coucheEdition)->operator()() == modele->operator()())) {
				_selection.push_back(ElementSelection(j->operator[](_coucheEdition), std::distance(i->begin(), j), std::distance(_niveau->_elements.begin(), i), es_ok));
			}
			pos.x += LARGEUR_CASE;
		}
		pos.y += LARGEUR_CASE;
	}
}

Rectangle const &Editeur::cadreEditeur() {	
	return _cadreEditeur;
}

Rectangle const &Editeur::cadreControles() {
	return _cadreControles;
}

Rectangle const &Editeur::cadreInventaire() {
	return _cadreInventaire;
}

Rectangle const &Editeur::cadreCarte() {
	return _cadreCarte;
}

void Editeur::initCadres() {
	_cadreControles = Rectangle(0, 0, 200, 200);
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
	
	if(n->Attribute("musique"))
		_musique = n->Attribute("musique");

	_elements.resize(_dimY);
	for(Ligne::iterator i = _elements.begin(); i != _elements.end(); ++i) {
		i->resize(_dimX);
	}
	
	if(_dimX <= 0 || _dimY <= 0) {
		throw Exc_ChargementEditeur("Dimensions du niveau invalides !");
	}

	{
		TiXmlElement *probas = n->FirstChildElement("proba");
		
		if(probas) {
			for(TiXmlElement *proba = probas->FirstChildElement(); proba; proba = proba->NextSiblingElement()) {
				std::string valeur = proba->Attribute("valeur");
				std::string nom;
				if(proba->Attribute("nom"))
					nom = proba->Attribute("nom");
				_probas.push_back(LoiProba(nom, valeur));
			}
		}
	}

	for(Niveau::couche_t couche = Niveau::premiereCouche; couche < Niveau::nbCouches; ++couche) {
		TiXmlElement *cc = n->FirstChildElement(Niveau::nomBaliseCouche(couche));
		if(!cc)
			continue;
		char const *texte = cc->GetText();
		size_t tailleAttendue = _dimY * (_dimX * 4 + 1) - 1; // chaque ligne contient 4 chiffres hexa par colonne et un saut de ligne, sauf pour la dernière ligne.
		if(!texte || std::strlen(texte) != tailleAttendue) {
			throw Exc_ChargementEditeur(std::string() + "Erreur : dimensions de la couche " + nombreVersTexte(couche) + " invalides (" + nombreVersTexte(tailleAttendue) +  " attendus) !");
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
	if(e->Attribute("image")) {
		_image = Image(Session::cheminRessources() + e->Attribute("image"));
		_cadre = Rectangle(glm::vec2(0), _image.dimensions());
	}

	_dimensions.x = _dimensions.y = LARGEUR_CASE;
		
	if(e->Attribute("x"))
		e->Attribute("x", &_cadre.gauche);
	if(e->Attribute("y"))
		e->Attribute("y", &_cadre.haut);
	if(e->Attribute("l"))
		e->Attribute("l", &_cadre.largeur);
	if(e->Attribute("h"))
		e->Attribute("h", &_cadre.hauteur);

	if(e->Attribute("oX"))
		e->Attribute("oX", &_origine.x);
	if(e->Attribute("oY"))
		e->Attribute("oY", &_origine.y);
	
	bool multi = false;
	TiXmlElement *el = e->FirstChildElement();
	if(el) {
		if(el->Attribute("nbPoses"))
			multi = true;
	}
	if(multi) {
		TiXmlElement *el = e->FirstChildElement(EntiteMobile::transcriptionAction(EntiteMobile::a_immobile));
		if(el && el->Attribute("image")) {
			_image = Image(Session::cheminRessources() + el->Attribute("image"));
		}
				       
		if(e->Attribute("blocX")) {
			e->Attribute("blocX", &_dimensions.x);
		}
		if(e->Attribute("blocY")) {
			e->Attribute("blocY", &_dimensions.y);
		}
		_cadre.definirDimensions(_dimensions);
		_dimensions = glm::vec2(LARGEUR_CASE, LARGEUR_CASE);

		_origine = glm::vec2(_dimensions.x / 2, _cadre.hauteur * 3 / 4);
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
				_origine -= glm::vec2(LARGEUR_CASE, 0) / 2.0f;
		}
		if(e->Attribute("nbPoses")) {
			int tmp;
			e->Attribute("nbPoses", &tmp);
			_cadre.definirDimensions(glm::vec2(_cadre.largeur / tmp, _cadre.hauteur));
		}
	}
	
	if(!_image.chargee()) {
		throw Exc_ChargementEditeur("Entité invalide, pas d'image !");
	}
}

Editeur::ElementEditeur::ElementEditeur(index_t loiProba) : _proba(true), _indexProba(loiProba), _image(Session::cheminRessources() + "alea.png"), _cadre(), _teinte() {
	_cadre = Rectangle(glm::vec2(0), _image.dimensions());
	_dimensions = glm::vec2(1, 1) * static_cast<coordonnee_t>(LARGEUR_CASE);
	_origine.x = 0;
	_origine.y = _image.dimensions().y / 2;

	std::hash<double> h;
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

glm::vec2 Editeur::ElementEditeur::dimensions() const {
	return _dimensions;
}

glm::vec2 Editeur::ElementEditeur::origine() const {
	return _origine;
}

Couleur Editeur::ElementEditeur::teinte() const {
	return _teinte;
}

Editeur::LoiProba::LoiProba(std::string const &nom, std::string const &proba) : _nom(nom), _proba() {
	if(proba.empty()) {
		std::memset(_proba, 0, ElementNiveau::nbTypesElement * sizeof(int));
	}
	else {
		for(ElementNiveau::elementNiveau_t e = ElementNiveau::premierTypeElement; e != ElementNiveau::nbTypesElement; ++e) {
			int val = caractereVersBase64(proba[e * CHIFFRES_VALEURS_PROBA_ENTITES]) * BASE_VALEURS_PROBA_ENTITES + caractereVersBase64(proba[e * CHIFFRES_VALEURS_PROBA_ENTITES + 1]);
			_proba[e] = val;
		}
	}
}

void Editeur::RemplacerEntite::operator()() {
	Editeur *ed = Editeur::editeur();
	ElementEditeur *&e = ed->_niveau->_elements[_y][_x]._contenu[_c];
	delete e;
	if(_remplacant) {
		e = new ElementEditeur(*_remplacant);
	}
	else {
		e = 0;
	}
}

Editeur::RedimensionnerNiveau::RedimensionnerNiveau(size_t dimX, size_t dimY) : _dX(dimX), _dY(dimY), _aDX(Editeur::editeur()->_niveau->_dimX), _aDY(Editeur::editeur()->_niveau->_dimY) {					
	NiveauEditeur *n = Editeur::editeur()->_niveau;
	for(Ligne::iterator i = n->_elements.begin(); i != n->_elements.end(); ++i) {
		for(Colonne::iterator j = i->begin(); j != i->end(); ++j) {
			for(Niveau::couche_t c = Niveau::premiereCouche; c != Niveau::nbCouches; ++c) {
				index_t x = std::distance(i->begin(), j), y = std::distance(n->_elements.begin(), i);
				if(y >= dimY || x >= dimX) {
					_cases.push_back(new RemplacerEntite(0, j->operator[](c), x, y, c));
				}
			}
		}
	}
}

Editeur::RedimensionnerNiveau::RedimensionnerNiveau(size_t dimX, size_t dimY, std::list<ActionEditeur *> remplacements) : _dX(dimX), _dY(dimY), _aDX(Editeur::editeur()->_niveau->_dimX), _aDY(Editeur::editeur()->_niveau->_dimY), _cases(remplacements) {
	
}

void Editeur::RedimensionnerNiveau::operator()() {
	NiveauEditeur *n = Editeur::editeur()->_niveau;

	for(std::list<ActionEditeur *>::iterator i = _cases.begin(); i != _cases.end(); ++i) {
		RemplacerEntite *r = static_cast<RemplacerEntite *>(*i);
		if(r->remplace())
			r->operator()();
	}
	n->_elements.resize(_dY);
	for(Ligne::iterator i = n->_elements.begin(); i != n->_elements.end(); ++i) {
		i->resize(_dX);
	}
	n->_dimX = _dX;
	n->_dimY = _dY;
	
	for(std::list<ActionEditeur *>::iterator i = _cases.begin(); i != _cases.end(); ++i) {
		RemplacerEntite *r = static_cast<RemplacerEntite *>(*i);
		if(r->remplacant())
			r->operator()();
	}
}
