//
//  Inventaire.cpp
//  Projet2MIC
//
//  Created by Rémi Saurel on 08/04/12.
//  Copyright (c) 2012 Rémi Saurel. All rights reserved.
//

#include "Inventaire.h"
#include <vector>
#include <list>
#include <limits>
#include "Session.h"
#include <algorithm>
#include "Joueur.h"
#include "Partie.h"
#include "Niveau.h"

Inventaire::Inventaire(Personnage &perso) : _perso(perso) {
	
}

Inventaire::~Inventaire() {
	
}

Personnage const &Inventaire::personnage() const {
	return  _perso;
}

Personnage &Inventaire::personnage() {
	return  _perso;
}

template <template <class e, class = std::allocator<e> > class Conteneur>
template <typename InputIterator>
InventaireC<Conteneur>::InventaireC(Personnage &perso, size_t capacite, InputIterator debut, InputIterator fin) : Inventaire(perso), _elements(debut, fin), _capacite(capacite) {
	if(capacite < std::distance(debut, fin)) {
		std::cerr << "initialisation erronée de l'inventaire : trop d'objets pour sa capacité. fuite de mémoire." << std::endl;
		iterator i = this->debut();
		std::advance(i, _capacite);
		for(; i != this->end();) {
			i = _elements.erase(i);
		}
	}
}

template <template <class e, class = std::allocator<e> > class Conteneur>
InventaireC<Conteneur>::InventaireC(Personnage &perso, size_t capacite) : Inventaire(perso), _elements(), _capacite(capacite) {
}

template <template <class e, class = std::allocator<e> > class Conteneur>
InventaireC<Conteneur>::~InventaireC() {
	for(iterator i = _elements.begin(); i != _elements.end(); ++i) {
		delete *i;
	}
}

template <template <class e, class = std::allocator<e> > class Conteneur>
size_t InventaireC<Conteneur>::nombreObjets() const {
	return _elements.size();
}

template <template <class e, class = std::allocator<e> > class Conteneur>
size_t InventaireC<Conteneur>::capacite() const {
	return _capacite;
}

template <template <class e, class = std::allocator<e> > class Conteneur>
void InventaireC<Conteneur>::definirCapacite(size_t c) {
	if(c < this->nombreObjets())
		return;
	
	_capacite = _capacite;
}

template <template <class e, class = std::allocator<e> > class Conteneur>
typename InventaireC<Conteneur>::const_iterator InventaireC<Conteneur>::debut() const {
	return _elements.begin();
}

template <template <class e, class = std::allocator<e> > class Conteneur>
typename InventaireC<Conteneur>::const_iterator InventaireC<Conteneur>::fin() const {
	return _elements.end();
}

template <template <class e, class = std::allocator<e> > class Conteneur>
typename InventaireC<Conteneur>::iterator InventaireC<Conteneur>::debut() {
	return _elements.begin();
}

template <template <class e, class = std::allocator<e> > class Conteneur>
typename InventaireC<Conteneur>::iterator InventaireC<Conteneur>::fin() {
	return _elements.end();
}

template <template <class e, class = std::allocator<e> > class Conteneur>
bool InventaireC<Conteneur>::ajouterObjet(ObjetInventaire *o) {
	if(this->nombreObjets() < this->capacite()) {
		_elements.push_back(o);
		return true;
	}
	return false;
}

template <template <class e, class = std::allocator<e> > class Conteneur>
void InventaireC<Conteneur>::supprimerObjet(ObjetInventaire *o) {	
	for(iterator i = _elements.begin(); i != _elements.end(); ++i) {
		if(*i == o) {
			_elements.erase(i._base);
			break;
		}
	}
}

template <template <class e, class = std::allocator<e> > class Conteneur>
void InventaireC<Conteneur>::vider() {
	for(iterator i = this->debut(); i != this->fin(); ++i) {
		delete *i;
	}
	_elements.clear();
}

template<typename InputIterator>
InventaireListe::InventaireListe(Personnage &perso, size_t capacite, InputIterator debut, InputIterator fin) : InventaireC<std::list>(perso, capacite, debut, fin) {
	
}

InventaireListe::InventaireListe(Personnage &perso, size_t capacite) : InventaireC<std::list>(perso, capacite)  {
	
}

InventaireListe::~InventaireListe() {
	
}

void InventaireListe::afficher() const {

}

