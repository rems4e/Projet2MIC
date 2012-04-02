//
//  Joueur.h
//  Projet2MIC
//
//  Created by Rémi Saurel on 11/02/12.
//  Copyright (c) 2012 Rémi Saurel. All rights reserved.
//

#ifndef Projet2MIC_Joueur_h
#define Projet2MIC_Joueur_h

#include "Personnage.h"

struct Joueur : public Personnage {
	friend Joueur *ElementNiveau::elementNiveau<Joueur>(Niveau *n, uindex_t i, ElementNiveau::elementNiveau_t) throw(ElementNiveau::Exc_EntiteIndefinie, ElementNiveau::Exc_DefinitionEntiteIncomplete);
public:
	virtual ~Joueur();
	
	virtual void animer(horloge_t tempsEcoule);
	virtual void interagir(Personnage *p);
	
	void afficher(Coordonnees const &d, double zoom = 1.0) const;
	
	virtual bool joueur() const;
	
protected:
	static ElementNiveau::elementNiveau_t categorie() { return ElementNiveau::ennemi; }

	Joueur(Niveau *n, uindex_t index, ElementNiveau::elementNiveau_t);
	Joueur(Joueur const &);
	Joueur &operator=(Joueur const &);
};

#endif
