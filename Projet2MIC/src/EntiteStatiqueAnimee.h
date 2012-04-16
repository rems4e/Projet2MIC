//
//  EntiteStatiqueAnimee.h
//  Projet2MIC
//
//  Created by Rémi Saurel on 11/03/12.
//  Copyright (c) 2012 Rémi Saurel. All rights reserved.
//

#ifndef Projet2MIC_EntiteStatiqueAnimee_h
#define Projet2MIC_EntiteStatiqueAnimee_h

#include "EntiteStatique.h"
#include "Geometrie.h"

class EntiteStatiqueAnimee : public EntiteStatique {
	friend EntiteStatiqueAnimee *ElementNiveau::elementNiveau<EntiteStatiqueAnimee>(bool decoupagePerspective, Niveau *n, uindex_t i, ElementNiveau::elementNiveau_t) throw(ElementNiveau::Exc_EntiteIndefinie, ElementNiveau::Exc_DefinitionEntiteIncomplete);
	friend class ElementNiveau;
public:
	virtual ~EntiteStatiqueAnimee();
	
	virtual void afficher(index_t deltaX, index_t deltaY, Coordonnees const &decalage, double zoom = 1.0) const;
	virtual void animer(horloge_t tempsEcoule);
	
	Rectangle const &cadre() const;

protected:
	static ElementNiveau::elementNiveau_t cat() { return ElementNiveau::entiteStatiqueAnimee; }
	EntiteStatiqueAnimee(bool decoupagePerspective, Niveau *n, uindex_t index, ElementNiveau::elementNiveau_t);

private:
	size_t _nbImages;
	Rectangle *_cadres;
	horloge_t _tempsAffichage;
		
	uindex_t _imageActuelle;
	
	horloge_t _tempsPrecedent;

	
	EntiteStatiqueAnimee(EntiteStatiqueAnimee const &);
	EntiteStatiqueAnimee &operator=(EntiteStatiqueAnimee const &);
};

#endif
