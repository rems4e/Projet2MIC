//
//  Teleporteur.h
//  Projet2MIC
//
//  Created by Rémi Saurel on 09/04/12.
//  Copyright (c) 2012 Rémi Saurel. All rights reserved.
//

#ifndef Projet2MIC_Teleporteur_h
#define Projet2MIC_Teleporteur_h

#include "EntiteStatiqueAnimee.h"

class Teleporteur : public EntiteStatiqueAnimee {
	friend Teleporteur *ElementNiveau::elementNiveau<Teleporteur>(bool decoupagePerspective, Niveau *n, uindex_t i, ElementNiveau::elementNiveau_t) throw(ElementNiveau::Exc_EntiteIndefinie, ElementNiveau::Exc_DefinitionEntiteIncomplete);
	friend class ElementNiveau;
public:
	virtual ~Teleporteur();
		
protected:
	static ElementNiveau::elementNiveau_t cat() { return ElementNiveau::teleporteur; }
	Teleporteur(bool decoupagePerspective, Niveau *n, uindex_t index, ElementNiveau::elementNiveau_t);
	
private:
	Coordonnees _destination;
	bool _actif;
	
	Teleporteur(Teleporteur const &);
	Teleporteur &operator=(Teleporteur const &);
};

#endif