void InventaireListe::gestionEvenements() {
	
}

template<typename InputIterator>
InventaireSol::InventaireSol(Personnage &perso, InputIterator debut, InputIterator fin) : InventaireListe(perso, std::numeric_limits<size_t>::max(), debut, fin) {
	
}

InventaireSol::InventaireSol(Personnage &perso) : InventaireListe(perso, std::numeric_limits<size_t>::max()) {
	
}

InventaireSol::~InventaireSol() {
	
}

void InventaireSol::definirCapacite(size_t c) {
	
}

void InventaireSol::afficher() const {
	
}

void InventaireSol::gestionEvenements() {
	
}

template<typename InputIterator>
InventaireTableau::InventaireTableau(Personnage &perso, size_t largeur, size_t hauteur, InputIterator debut, InputIterator fin) : InventaireC<std::vector>(perso, largeur * hauteur, debut, fin), _largeur(largeur), _hauteur(hauteur) {
	size_t nb = std::distance(debut, fin);
	size_t manquants = this->capacite() - nb;
	for(index_t i = 0; i < manquants; ++i) {
		this->ajouterObjet(0);
	}
}

InventaireTableau::InventaireTableau(Personnage &perso, size_t largeur, size_t hauteur) : InventaireC<std::vector>(perso, largeur * hauteur), _largeur(largeur), _hauteur(hauteur) {
	for(index_t i = 0; i < this->capacite(); ++i) {
		this->InventaireC<std::vector>::ajouterObjet(0);
	}
}

InventaireTableau::~InventaireTableau() {
	
}

void InventaireTableau::afficher() const {
	
}

size_t InventaireTableau::nombreObjets() const {
	size_t nb = 0;
	for(const_iterator i = debut(); i != fin(); ++i) {
		if(*i)
			++nb;
	}
	
	return nb;
}

bool InventaireTableau::ajouterObjet(ObjetInventaire *o) {
	if(!o)
		return false;
	for(index_t i = 0; i < this->capacite(); ++i) {
		if(this->ajouterObjetEnPosition(o, i))
			return true;
	}

	return false;
}

bool InventaireTableau::ajouterObjetEnPosition(ObjetInventaire *o, index_t position) {
	if(!o)
		return false;
	size_t dimX = o->dimensionsInventaire().x, dimY = o->dimensionsInventaire().y;
	index_t pX = position % _largeur, pY = position / _largeur;

	
	for(index_t x = 0; x < dimX; ++x) {
		for(index_t y = 0; y < dimY; ++y) {
			if(x + pX >= _largeur || y + pY >= _hauteur) {
				return false;
			}
			if(this->objetDansCase(x + pX, y + pY)) {
				return false;
			}
		}
	}
	
	(*(this->debut()._base + position)) = o;
	
	return true;
}

void InventaireTableau::supprimerObjet(ObjetInventaire *o) {
	if(!o)
		return;
	std::vector<ObjetInventaire *>::iterator pos = std::find(this->debut()._base, this->fin()._base, o);
	if(pos != this->fin()._base) {
		*pos = 0;
	}
}

ObjetInventaire *InventaireTableau::objetDansCase(index_t position) {
	return this->objetDansCase(position % _largeur, position / _largeur);
}

ObjetInventaire *InventaireTableau::objetDansCase(index_t pX, index_t pY) {
	const_iterator deb = this->debut();
	for(index_t x = 0; x <= pX; ++x) {
		for(index_t y = 0; y <= pY; ++y) {
			index_t p = y * _largeur + x;
			if(!comprisEntre<index_t>(p, 0, this->capacite() - 1))
				return 0;
			ObjetInventaire *o = *(deb._base + p);
			if(o && x + o->dimensionsInventaire().x > pX && y + o->dimensionsInventaire().y > pY) {
				return o;
			}
		}
	}
	
	return 0;
}

bool InventaireTableau::peutPlacerObjetDansCase(index_t pX, index_t pY, ObjetInventaire *objet, ObjetInventaire *&remp) {
	remp = 0;
	if(pX + objet->dimensionsInventaire().x > this->largeur() || pY + objet->dimensionsInventaire().y > this->hauteur())
		return false;
	
	for(index_t x = 0; x < objet->dimensionsInventaire().x; ++x) {
		for(index_t y = 0; y < objet->dimensionsInventaire().y; ++y) {
			ObjetInventaire *obj = this->objetDansCase(pX + x, pY + y);
			if(remp == 0)
				remp = obj;
			else if(obj && remp != obj) {
				return false;
			}
		}
	}

	return true;
}

