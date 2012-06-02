//
//  EntiteMobile.h
//  Projet2MIC
//
//  Créé par Marc Promé et Rémi Saurel.
//  Ce fichier et son contenu sont librement distribuables, modifiables et utilisables pour toute œuvre non commerciale, à condition d'en citer les auteurs.
//

#ifndef Projet2MIC_EntiteMobile_h
#define Projet2MIC_EntiteMobile_h

#include "ElementNiveau.h"
#include "Image.h"
#include "Geometrie.h"
#include "Niveau.h"
#include <functional>

class EntiteMobile : public ElementNiveau {
public:
	enum action_t {premiereAction, a_immobile = premiereAction, a_deplacer, a_attaquer, a_mourir, nbActions};
	enum direction_t {gauche, gaucheHaut, haut, droiteHaut, droite, droiteBas, bas, gaucheBas};
	enum categorie_t {em_ennemi, em_joueur, em_marchand};
			
	virtual ~EntiteMobile();
	
	virtual void afficher(index_t deltaY, Coordonnees const &decalage) const;
	virtual bool collision(index_t x, index_t y) const;
	virtual void animer();
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
	virtual bool deplacerPosition(Coordonnees const &dep);
		
	virtual Niveau::couche_t couche() const = 0;
	virtual bool mobile() const;
	virtual categorie_t categorieMobile() const = 0;
	bool personnage() const;
	
	index_t nPX(coordonnee_t pX) const;
	index_t nPY(coordonnee_t pY) const;

	bool mort() const;
	virtual void renaitre();

protected:
	Rectangle const &cadre() const;
	bool actionDisponible(action_t a) const;

	EntiteMobile(bool decoupagePerspective, Niveau *n, uindex_t index, ElementNiveau::elementNiveau_t);
	static char const *transcriptionAction(action_t a);
	static bool actionInterruptible(action_t a);
	bool actionInterruptible() const;
	bool testerDeplacement(Coordonnees const &dep);
	
	index_t imageActuelle() const { return _imageActuelle; }
	index_t imageAttaque() const { return _imageAttaque; }
	action_t actionActuelle() const { return _action; }
			
	void mourir();
	
private:
	size_t _nbImages[nbActions];
	Image _image;
	Rectangle *_cadres[nbActions][8];
	horloge_t _tempsAffichage[nbActions];
	
	direction_t _direction;
	index_t _compteurDirection;
	
	action_t _action;
	uindex_t _imageActuelle;
	horloge_t _tempsPrecedent;
	
	bool _mort;
	uindex_t _imageAttaque;
	
	EntiteMobile(EntiteMobile const &);
	EntiteMobile &operator=(EntiteMobile const &);
};

EntiteMobile::action_t &operator++(EntiteMobile::action_t &c);

#endif
