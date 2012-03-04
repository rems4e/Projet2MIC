//
//  ElementNiveau.h
//  Projet2MIC
//
//  Created by Rémi Saurel on 03/02/12.
//  Copyright (c) 2012 Rémi Saurel. All rights reserved.
//

#ifndef Projet2MIC_ElementNiveau_h
#define Projet2MIC_ElementNiveau_h

#include <string>
#include "Constantes.h"
#include "Geometrie.h"
#include "Image.h"
#include "horloge.h"
#include <exception>

class Niveau;
class TiXmlElement;

class ElementNiveau {
public:
	enum elementNiveau_t {premierTypeElement = 0, entiteStatique = premierTypeElement, joueur, ennemi, ndef4, ndef5, ndef6, ndef7, ndef8,
		ndef9, ndef10, ndef11, ndef12, ndef13, ndef14, ndef15, ndef16, ndef17, ndef18, ndef19, ndef20, ndef21, ndef22, ndef23, ndef24,
		ndef25, ndef26, ndef27, ndef28, ndef29, ndef30, ndef31, ndef32, ndef33, ndef34, ndef35, ndef36, ndef37, ndef38, ndef39, ndef40,
		ndef41, ndef42, ndef43, ndef44, ndef45, ndef46, ndef47, ndef48, ndef49, ndef50, ndef51, ndef52, ndef53, ndef54, ndef55, ndef56,
		ndef57, ndef58, ndef59, ndef60, ndef61, ndef62, ndef63, ndef64, nbTypesElement};
	
	// Exception lancée si une entité n'a pu être créée
	class Exc_EntiteIndefinie : public std::exception {
	public:
		Exc_EntiteIndefinie() throw() : std::exception() { }
		virtual ~Exc_EntiteIndefinie() throw() { }
		virtual const char* what() const throw() { return "Création de l'entité impossible !"; }
	};
	
	// Exception lancée si la définition de l'entité dans le .xml est incomplète (manque le fichier image…)
	class Exc_DefinitionEntiteIncomplete : public std::exception {
	public:
		Exc_DefinitionEntiteIncomplete() throw() : std::exception() { }
		virtual ~Exc_DefinitionEntiteIncomplete() throw() { }
		virtual const char* what() const throw() { return "Définition de l'entité incomplète !"; }
	};
	
	// Création d'une entité en fonction du type passe en paramètre template.
	template<class TypeEntite>
	static TypeEntite *elementNiveau(Niveau *n, uindex_t index) throw(Exc_EntiteIndefinie, Exc_DefinitionEntiteIncomplete) {
		if(ElementNiveau::description(index, TypeEntite::categorie())) {
			return new TypeEntite(n, index, TypeEntite::categorie());
		}
		
		throw Exc_EntiteIndefinie();
	}
	
	// Création d'une entité en fonction de sa catégorie
	static ElementNiveau *elementNiveau(Niveau *n, uindex_t index, elementNiveau_t categorie = ElementNiveau::entiteStatique) throw(Exc_EntiteIndefinie, Exc_DefinitionEntiteIncomplete);
	static size_t nombreEntites(elementNiveau_t categorie);
	
	virtual ~ElementNiveau();

	// Affichage de l'entité dans ses caractéristiques actuelles
	virtual void afficher(Coordonnees const &decalage, double zoom = 1.0) const = 0;
	// L'entité entre-t-elle en collision avec les autres ?
	virtual bool collision() const = 0;
	// Évolution de l'état de l'entité au fil du temps
	virtual void animer(horloge_t tempsEcoule) = 0;
	
	// Position de l'entité sur l'écran.
	// Si la fonction 'grille' retourne vrai pour un type d'entité, la position représente les coordonnées de la case du niveau.
	// Si la fonction retourne faux, la position est en pixels.
	Coordonnees position() const;
	void definirPosition(Coordonnees const &p);
	virtual bool grille() const = 0;
	
	// Déplacer la position d'un vecteur donné
	virtual void deplacerPosition(Coordonnees const &dep);
	
	
	// Dimensions de l'entité (de son image affichée).
	virtual Coordonnees dimensions() const = 0;
	// Si l'entité recouvre plusieurs cases ou non.
	bool multi() const;

	
	void definirNiveau(Niveau *n);
	// Position en pixels, après changement de repère depuis celui du niveau vers celui de l'écran.
	Coordonnees positionAffichage() const;
	
	// Origine de l'image en pixels dans le repère du niveau depuis le coin supérieur gauche de l'image.
	virtual Coordonnees origine() const;
	// L'image est-elle centrée sur la cae où elle se trouve ?
	virtual bool centrage() const;

protected:
	Niveau *_niveau;
	Coordonnees _position;
	Coordonnees _origine;
	bool _centrage;
	bool _multi;
	
	bool testerDeplacement(Coordonnees const &dep);
	
	static TiXmlElement *description(uindex_t index, elementNiveau_t cat);
	static char const *nomCategorie(elementNiveau_t cat);
	
	ElementNiveau(Niveau *n, uindex_t index, elementNiveau_t cat);
	ElementNiveau(ElementNiveau const &);
	ElementNiveau &operator=(ElementNiveau const &);
};


#endif
