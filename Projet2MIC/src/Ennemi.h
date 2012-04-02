//
//  Ennemi.h
//  Projet2MIC
//
//  Created by Rémi Saurel on 02/03/12.
//  Copyright (c) 2012 Rémi Saurel. All rights reserved.
//

#ifndef Projet2MIC_Ennemi_h
#define Projet2MIC_Ennemi_h

#include "Personnage.h"

class Ennemi : public Personnage {
	friend Ennemi *ElementNiveau::elementNiveau<Ennemi>(Niveau *n, uindex_t i, ElementNiveau::elementNiveau_t) throw(ElementNiveau::Exc_EntiteIndefinie, ElementNiveau::Exc_DefinitionEntiteIncomplete);
public:
	
	virtual ~Ennemi();
	
	virtual void animer(horloge_t tempsEcoule);
	virtual void interagir(Personnage *p);
	
	virtual index_t porteeVision() const;
	virtual double vitesse() const;

protected:
	static ElementNiveau::elementNiveau_t categorie() { return ElementNiveau::ennemi; }

	Ennemi(Niveau *n, uindex_t index, ElementNiveau::elementNiveau_t);
	Ennemi(Ennemi const &);
	Ennemi &operator=(Ennemi const &);
	
	Coordonnees _cible;
};

#endif
