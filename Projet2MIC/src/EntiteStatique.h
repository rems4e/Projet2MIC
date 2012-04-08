//
//  EntiteStatique.h
//  Projet2MIC
//
//  Created by Rémi Saurel on 06/02/12.
//  Copyright (c) 2012 Rémi Saurel. All rights reserved.
//

#ifndef Projet2MIC_EntiteStatique_h
#define Projet2MIC_EntiteStatique_h

#include "ElementNiveau.h"

class EntiteStatique : public ElementNiveau {
	friend EntiteStatique *ElementNiveau::elementNiveau<EntiteStatique>(Niveau *n, uindex_t i, ElementNiveau::elementNiveau_t) throw(ElementNiveau::Exc_EntiteIndefinie, ElementNiveau::Exc_DefinitionEntiteIncomplete);
	friend class ElementNiveau;
public:
	virtual ~EntiteStatique();
	
	virtual void afficher(Coordonnees const &decalage, double zoom = 1.0) const;
	virtual bool collision() const;
	virtual void animer(horloge_t tempsEcoule);
	virtual Coordonnees dimensions() const;
	
protected:
	static ElementNiveau::elementNiveau_t categorie() { return ElementNiveau::entiteStatique; }
	
	EntiteStatique(Niveau *n, uindex_t index, ElementNiveau::elementNiveau_t);
	EntiteStatique(EntiteStatique const &);
	EntiteStatique &operator=(EntiteStatique const &);
	
	Image const &image() const;
	
private:
	Image _image;
};

#endif
