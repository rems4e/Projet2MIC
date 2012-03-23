//
//  Editeur.h
//  Projet2MIC
//
//  Created by Rémi Saurel on 11/03/12.
//  Copyright (c) 2012 Rémi Saurel. All rights reserved.
//

#ifndef Projet2MIC_Editeur_h
#define Projet2MIC_Editeur_h

#include <string>
#include <vector>
#include <exception>
#include "Niveau.h"
#include "Image.h"
#include "ElementNiveau.h"
#include "Geometrie.h"
#include <list>

class Editeur {
public:
	class Exc_ChargementEditeur : public std::exception {
	public:
		Exc_ChargementEditeur(std::string const &v) throw() : std::exception(), _valeur(v) { }
		virtual ~Exc_ChargementEditeur() throw() { }
		virtual const char* what() const throw() { return _valeur.c_str(); }
	private:
		std::string _valeur;
	};

	static Editeur *editeur();
	void editerNiveau(std::string const &fichier);
	virtual ~Editeur();
	
protected:	
	Editeur();
	
	void enregistrer();
	void afficher();
	void afficherCouche(Niveau::couche_t couche);
	void afficherGrille();
	void afficherInterface();
	void afficherInventaire();
	void afficherControles();
	
	void sourisInventaire();
	void sourisControles();
	void sourisEditeur();
	
	Rectangle const &cadreInventaire() const;
	Rectangle const &cadreControles() const;
	Rectangle const &cadreEditeur() const;
	
	void modifCouche();
	void modifCategorie();
	
private:
	struct ElementEditeur {
		ElementEditeur(index_t index, ElementNiveau::elementNiveau_t cat);
		ElementEditeur(index_t loiProba);
		
		uint16_t operator()() const;
		
		Image const &image() const;
		Rectangle cadre() const;
		Coordonnees origine() const;
		
	private:
		bool _proba;
		index_t _indexProba;
		index_t _index;
		
		Image _image;
		Rectangle _cadre;
		Coordonnees _origine;
		
		ElementNiveau::elementNiveau_t _categorie;
	};
	
	struct Case {
		ElementEditeur *_contenu[Niveau::nb_couches];
		
		Case();
		ElementEditeur * &operator[](Niveau::couche_t c);
	};
	
	typedef std::vector<Case> Colonne;
	typedef std::vector<Colonne> Ligne;
	
	struct NiveauEditeur {
		NiveauEditeur(std::string const &fichier);
		~NiveauEditeur();
		
		Ligne _elements;
		std::string const _fichier;
		size_t _dimX, _dimY;
	};
	
	bool _continuer;
	
	NiveauEditeur *_niveau;
	Coordonnees _origine;
	Niveau::couche_t _coucheEdition;
	ElementNiveau::elementNiveau_t _categorie;
	Rectangle const *_ancienRectangle;
	Rectangle _affichageSelection;
	Rectangle _selection;
	
	static Rectangle _cadreEditeur;
	static Rectangle _cadreControles;
	static Rectangle _cadreInventaire;

	static void initCadres();
	
	static Editeur *_editeur;
	
	typedef void (Editeur::*fonctionEditeur_t)();
	typedef std::list<std::pair<Rectangle, fonctionEditeur_t> > listeFonctions_t;
	
	listeFonctions_t _fonctionsControles;
	listeFonctions_t _fonctionsInventaire;

};

#endif
