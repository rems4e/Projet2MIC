//
//  EntiteMobile.h
//  Projet2MIC
//
//  Created by Rémi Saurel on 06/02/12.
//  Copyright (c) 2012 Rémi Saurel. All rights reserved.
//

#ifndef Projet2MIC_EntiteMobile_h
#define Projet2MIC_EntiteMobile_h

#include "ElementNiveau.h"
#include "Image.h"
#include "Geometrie.h"

class EntiteMobile : public ElementNiveau {
public:
	enum action_t {premiereAction, a_immobile = premiereAction, a_deplacer, a_attaquer, a_mourir, nbActions};
	enum direction_t {gauche, gaucheHaut, haut, droiteHaut, droite, droiteBas, bas, gaucheBas};
	
	virtual ~EntiteMobile();
	
	virtual void afficher(Coordonnees const &decalage, double zoom = 1.0) const;
	virtual bool collision() const;
	virtual void animer(horloge_t tempsEcoule);
	virtual Coordonnees dimensions() const;
	
	// Direction (gauche, droite…)
	direction_t direction() const;
	void definirDirection(direction_t d);

	action_t action() const;
	// On définit la nouvelle action du personnage, si elle est disponible et que l'action actuelle est interruptible.
	// Elle retourne vrai si l'affectation a réussi, faux si l'action actuelle n'était pas interruptible
	virtual bool definirAction(action_t a);
	
	// Met à jour la direction de l'entité en fonction de la direction du vecteur de déplacement.
	// ATTENTION : utiliser cette fonction va générer une détection des collisions. Pour décaler une entité sans tester les collisions, utiliser entité.definirPosition(entité.position() + dep);
	virtual void deplacerPosition(Coordonnees const &dep);
		
protected:	
	Rectangle const &cadre() const;
	bool actionDisponible(action_t a) const;

	EntiteMobile(Niveau *n, uindex_t index, ElementNiveau::elementNiveau_t);
	static char const *transcriptionAction(action_t a);
	static bool actionInterruptible(action_t a);
	
private:
	size_t _nbImages[nbActions];
	Image _image;
	Rectangle *_cadres[nbActions][8];
	horloge_t _tempsAffichage[nbActions];
	
	direction_t _direction;
	
	action_t _action;
	uindex_t _imageActuelle;
	
	horloge_t _tempsPrecedent;
	
	
	EntiteMobile(EntiteMobile const &);
	EntiteMobile &operator=(EntiteMobile const &);
};

EntiteMobile::action_t &operator++(EntiteMobile::action_t &c);


#endif
