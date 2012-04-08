//
//  ObjetInventaire.h
//  Projet2MIC
//
//  Created by Rémi Saurel on 08/04/12.
//  Copyright (c) 2012 Rémi Saurel. All rights reserved.
//

#ifndef Projet2MIC_ObjetInventaire_h
#define Projet2MIC_ObjetInventaire_h


#include "EntiteStatique.h"

class ObjetInventaire : public EntiteStatique {
	friend ObjetInventaire *ElementNiveau::elementNiveau<ObjetInventaire>(Niveau *n, uindex_t i, ElementNiveau::elementNiveau_t) throw(ElementNiveau::Exc_EntiteIndefinie, ElementNiveau::Exc_DefinitionEntiteIncomplete);
	friend class ElementNiveau;
public:
	virtual ~ObjetInventaire();
	
	virtual void afficher(Coordonnees const &decalage, double zoom = 1.0) const;
	virtual void animer(horloge_t tempsEcoule);
	virtual Coordonnees dimensions() const;
	
protected:	
	static ElementNiveau::elementNiveau_t categorie() { return ElementNiveau::objetInventaire; }
	
	ObjetInventaire(Niveau *n, uindex_t index, ElementNiveau::elementNiveau_t);
	ObjetInventaire(EntiteStatique const &);
	ObjetInventaire &operator=(ObjetInventaire const &);
};

#endif
