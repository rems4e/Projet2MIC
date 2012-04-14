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
#include "Inventaire.h"

class Partie;

struct Joueur : public Personnage {
	friend Joueur *ElementNiveau::elementNiveau<Joueur>(Niveau *n, uindex_t i, ElementNiveau::elementNiveau_t) throw(ElementNiveau::Exc_EntiteIndefinie, ElementNiveau::Exc_DefinitionEntiteIncomplete);
public:
	virtual ~Joueur();
	
	virtual void animer(horloge_t tempsEcoule);
	virtual void interagir(Personnage *p);
	
	void afficher(index_t deltaX, index_t deltaY, Coordonnees const &d, double zoom = 1.0) const;
	
	virtual bool joueur() const;
	virtual categorie_t type() const;
	
	bool inventaireAffiche() const;
	void definirInventaireAffiche(bool af);
			
protected:
	static ElementNiveau::elementNiveau_t cat() { return ElementNiveau::ennemi; }

	Joueur(Niveau *n, uindex_t index, ElementNiveau::elementNiveau_t);
	Joueur(Joueur const &);
	Joueur &operator=(Joueur const &);
	
private:
	bool _inventaireAffiche;
};

#endif
