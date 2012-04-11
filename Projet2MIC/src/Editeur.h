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
#include <set>

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

private:
	struct ElementEditeur {
		ElementEditeur(ElementNiveau::elementNiveau_t cat, index_t index);
		ElementEditeur(index_t loiProba);
		
		uint16_t operator()() const;
		
		Image const &image() const;
		Rectangle cadre() const;
		Coordonnees dimensions() const;
		Coordonnees origine() const;
		Couleur teinte() const;
		
	private:
		bool _proba;
		index_t _indexProba;
		index_t _index;
		
		Image _image;
		Rectangle _cadre;
		Coordonnees _origine;
		Couleur _teinte;
		Coordonnees _dimensions;
		
		ElementNiveau::elementNiveau_t _categorie;
	};
	
	struct Case {
		ElementEditeur *_contenu[Niveau::nbCouches];
		
		Case();
		ElementEditeur * &operator[](Niveau::couche_t c);
	};
	
	typedef std::vector<Case> Colonne;
	typedef std::vector<Colonne> Ligne;
	
	struct LoiProba {
		LoiProba(std::string const &nom, std::string const &loi);
		int &operator[](ElementNiveau::elementNiveau_t c);

		int _proba[ElementNiveau::nbTypesElement];
		std::string _nom;
	};

	struct NiveauEditeur {
		NiveauEditeur(std::string const &fichier);
		~NiveauEditeur();
		
		Ligne _elements;
		std::vector<LoiProba> _probas;
		std::string const _fichier;
		size_t _dimX, _dimY;
	};

	typedef void (Editeur::*fonctionEditeur_t)();
	
	typedef std::list<std::pair<Rectangle, fonctionEditeur_t> > listeFonctions_t;
	enum etat_selection_t {es_ok, es_aj, es_sup};
	struct ElementSelection {
		ElementEditeur const *_e;
		index_t _posX, _posY;
		etat_selection_t _etat;
		ElementSelection(ElementEditeur const *e, index_t posX, index_t posY, etat_selection_t etat) : _e(e), _posX(posX), _posY(posY), _etat(etat) { }
		
		inline bool operator==(ElementSelection const &e) const {
			if(_e == 0 && e._e == 0)
				return _posX == e._posX && _posY == e._posY;
			return _e == e._e; }
	};

	typedef std::list<ElementSelection> selection_t;
	
protected:	
	Editeur();
	
	void modification();
	void enregistrer();
	void afficher();
	void afficherCouche(Niveau::couche_t couche);
	void afficherGrille();
	void afficherInterface();
	void afficherInventaire();
	void afficherControles();
	void afficherCarte();
	
	void sourisInventaire();
	void sourisControles();
	void sourisEditeur();
	void sourisCarte();
	
	Rectangle const &cadreInventaire() const;
	Rectangle const &cadreControles() const;
	Rectangle const &cadreEditeur() const;
	Rectangle const &cadreCarte() const;
	
	void modifCouche();
	void modifProba();
	void modifIndexProba();
	void modifIndex();
	void modifCategorie();
	void modifDimensions();
	
	void modifLoisProbas();
	void editerLoiProba(index_t loi, Image &fond);
	
	void selectionnerSemblables();
		
private:
	bool _continuer;
	bool _modifie;
	Image _sauve;
	
	NiveauEditeur *_niveau;
	Coordonnees _origine;
	Niveau::couche_t _coucheEdition;
	Rectangle const *_ancienRectangle;
	Rectangle _affichageSelection;
	Rectangle _cadreSelection;
	
	static Rectangle _cadreEditeur;
	static Rectangle _cadreControles;
	static Rectangle _cadreInventaire;
	static Rectangle _cadreCarte;

	static void initCadres();
	
	static Editeur *_editeur;
		
	listeFonctions_t _fonctionsControles;
	listeFonctions_t _fonctionsInventaire;
	selection_t _selection;
};

#endif
