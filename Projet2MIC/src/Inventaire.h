//
//  Inventaire.h
//  Projet2MIC
//
//  Created by Rémi Saurel on 08/04/12.
//  Copyright (c) 2012 Rémi Saurel. All rights reserved.
//

#ifndef Projet2MIC_Inventaire_h
#define Projet2MIC_Inventaire_h

#define CAPACITE_MARCHAND 16
#define CAPACITE_JOUEUR 16

#include "ObjetInventaire.h"

#include <list>
#include <vector>

template<template <class e, class = std::allocator<e> > class Conteneur>
class Inventaire {
public:
	typedef typename Conteneur<ObjetInventaire *>::const_iterator const_iterator;
	typedef typename Conteneur<ObjetInventaire *>::iterator iterator;
	
	template<typename InputIterator>
	Inventaire(size_t capacite, InputIterator debut, InputIterator fin);
	Inventaire(size_t capacite);
	virtual ~Inventaire();
	
	size_t capacite() const;
	void definirCapacite(size_t c);
	
	virtual size_t nombreObjets() const;
	
	virtual void ajouterObjet(ObjetInventaire *o);
	virtual void supprimerObjet(ObjetInventaire *o);
	
	virtual void afficher() const = 0;

	const_iterator debut() const;
	const_iterator fin() const;
		
protected:
	iterator debut();
	iterator fin();

private:
	Conteneur<ObjetInventaire *> _elements;
	size_t _capacite;
};

class InventaireListe : public Inventaire<std::list> {
public:
	typedef Inventaire::const_iterator const_iterator;
	typedef Inventaire::iterator iterator;
	
	template<typename InputIterator>
	InventaireListe(size_t capacite, InputIterator debut, InputIterator fin);
	InventaireListe(size_t capacite);
	virtual ~InventaireListe();
			
private:
	virtual void afficher() const;
};

typedef InventaireListe InventaireEnnemi;

class InventaireSol : public InventaireListe {
public:
	typedef Inventaire::const_iterator const_iterator;
	typedef Inventaire::iterator iterator;
	
	template<typename InputIterator>
	InventaireSol(InputIterator debut, InputIterator fin);
	InventaireSol();
	~InventaireSol();

	virtual void afficher() const;
		
private:
	virtual void definirCapacite(size_t c);
};

class InventaireTableau : public Inventaire<std::vector> {
public:
	typedef Inventaire::const_iterator const_iterator;
	typedef Inventaire::iterator iterator;
	
	template<typename InputIterator>
	InventaireTableau(size_t capacite, InputIterator debut, InputIterator fin);
	InventaireTableau(size_t capacite);
	virtual ~InventaireTableau();
	
	void ajouterObjetEnPosition(ObjetInventaire *o, index_t position);
	virtual void supprimerObjet(ObjetInventaire *o);
	virtual size_t nombreObjets() const;

private:
	virtual void afficher() const;
};

class InventaireMarchand : public InventaireTableau {
public:
	typedef Inventaire::const_iterator const_iterator;
	typedef Inventaire::iterator iterator;
	
	InventaireMarchand();
	virtual ~InventaireMarchand();
		
	virtual void afficher() const;

private:
	virtual void definirCapacite(size_t c);
};

class InventaireJoueur : public InventaireTableau {
public:
	typedef Inventaire::const_iterator const_iterator;
	typedef Inventaire::iterator iterator;
	
	InventaireJoueur();
	virtual ~InventaireJoueur();
	
	virtual void afficher() const;
	
private:
	virtual void definirCapacite(size_t c);
	Image _fond;
};

#endif
