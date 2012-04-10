//
//  Inventaire.h
//  Projet2MIC
//
//  Created by Rémi Saurel on 08/04/12.
//  Copyright (c) 2012 Rémi Saurel. All rights reserved.
//

#ifndef Projet2MIC_Inventaire_h
#define Projet2MIC_Inventaire_h

#define CAPACITE_ENNEMI 4
#define LARGEUR_MARCHAND 10
#define HAUTEUR_MARCHAND 4
#define LARGEUR_JOUEUR 10
#define HAUTEUR_JOUEUR 4

#include "ObjetInventaire.h"
#include "Personnage.h"

#include <list>
#include <vector>

class Inventaire {
protected:
	class iterator {
	public:
		virtual ~iterator();
		
		virtual iterator &operator=(iterator const &) = 0;
		virtual iterator &operator++() = 0;
		virtual iterator &operator--() = 0;
		
		virtual bool operator==(iterator const &) const = 0;
		virtual bool operator!=(iterator const &) const = 0;
		
		virtual ObjetInventaire *&operator*() const = 0;
		virtual ObjetInventaire **operator->() const = 0;
	};
	
public:
	class const_iterator {
	public:
		virtual ~const_iterator();

		virtual const_iterator &operator=(const_iterator const &) = 0;
		virtual const_iterator &operator=(iterator const &) = 0;
		virtual const_iterator &operator++() = 0;
		virtual const_iterator &operator--() = 0;
		
		virtual bool operator==(iterator const &) const = 0;
		virtual bool operator!=(iterator const &) const = 0;

		virtual ObjetInventaire const * operator*() const = 0;
		virtual ObjetInventaire const **operator->() const = 0;
	};
public:
	Inventaire(Personnage &perso);
	virtual ~Inventaire();
	
	virtual size_t capacite() const = 0;
	virtual void definirCapacite(size_t c) = 0;
	
	virtual size_t nombreObjets() const = 0;
	
	virtual bool ajouterObjet(ObjetInventaire *o) = 0;
	virtual void supprimerObjet(ObjetInventaire *o) = 0;
	virtual void vider() = 0;
	
	virtual void afficher() const = 0;
	virtual void gestionEvenements() = 0;
	
	const_iterator const &debut() const;
	const_iterator const &fin() const;
	
protected:
	iterator const &debut();
	iterator const &fin();
	
	Personnage const &personnage() const;
	Personnage &personnage();
	
private:
	Personnage &_perso;
};

template<template <class e, class = std::allocator<e> > class Conteneur>
class InventaireC : public Inventaire {
protected:
	class iterator {
	public:
		iterator() : _base() { }
		iterator(iterator const &i) : _base(i._base) { }
		virtual ~iterator() { }
		
		virtual iterator &operator=(iterator const &i) { _base = i._base; return *this; }
		virtual iterator &operator++() { ++_base; return *this; }
		virtual iterator &operator--() { --_base; return *this; }
		
		virtual bool operator==(iterator const &i) const { return _base == i._base; }
		virtual bool operator!=(iterator const &i) const { return _base != i._base; };
		
		virtual ObjetInventaire *&operator*() const { return *_base; }
		virtual ObjetInventaire **operator->() const { return &*_base; }

		iterator(typename Conteneur<ObjetInventaire *>::iterator const &i) : _base(i) { }
		typename Conteneur<ObjetInventaire *>::iterator _base;
	};
	
public:
	class const_iterator {
	public:
		const_iterator() : _base() { }
		const_iterator(const_iterator const &i) : _base(i._base) { }
		const_iterator(iterator const &i) : _base(i._base) { }
		virtual ~const_iterator() { }
		
		virtual const_iterator &operator=(const_iterator const &i) { _base = i._base; return *this; }
		virtual const_iterator &operator=(iterator const &i) { _base = i._base; return *this; }
		virtual const_iterator &operator++() { ++_base; return *this; }
		virtual const_iterator &operator--() { --_base; return *this; }
		
		virtual bool operator==(const_iterator const &i) const { return _base == i._base; }
		virtual bool operator!=(const_iterator const &i) const { return _base != i._base; }
		virtual bool operator==(iterator const &i) const { return _base == i._base; }
		virtual bool operator!=(iterator const &i) const { return _base != i._base; }
		
		virtual ObjetInventaire const * operator*() const { return *_base; }
		virtual ObjetInventaire const * const *operator->() const { return &*_base; }

