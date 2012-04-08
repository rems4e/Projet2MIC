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

template <template <class e, class = std::allocator<e> > class Conteneur>
template <typename InputIterator>
Inventaire<Conteneur>::Inventaire(size_t capacite, InputIterator debut, InputIterator fin) : _elements(debut, fin), _capacite(capacite) {
	if(capacite < std::distance(debut, fin)) {
		std::cerr << "initialisation erronée de l'inventaire : trop d'objets pour sa capacité. fuite de mémoire." << std::endl;
		iterator i = this->debut();
		std::advance(i, _capacite);
		for(; i != this->end();) {
			delete *i;
			i = _elements.erase(i);
		}
	}
}

template <template <class e, class = std::allocator<e> > class Conteneur>
Inventaire<Conteneur>::Inventaire(size_t capacite) : _elements(), _capacite(capacite) {
}

template <template <class e, class = std::allocator<e> > class Conteneur>
Inventaire<Conteneur>::~Inventaire() {
	for(iterator i = _elements.begin(); i != _elements.end(); ++i) {
		delete *i;
	}
}

template <template <class e, class = std::allocator<e> > class Conteneur>
size_t Inventaire<Conteneur>::nombreObjets() const {
	return _elements.size();
}

template <template <class e, class = std::allocator<e> > class Conteneur>
size_t Inventaire<Conteneur>::capacite() const {
	return _capacite;
}

template <template <class e, class = std::allocator<e> > class Conteneur>
void Inventaire<Conteneur>::definirCapacite(size_t c) {
	if(c < this->nombreObjets())
		return;
	
	_capacite = _capacite;
}

template <template <class e, class = std::allocator<e> > class Conteneur>
typename Inventaire<Conteneur>::const_iterator Inventaire<Conteneur>::debut() const {
	return _elements.begin();
}

template <template <class e, class = std::allocator<e> > class Conteneur>
typename Inventaire<Conteneur>::const_iterator Inventaire<Conteneur>::fin() const {
	return _elements.end();
}

template <template <class e, class = std::allocator<e> > class Conteneur>
typename Inventaire<Conteneur>::iterator Inventaire<Conteneur>::debut() {
	return _elements.begin();
}

template <template <class e, class = std::allocator<e> > class Conteneur>
typename Inventaire<Conteneur>::iterator Inventaire<Conteneur>::fin() {
	return _elements.end();
}

template <template <class e, class = std::allocator<e> > class Conteneur>
void Inventaire<Conteneur>::ajouterObjet(ObjetInventaire *o) {
	if(this->nombreObjets() < this->capacite())
		_elements.push_back(o);
}

template <template <class e, class = std::allocator<e> > class Conteneur>
void Inventaire<Conteneur>::supprimerObjet(ObjetInventaire *o) {	
	for(iterator i = _elements.begin(); i != _elements.end(); ++i) {
		if(*i == o) {
			_elements.erase(i);
			break;
		}
	}
}

template<typename InputIterator>
InventaireListe::InventaireListe(size_t capacite, InputIterator debut, InputIterator fin) : Inventaire(capacite, debut, fin) {
	
}

InventaireListe::InventaireListe(size_t capacite) : Inventaire(capacite)  {
	
}

InventaireListe::~InventaireListe() {
	
}

void InventaireListe::afficher() const {

}

template<typename InputIterator>
InventaireSol::InventaireSol(InputIterator debut, InputIterator fin) : InventaireListe(std::numeric_limits<size_t>::max(), debut, fin) {
	
}

InventaireSol::InventaireSol() : InventaireListe(std::numeric_limits<size_t>::max()) {
	
}

InventaireSol::~InventaireSol() {
	
}

void InventaireSol::definirCapacite(size_t c) {
	
}

void InventaireSol::afficher() const {
	
}

template<typename InputIterator>
InventaireTableau::InventaireTableau(size_t capacite, InputIterator debut, InputIterator fin) : Inventaire(capacite, debut, fin) {
	size_t nb = std::distance(debut, fin);
	size_t manquants = capacite - nb;
	for(index_t i = 0; i < manquants; ++i) {
		this->ajouterObjet(0);
	}
}

InventaireTableau::InventaireTableau(size_t capacite) : Inventaire(capacite) {
	for(index_t i = 0; i < capacite; ++i) {
		this->ajouterObjet(0);
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

void InventaireTableau::ajouterObjetEnPosition(ObjetInventaire *o, index_t position) {
	if(*(this->debut() + position))
		return;
	else
		(*(this->debut() + position)) = o;	
}

void InventaireTableau::supprimerObjet(ObjetInventaire *o) {
	if(!o)
		return;
	this->Inventaire::supprimerObjet(o);
}

InventaireMarchand::InventaireMarchand() : InventaireTableau(CAPACITE_MARCHAND) {
	
}

InventaireMarchand::~InventaireMarchand() {
	
}

void InventaireMarchand::afficher() const {
	
}

void InventaireMarchand::definirCapacite(size_t c) {
	
}

InventaireJoueur::InventaireJoueur() : InventaireTableau(CAPACITE_JOUEUR), _fond(Session::cheminRessources() + "inventaire.png") {
	
}

InventaireJoueur::~InventaireJoueur() {
	
}

void InventaireJoueur::afficher() const {
	_fond.afficher(Coordonnees());
}

void InventaireJoueur::definirCapacite(size_t c) {
	
}