index_t InventaireTableau::largeur() const {
	return _largeur;
}

index_t InventaireTableau::hauteur() const {
	return _hauteur;
}

Coordonnees InventaireTableau::positionObjet(ObjetInventaire *o) const {
	for(const_iterator i = this->debut(); i != this->fin(); ++i) {
		if(*i == o) {
			index_t pos = std::distance(this->debut()._base, i._base);
			return Coordonnees(pos % _largeur, pos / _largeur);
		}
	}
	
	return Coordonnees::aucun;
}

void InventaireTableau::vider() {
	this->InventaireC<std::vector>::vider();
	for(int i = 0; i < this->capacite(); ++i) {
		this->InventaireC<std::vector>::ajouterObjet(0);
	}
}

/*InventaireMarchand::InventaireMarchand(Marchand &perso) : InventaireTableau(perso, LARGEUR_MARCHAND, HAUTEUR_MARCHAND) {
	
}

InventaireMarchand::~InventaireMarchand() {
	
}

void InventaireMarchand::afficher() const {
	
}

void InventaireMarchand::gestionEvenements() {
	
}

void InventaireMarchand::definirCapacite(size_t c) {
	
}*/

InventaireJoueur::InventaireJoueur(Joueur &perso) : InventaireTableau(perso, LARGEUR_JOUEUR, HAUTEUR_JOUEUR), _fond(Session::cheminRessources() + "inventaire.png"), _objetTransfert(0) {
	_inventaire = Rectangle(21, 282, 365, 150);
	_tenue[Personnage::brasG] = Rectangle(22, 51, 76, 136);
	_tenue[Personnage::brasD] = Rectangle(308, 51, 76, 136);
	_tenue[Personnage::casque] = Rectangle(162, 2, 80, 72);
	_tenue[Personnage::armure] = Rectangle(164, 83, 80, 105);
	_tenue[Personnage::gants] = Rectangle(19, 200, 80, 75);
	_tenue[Personnage::bottes] = Rectangle(307, 200, 80, 75);
}

InventaireJoueur::~InventaireJoueur() {
	delete _objetTransfert;
}

void InventaireJoueur::afficher() const {
	_fond.afficher(Coordonnees());

	Coordonnees pos(32, 288);
	index_t i = 0;
	for(const_iterator j = this->debut(); j != this->fin(); ++j) {
		if(*j) {
			(*j)->image().afficher(pos);
		}
		pos.x += 36;
		++i;
		if((i % this->largeur()) == 0) {
			pos.x = 32;
			pos.y += 37;
		}
	}
	for(Personnage::positionTenue_t i = Personnage::premierePositionTenue; i != Personnage::nbPositionsTenue; ++i) {
		if(this->personnage().tenue(i)) {
			Image const &img = this->personnage().tenue(i)->image();
			img.afficher(_tenue[i].origine() + (_tenue[i].dimensions() - img.dimensions()) / 2);

		}
	}
	
	if(_surlignage != Rectangle::aucun) {
		Ecran::afficherRectangle(_surlignage, _couleurSurlignage);
	}
	if(_surlignageTransfert != Rectangle::aucun) {
		Ecran::afficherRectangle(_surlignageTransfert.intersection(_inventaire), _couleurSurlignageTransfert);
	}
	
	if(_objetTransfert) {
		_objetTransfert->image().afficher(Session::souris());
	}
	
	/*Ecran::afficherRectangle(_inventaire, Couleur(255, 0, 0, 128));
	for(Personnage::positionTenue_t i = Personnage::premierePositionTenue; i != Personnage::nbPositionsTenue; ++i) {
		Ecran::afficherRectangle(_tenue[i], Couleur(255, 0, 0 + i * 40, 128));
	}*/
}