		const_iterator(typename Conteneur<ObjetInventaire *>::const_iterator const &i) : _base(i) { }
		const_iterator(typename Conteneur<ObjetInventaire *>::iterator const &i) : _base(i) { }
		typename Conteneur<ObjetInventaire *>::const_iterator _base;
	};
	
	template<typename InputIterator>
	InventaireC(Personnage &perso, size_t capacite, InputIterator debut, InputIterator fin);
	InventaireC(Personnage &perso, size_t capacite);
	virtual ~InventaireC();
	
	size_t capacite() const;
	void definirCapacite(size_t c);
	
	virtual size_t nombreObjets() const;
	
	virtual bool ajouterObjet(ObjetInventaire *o);
	virtual void supprimerObjet(ObjetInventaire *o);
	virtual void vider();
	
	const_iterator debut() const;
	const_iterator fin() const;
		
protected:
	iterator debut();
	iterator fin();

private:
	Conteneur<ObjetInventaire *> _elements;
	size_t _capacite;
};

class InventaireListe : public InventaireC<std::list> {
public:
	typedef InventaireC<std::list>::const_iterator const_iterator;
	typedef InventaireC<std::list>::iterator iterator;
	
	template<typename InputIterator>
	InventaireListe(Personnage &perso, size_t capacite, InputIterator debut, InputIterator fin);
	InventaireListe(Personnage &perso, size_t capacite);
	virtual ~InventaireListe();
			
private:
	virtual void afficher() const;
	virtual void gestionEvenements();
};

typedef InventaireListe InventaireEnnemi;

class InventaireSol : public InventaireListe {
public:
	typedef InventaireListe::const_iterator const_iterator;
	typedef InventaireListe::iterator iterator;
	
	template<typename InputIterator>
	InventaireSol(Personnage &perso, InputIterator debut, InputIterator fin);
	InventaireSol(Personnage &perso);
	~InventaireSol();

	virtual void afficher() const;
	virtual void gestionEvenements();
	
private:
	virtual void definirCapacite(size_t c);
};

class InventaireTableau : public InventaireC<std::vector> {
public:
	typedef InventaireC<std::vector>::const_iterator const_iterator;
	typedef InventaireC<std::vector>::iterator iterator;
	
	template<typename InputIterator>
	InventaireTableau(Personnage &perso, size_t largeur, size_t hauteur, InputIterator debut, InputIterator fin);
	InventaireTableau(Personnage &perso, size_t largeur, size_t hauteur);
	virtual ~InventaireTableau();
	
	bool ajouterObjetEnPosition(ObjetInventaire *o, index_t position);
	virtual bool ajouterObjet(ObjetInventaire *o);
	virtual void supprimerObjet(ObjetInventaire *o);
	virtual size_t nombreObjets() const;

	virtual void vider();
	
protected:
	ObjetInventaire *objetDansCase(index_t position);
	ObjetInventaire *objetDansCase(index_t pX, index_t pY);
	bool peutPlacerObjetDansCase(index_t pX, index_t pY, ObjetInventaire *objet, ObjetInventaire *&objetARemplacer);
	Coordonnees positionObjet(ObjetInventaire *o) const;

	index_t largeur() const;
	index_t hauteur() const;

private:
	virtual void afficher() const;

	size_t _largeur;
	size_t _hauteur;
};

/*class InventaireMarchand : public InventaireTableau {
public:
	typedef InventaireTableau::const_iterator const_iterator;
	typedef InventaireTableau::iterator iterator;
	
	InventaireMarchand();
	virtual ~InventaireMarchand();
		
	virtual void afficher() const;
	virtual void gestionEvenements();

private:
	virtual void definirCapacite(size_t c);
};*/

class InventaireJoueur : public InventaireTableau {
public:
	typedef InventaireTableau::const_iterator const_iterator;
	typedef InventaireTableau::iterator iterator;
	
	InventaireJoueur(Joueur &joueur);
	virtual ~InventaireJoueur();
	
	virtual void afficher() const;
	virtual void gestionEvenements();
	
private:
	virtual void definirCapacite(size_t c);
	Image _fond;
	Rectangle _inventaire;
	Rectangle _tenue[Personnage::nbPositionsTenue];
	Rectangle _surlignage;
	Couleur _couleurSurlignage;
	
	Rectangle _surlignageTransfert;
	Couleur _couleurSurlignageTransfert;
	
	ObjetInventaire *_objetTransfert;
};

#endif
