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
#include "Personnage.h"

class ObjetInventaire : public EntiteStatique {
	friend ObjetInventaire *ElementNiveau::elementNiveau<ObjetInventaire>(bool decoupagePerspective, Niveau *n, uindex_t i, ElementNiveau::elementNiveau_t) throw(ElementNiveau::Exc_EntiteIndefinie, ElementNiveau::Exc_DefinitionEntiteIncomplete);
	friend class ElementNiveau;
public:
	enum categorie_t {arme, bouclier, armure, casque, gants, bottes, potion};
	
	virtual ~ObjetInventaire();
	
	virtual void afficher(index_t deltaX, index_t deltaY, Coordonnees const &decalage, double zoom = 1.0) const;
	virtual void animer(horloge_t tempsEcoule);
	virtual Coordonnees dimensions() const;
	
	categorie_t categorieObjet() const;
	bool tenue() const;
	Coordonnees dimensionsInventaire() const;
	Personnage::Competences const &competencesRequises() const;
		
protected:
	static ElementNiveau::elementNiveau_t cat() { return ElementNiveau::objetInventaire; }
	
	ObjetInventaire(bool decoupagePerspective, Niveau *n, uindex_t index, ElementNiveau::elementNiveau_t);
	ObjetInventaire(EntiteStatique const &);
	ObjetInventaire &operator=(ObjetInventaire const &);
	
	Coordonnees _dimInventaire;
	Personnage::Competences _competencesRequises;
	categorie_t _categorie;
};

#endif
