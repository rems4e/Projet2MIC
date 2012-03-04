//
//  Niveau.h
//  Projet2MIC
//
//  Created by Rémi Saurel on 03/02/12.
//  Copyright (c) 2012 Rémi Saurel. All rights reserved.
//

#ifndef Projet2MIC_Niveau_h
#define Projet2MIC_Niveau_h

#include "Geometrie.h"
#include "Constantes.h"
#include <list>
#include "horloge.h"
#include <exception>

class ElementNiveau;
class EntiteStatique;
class EntiteMobile;
class Personnage;
class Joueur;

#define LARGEUR_CASE 64
#define HAUTEUR_CASE 32

class Niveau {
public:
	enum couche_t {premierCouche, cn_sol = premierCouche, cn_sol2, cn_objet, nb_couches};

	class Exc_CreationNiveau : public std::exception {
	public:
		Exc_CreationNiveau(std::string const &v) throw() : std::exception(), _valeur(v) { }
		virtual ~Exc_CreationNiveau() throw() { }
		virtual const char* what() const throw() { return _valeur.c_str(); }
	private:
		std::string _valeur;
	};
	
	struct Case {
		Case();
		
		ElementNiveau *_entites[nb_couches];
		bool _entiteExterieure[nb_couches];
	};
	
	static ElementNiveau * const aucunElement;

	Niveau(Joueur *j, std::string const &nomFichier);
	Niveau(Joueur *j);
	virtual ~Niveau();
	
	// Dimensions du niveau en nombre de cases
	size_t dimX() const;
	size_t dimY() const;
	
	// Obtention du contenu d'une case en un point donné. Si ce point est en dehors du niveau, retourne Niveau::aucunElement.
	//ElementNiveau *element(index_t x, index_t y, couche_t couche = cn_sol);
	ElementNiveau const *element(index_t x, index_t y, couche_t couche = cn_sol) const;
	bool collision(index_t x, index_t y) const;
	
	double zoom() const;
	void definirZoom(double z);
	
	void animer(horloge_t tempsEcoule);
	void afficher();
	
	void definirJoueur(Joueur *j);
		
protected:
	Case **_elements;
	Case **_bordures[4];
	size_t _dimX, _dimY;
	double _zoom;
	std::list<EntiteMobile *> _entitesMobiles;
	Joueur *_perso;
	
	Niveau(Joueur *p, Niveau const &niveau);
	Niveau &operator=(Niveau const &);
	
	void afficherCouche(couche_t couche, Coordonnees const &cam, index_t pX, index_t pY);
	void afficherBordure(int cote, Coordonnees const &cam);
	
	void definirContenuCase(index_t x, index_t y, couche_t couche, ElementNiveau *e);
	
	void init();
	void remplissageBordures();
	static ssize_t epaisseurBordure();
};

Niveau::couche_t &operator++(Niveau::couche_t &c);
Niveau::couche_t operator+(Niveau::couche_t, int i);


#endif
