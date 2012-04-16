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
	friend EntiteStatique *ElementNiveau::elementNiveau<EntiteStatique>(bool decoupagePerspective, Niveau *n, uindex_t i, ElementNiveau::elementNiveau_t) throw(ElementNiveau::Exc_EntiteIndefinie, ElementNiveau::Exc_DefinitionEntiteIncomplete);
	friend class ElementNiveau;
public:
	virtual ~EntiteStatique();
	
	virtual void afficher(index_t deltaX, index_t deltaY, Coordonnees const &decalage, double zoom = 1.0) const;
	virtual void animer(horloge_t tempsEcoule);

	Image const &image() const;

protected:
	static ElementNiveau::elementNiveau_t cat() { return ElementNiveau::entiteStatique; }
	
	EntiteStatique(bool decoupagePerspective, Niveau *n, uindex_t index, ElementNiveau::elementNiveau_t);
	EntiteStatique(EntiteStatique const &);
	EntiteStatique &operator=(EntiteStatique const &);
		
private:
	Image _image;
	Rectangle *_cadres;
};

#endif
