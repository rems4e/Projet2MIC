//
//  Personnage.h
//  Projet2MIC
//
//  Created by Rémi Saurel on 07/02/12.
//  Copyright (c) 2012 Rémi Saurel. All rights reserved.
//

#ifndef Projet2MIC_Personnage_h
#define Projet2MIC_Personnage_h

#include "EntiteMobile.h"

class Personnage : public EntiteMobile {
public:
	virtual ~Personnage() { }
	
	virtual void animer(horloge_t tempsEcoule);
	virtual void interagir(Personnage *p) = 0;

	virtual bool grille() const;
	virtual Coordonnees origine() const;
	
	// Le coefficient multiplicateur de la vitesse de déplacement de l'entité
	virtual double vitesse() const;

protected:
	Personnage(Niveau *n, uindex_t index, ElementNiveau::elementNiveau_t);
	Personnage(Personnage const &);
	Personnage &operator=(Personnage const &);
	
	double _vitesse;
};

#endif
