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
	
	virtual Coordonnees origine() const;
	virtual bool centrage() const;
	
	// Le coefficient multiplicateur de la vitesse de déplacement de l'entité
	virtual double vitesse() const;
	unsigned int vieActuelle() const;
	//non spécifié yet
	virtual unsigned int vieTotale() const;
	//non spécifié yet 
	void modifierVieActuelle(int delta);     
	virtual bool definirAction(action_t a);
	virtual Niveau::couche_t couche() const;

protected:
	Personnage(Niveau *n, uindex_t index, ElementNiveau::elementNiveau_t);
	Personnage(Personnage const &);
	Personnage &operator=(Personnage const &);
	
private:
	unsigned int _vieActuelle;
	unsigned int _vieTotale;
	double _vitesse;
	struct DelaisAction {
		horloge_t _cooldown;
		horloge_t _cdAbsolu;
	};
	DelaisAction _delaisAction[nbActions];
};

#endif
