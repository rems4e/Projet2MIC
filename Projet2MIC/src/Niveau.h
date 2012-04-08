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
#include "Image.h"

class ElementNiveau;
class EntiteStatique;
class EntiteMobile;
class Personnage;
class Joueur;

#define LARGEUR_CASE 64
#define HAUTEUR_CASE 32

#define NB_VALEURS_PROBA_ENTITES 64 * 64
#define BASE_VALEURS_PROBA_ENTITES 64
#define CHIFFRES_VALEURS_PROBA_ENTITES 2

class Niveau {
	friend class Editeur;
public:
	enum couche_t {premiereCouche, cn_sol = premiereCouche, cn_sol2, cn_transitionSol, cn_objet, nbCouches};
	typedef std::list<ElementNiveau *> elements_t;
	typedef std::pair<elements_t::const_iterator, elements_t::const_iterator> listeElements_t;
	
	class Exc_CreationNiveau : public std::exception {
	public:
		Exc_CreationNiveau(std::string const &v) throw() : std::exception(), _valeur(v) { }
		virtual ~Exc_CreationNiveau() throw() { }
		virtual const char* what() const throw() { return _valeur.c_str(); }
	private:
		std::string _valeur;
	};
		
	static ElementNiveau * const aucunElement;

	Niveau(Joueur *j, std::string const &nomFichier);
	Niveau(Joueur *j);
	virtual ~Niveau();
	
	Joueur *joueur();
	
	// Dimensions du niveau en nombre de cases
	size_t dimX() const;
	size_t dimY() const;
	
	// Obtention du contenu d'une case en un point donné. Si ce point est en dehors du niveau, retourne Niveau::aucunElement.
	//ElementNiveau *element(index_t x, index_t y, couche_t couche = cn_sol);
	listeElements_t elements(index_t x, index_t y, couche_t couche) const;
	bool collision(index_t x, index_t y, couche_t couche, ElementNiveau *e) const;
	
	double zoom() const;
	void definirZoom(double z);
	
	void animer(horloge_t tempsEcoule);
	void afficher();
	
	static char const *nomCouche(couche_t couche);
		
protected:
	Niveau(Joueur *p, Niveau const &niveau);
	Niveau &operator=(Niveau const &);
	
	void afficherCouche(couche_t couche, Coordonnees const &cam);
	void afficherBordure(int cote, Coordonnees const &cam);
	
	void definirContenuCase(index_t x, index_t y, couche_t couche, ElementNiveau *e);
	
	void allocationCases();
	void remplissageBordures();
	static ssize_t epaisseurBordure();
	static bool collision(couche_t couche);
	void definirJoueur(Joueur *j);

private:
	struct Case {
		Case();
		
		elements_t _entites[nbCouches];
		bool _entiteExterieure[nbCouches];
	};
	struct CaseMobile {
		EntiteMobile *_e;
		Case *_pos;
	};

	Case **_elements;
	Case **_bordures[4];
	size_t _dimX, _dimY;
	double _zoom;
	std::list<CaseMobile> _entitesMobiles;
	Joueur *_perso;
	Image _b1;
	Image _b2;
};

Niveau::couche_t &operator++(Niveau::couche_t &c);
Niveau::couche_t operator+(Niveau::couche_t, int i);


#endif
