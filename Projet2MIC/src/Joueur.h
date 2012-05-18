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

class TiXmlElement;

struct Joueur : public Personnage {
	friend Joueur *ElementNiveau::elementNiveau<Joueur>(bool decoupagePerspective, Niveau *n, uindex_t i, ElementNiveau::elementNiveau_t) throw(ElementNiveau::Exc_EntiteIndefinie, ElementNiveau::Exc_DefinitionEntiteIncomplete);
public:
	virtual ~Joueur();
	
	virtual void animer();
	virtual bool interagir(Personnage *p);
	
	void afficher(index_t deltaY, Coordonnees const &d) const;
	
	virtual bool joueur() const;
	virtual categorie_t categorieMobile() const;
	
	bool inventaireAffiche() const;
	void definirInventaireAffiche(bool af);
	
	void renaitre();
	
	TiXmlElement *sauvegarde() const;
	void restaurer(TiXmlElement *sauvegarde);
			
protected:
	static ElementNiveau::elementNiveau_t cat() { return ElementNiveau::ennemi; }

	Joueur(bool decoupagePerspective, Niveau *n, uindex_t index, ElementNiveau::elementNiveau_t);
	Joueur(Joueur const &);
	Joueur &operator=(Joueur const &);
	
	virtual void mourir();
	
	virtual void jeterObjets();

private:
	bool _inventaireAffiche;
};

#endif
