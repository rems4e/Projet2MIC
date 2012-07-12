//
//  Joueur.cpp
//  Projet2MIC
//
//  Créé par Marc Promé et Rémi Saurel.
//  Ce fichier et son contenu sont librement distribuables, modifiables et utilisables pour toute œuvre non commerciale, à condition d'en citer les auteurs.
//


#include "Joueur.h"
#include "Session.h"
#include "Partie.h"
#include "Marchand.h"
#include "UtilitaireNiveau.h"
#include "tinyxml.h"
#include <cmath>

Joueur::Joueur(bool decoupagePerspective, Niveau *n, uindex_t index, ElementNiveau::elementNiveau_t cat) : Personnage(decoupagePerspective, n, index, cat, new InventaireJoueur(*this)), _inventaireAffiche(false), _xp(0), _niveauXp(1) {

}

Joueur::~Joueur() {
	
}

void Joueur::afficher(index_t deltaY, Coordonnees const &decalage) const {
	if(_invincible) {
		static int v = 128;
		static int sens = -1;
		v += sens * 8.0 * 60.0 / Ecran::frequenceInstantanee();
		if(v < 0) {
			v = 0;
			sens = 1;
		}
		else if(v > 255) {
			v = 255;
			sens = -1;
		}

		Image::definirTeinte(Couleur(v, (v + 128) % 255, (v + 200) % 255));
	}
	this->Personnage::afficher(deltaY, decalage);
	Image::definirTeinte(Couleur::blanc);
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
		_interaction = ij_aucune;

		dep.normaliser();
		dep *= this->vitesse() * 60.0 / Ecran::frequenceInstantanee();
		if(this->definirAction(EntiteMobile::a_deplacer)) {
			this->deplacerPosition(dep);
		}
	}
	if(dep.vecteurNul()) {
		Personnage *interaction = this->Personnage::interagir(true);
		Niveau::listeElements_t listeObjets = this->niveau()->elements(this->pX(), this->pY(), Niveau::cn_objetsInventaire);
		_nombreObjets.first = _nombreObjets.second = 0;
		if(!interaction) {
			_nombreObjets.first = std::distance(listeObjets.first, listeObjets.second);
			_nombreObjets.second = this->niveau()->monnaie(this->pX(), this->pY());
			if(_nombreObjets.first || _nombreObjets.second) {
				_interaction = ij_ramasser;
			}
			else {
				Niveau::listeElements_t liste = this->niveau()->elements(this->pX(), this->pY(), Niveau::cn_sol);
				if(liste.first != liste.second) {
					ElementNiveau *e = liste.first->_entite;
					if(e->categorie() == ElementNiveau::teleporteur && e->index() == 1 && this->peutTerminerNiveau()) {
						_interaction = ij_finirNiveau;
					}
				}
			}
		}
		
		if(Session::evenement(Parametres::evenementAction(Parametres::interagir))) {
			if(interaction)
				this->interagir(interaction, false);
			else {
				Niveau::listeElements_t liste = this->niveau()->elements(this->pX(), this->pY(), Niveau::cn_sol);
				bool ok = false;
				if(liste.first != liste.second) {
					ElementNiveau *e = liste.first->_entite;
					if(e->categorie() == ElementNiveau::teleporteur && e->index() == 1 && this->peutTerminerNiveau()) {
						Partie::partie()->terminerNiveau();
						ok = true;
					}
				}
				
				if(!ok) {
					index_t x = this->pX(), y = this->pY();
					
					this->inventaire()->modifierMonnaie(this->niveau()->monnaie(x, y));
					this->niveau()->modifierMonnaie(x, y, -this->niveau()->monnaie(x, y));
					
					for(Niveau::elements_t::iterator el = listeObjets.first; el != listeObjets.second;) {
						if(this->inventaire()->ajouterObjet(static_cast<ObjetInventaire *>(el->_entite))) {
							el = this->niveau()->supprimerElement(el, x, y, Niveau::cn_objetsInventaire, false);
						}
						else {
							++el;
						}
					}
				}
				
			}
			Session::reinitialiser(Parametres::evenementAction(Parametres::interagir));
		}
		else if(Session::evenement(Session::T_m)) {
			this->modifierVieActuelle(-this->vieTotale());
		}
		else
			this->definirAction(EntiteMobile::a_immobile);
	}
}