void InventaireJoueur::gestionEvenements() {
	_surlignage = Rectangle::aucun;
	_surlignageTransfert = Rectangle::aucun;

	if(Session::souris() < Partie::partie()->zoneJeu() && Session::evenement(Session::B_GAUCHE)) {
		if(_objetTransfert) {
			this->personnage().niveau()->ajouterElement(this->personnage().pX(), this->personnage().pY(), Niveau::cn_objetsInventaire, _objetTransfert);
			_objetTransfert = 0;
		}
		Session::reinitialiser(Session::B_GAUCHE);
	}
	else if(Session::souris() < _inventaire) {
		Coordonnees pos = Session::souris() - _inventaire.origine();
		index_t pX = pos.x / 36, pY = pos.y / 37;
		if(comprisEntre<index_t>(pX, 0, this->largeur() - 1) && comprisEntre<index_t>(pY, 0, this->hauteur() - 1)) {
			if(Session::evenement(Session::B_GAUCHE)) {
				if(_objetTransfert) {
					ObjetInventaire *remp;
					bool ok = this->peutPlacerObjetDansCase(pX, pY, _objetTransfert, remp);
					if(ok) {
						this->supprimerObjet(remp);
						this->ajouterObjetEnPosition(_objetTransfert, pY * this->largeur() + pX);
						_objetTransfert = remp;
					}
				}
				else {
					ObjetInventaire *o = this->objetDansCase(pX, pY);
					_objetTransfert = o;
					this->supprimerObjet(o);
				}
				Session::reinitialiser(Session::B_GAUCHE);
			}
			else {
				ObjetInventaire *o = this->objetDansCase(pX, pY);
				if(o) {
					Coordonnees positionCase = this->positionObjet(o);
					_surlignage = Rectangle(_inventaire.origine() + Coordonnees(positionCase.x * 36, positionCase.y * 37), Coordonnees(o->dimensionsInventaire().x * 36, o->dimensionsInventaire().y * 37));
					_couleurSurlignage = Couleur(Couleur::blanc, 66);
				}
				if(_objetTransfert) {
					_surlignageTransfert = Rectangle(_inventaire.origine() + Coordonnees(pX * 36, pY * 37), Coordonnees(_objetTransfert->dimensionsInventaire().x * 36, _objetTransfert->dimensionsInventaire().y * 37));
					ObjetInventaire *remp;
					bool ok = this->peutPlacerObjetDansCase(pX, pY, _objetTransfert, remp);
					if(ok) {
						if(remp) {
							Coordonnees positionCase = this->positionObjet(remp);
							_surlignage = Rectangle(_inventaire.origine() + Coordonnees(positionCase.x * 36, positionCase.y * 37), Coordonnees(remp->dimensionsInventaire().x * 36, remp->dimensionsInventaire().y * 37));
							_couleurSurlignage = Couleur(Couleur::blanc, 33);
							_couleurSurlignageTransfert = Couleur(Couleur::blanc, 66);
						}
						else {
							_couleurSurlignageTransfert = Couleur(Couleur::blanc, 66);
						}
					}
					else {
						_couleurSurlignageTransfert = Couleur(Couleur::rouge, 50);
						_surlignage = Rectangle::aucun;
					}
				}
			}
		}
	}
	else {
		for(Personnage::positionTenue_t i = Personnage::premierePositionTenue; i != Personnage::nbPositionsTenue; ++i) {
			if(Session::souris() < _tenue[i]) {
				if(Session::evenement(Session::B_GAUCHE)) {
					if(_objetTransfert) {
						if(this->personnage().peutEquiperObjet(_objetTransfert, i)) {
							ObjetInventaire *o = _objetTransfert;
							_objetTransfert = this->personnage().tenue(i);
							this->personnage().definirTenue(i, o);
						}
					}
					else {
						_objetTransfert = this->personnage().tenue(i);
						this->personnage().definirTenue(i, 0);
					}
					Session::reinitialiser(Session::B_GAUCHE);
				}
				else {
					if(_objetTransfert) {
						if(this->personnage().peutEquiperObjet(_objetTransfert, i)) {
							_couleurSurlignage = Couleur(Couleur::blanc, 66);
						}
						else {
							_couleurSurlignage = Couleur(Couleur::rouge, 50);
						}
						_surlignage = _tenue[i];
					}
					else if(this->personnage().tenue(i)) {
						_couleurSurlignage = Couleur(Couleur::blanc, 66);
						_surlignage = _tenue[i];
					}
				}
				break;
			}
		}
	}
	if(_objetTransfert) {
		Ecran::definirPointeur(&_objetTransfert->image());
	}
}

void InventaireJoueur::definirCapacite(size_t c) {
	
}