bool Joueur::interagir(Personnage *p, bool test) {
	_interaction = ij_aucune;
	
	switch(p->categorieMobile()) {
		case EntiteMobile::em_ennemi:
			if(!test)
				this->attaquer(p);
			_interaction = ij_attaquer;
			_attaque = p;
			
			return true;
		case EntiteMobile::em_marchand:
			if(!test)
				Partie::partie()->definirMarchand(static_cast<Marchand *>(p));
			_interaction = ij_commerce;
			
			return true;
		case EntiteMobile::em_joueur:
			break;
	}
	
	return false;
}

Joueur::interactionJoueur_t Joueur::interaction() const {
	return _interaction;
}

std::pair<size_t, size_t> Joueur::nombreObjets() const {
	return _nombreObjets;
}

Personnage *Joueur::attaque() const {
	return _attaque;
}

void Joueur::renaitre() {
	this->competences()[force] = 10;
	this->competences()[agilite] = 5;
	this->competences()[endurance] = 5;

	this->Personnage::modifierVieActuelle(-this->vieActuelle() + this->vieTotale());
	this->Personnage::renaitre();
	this->inventaire()->modifierMonnaie(50);

	_niveauXp = 1;
	_xp = 0;
	_invincible = false;
}

bool Joueur::invincible() const {
	return _invincible;
}

void Joueur::definirInvicible(bool i) {
	_invincible = i;
}

void Joueur::modifierVieActuelle(int d) {
	if(!_invincible) {
		this->Personnage::modifierVieActuelle(d);
	}
}

void Joueur::mourir() {
	Personnage::mourir();
}

bool Joueur::joueur() const {
	return true;
}

size_t Joueur::xp() const {
	return _xp;
}

size_t Joueur::xpTotale() const {
	return std::exp((this->niveauXp() - 1) / 5.0) * 100;
}

index_t Joueur::niveauXp() const {
	return _niveauXp;
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

bool Joueur::peutTerminerNiveau() const {
	for(InventaireJoueur::const_iterator i = static_cast<InventaireJoueur const *>(this->inventaire())->debut(); i != static_cast<InventaireJoueur const *>(this->inventaire())->fin(); ++i) {
		if(*i && (*i)->index() == 666)
			return true;
	}
	
	return false;
}

void Joueur::gagnerXp(Personnage *p) {
	size_t xp = 0;
	for(competences_t c = premiereCompetence; c != nbCompetences; ++c) {
		xp += this->attaque()->competences()[c];
	}
	_xp += xp;
	size_t xpNiveau = this->xpTotale();
	if(_xp >= xpNiveau) {
		_xp %= xpNiveau;
		++_niveauXp;
		Competences cc = this->competences();
		for(competences_t c = premiereCompetence; c != nbCompetences; ++c) {
			cc[c] += std::max<size_t>(1, cc[c] * 0.1);
		}
		this->definirCompetences(cc);
	}
}

TiXmlElement *Joueur::sauvegarde() const {
	TiXmlElement *el = new TiXmlElement("Joueur");
	
	el->SetAttribute("vie", this->vieActuelle());
	el->SetAttribute("xp", this->xp());
	el->SetAttribute("niveauXp", this->niveauXp());
	
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
	if(vie)
		this->renaitre();
	
	
	if(el->Attribute("xp"))
		el->Attribute("xp", &_xp);
	if(el->Attribute("niveauXp"))
		el->Attribute("niveauXp", &_niveauXp);
	
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

