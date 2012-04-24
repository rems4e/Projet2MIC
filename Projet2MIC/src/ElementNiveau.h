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
class TiXmlDocument;

class ElementNiveau {
public:
	enum elementNiveau_t {premierTypeElement = 0, entiteStatique = premierTypeElement, ennemi, arbre, entiteStatiqueAnimee, objetInventaire, teleporteur, maison, arbreMort,
		marchand, ndef10, ndef11, ndef12, ndef13, ndef14, ndef15, ndef16, ndef17, ndef18, ndef19, ndef20, ndef21, ndef22, ndef23, ndef24,
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
		
	// Création d'une entité en fonction de sa catégorie
	static ElementNiveau *elementNiveau(bool decoupagePerspective, Niveau *n, uindex_t index, elementNiveau_t categorie = ElementNiveau::entiteStatique) throw(Exc_EntiteIndefinie, Exc_DefinitionEntiteIncomplete);
	
	// Création d'une entité en fonction du type passe en paramètre template. Permet d'avoir à se dispenser d'un static_cast<VraiType *>(…).
	// Sans spécifier le 3e paramètre, l'appel de cette fonction est "sûr".
	// En  donnant une valeur à ce 3e paramètre, l'utilisateur doit savoir ce qu'il fait. Dans le cas contraire, la création de l'entité va échouer
	// car elle est créée à partir d'un patron qui ne correspond pas à son type réel.
	// Exemple INCORRECT : elementNiveau<Joueur> créé avec une catégorie à ElementNiveau::arbre. Un joueur n'est en aucun cas un arbre, basé sur EntiteStatique.
	// Exemple CORRECT : elementNiveau<EntiteStatique> créé avec une catégorie à ElementNiveau::arbre. Un arbre est une entité statique et une entité statique peut être un arbre.
	template<class TypeEntite>
	static TypeEntite *elementNiveau(bool decoupagePerspective, Niveau *n, uindex_t index, elementNiveau_t categorie = TypeEntite::cat()) throw(Exc_EntiteIndefinie, Exc_DefinitionEntiteIncomplete) {
		if(ElementNiveau::description(index, categorie)) {
			return new TypeEntite(decoupagePerspective, n, index, categorie);
		}
		
		throw Exc_EntiteIndefinie();
	}

	// Le nombre d'entités d'une catégorie, comme défini dans le document XML de description des entités.
	static size_t nombreEntites(elementNiveau_t categorie);
	
	virtual ~ElementNiveau();

	// Affichage de l'entité dans ses caractéristiques actuelles
	virtual void afficher(index_t deltaY, Coordonnees const &decalage, double zoom = 1.0) const = 0;
	// L'entité entre-t-elle en collision avec les autres, dans sa case (x, y) ?
	virtual bool collision(index_t x, index_t y) const;
	// Évolution de l'état de l'entité au fil du temps
	virtual void animer() = 0;
	
	// Position de l'entité sur l'écran.
	// Si la fonction 'grille' retourne vrai pour un type d'entité, la position représente les coordonnées de la case du niveau.
	// Si la fonction retourne faux, la position est en pixels.
	Coordonnees position() const;
	void definirPosition(Coordonnees const &p);
	virtual index_t pX() const;
	virtual index_t pY() const;
	
	// Dimensions de l'entité en nombre de cases.
	virtual size_t dimX() const;
	virtual size_t dimY() const;
	
	Niveau *niveau();
	void definirNiveau(Niveau *n);
	// Position en pixels, après changement de repère depuis celui du niveau vers celui de l'écran.
	Coordonnees positionAffichage() const;
	
	// Origine de l'image en pixels dans le repère du niveau depuis le coin supérieur gauche de l'image.
	virtual Coordonnees origine() const;
	// L'image est-elle centrée sur la cae où elle se trouve ?
	virtual bool centrage() const;
	bool decoupagePerspective() const;
	
	virtual bool mobile() const;
	virtual bool joueur() const;
	
	elementNiveau_t categorie() const;

	static TiXmlElement *description(uindex_t index, elementNiveau_t cat);
	static char const *nomCategorie(elementNiveau_t cat);

protected:
	ElementNiveau(bool decoupagePerspective, Niveau *n, uindex_t index, elementNiveau_t cat) throw(Exc_DefinitionEntiteIncomplete);
	ElementNiveau(ElementNiveau const &);
	ElementNiveau &operator=(ElementNiveau const &);

	void calcPX();
	void calcPY();
		
private:
	Niveau *_niveau;
	Coordonnees _position;
	Coordonnees _origine;
	bool _centrage;
	elementNiveau_t _categorie;
	
	index_t _pX, _pY;
	size_t _dimX, _dimY;
	bool _decoupagePerspective;
	bool _relief;
	
	bool **_collision;
	
	static TiXmlDocument *_description;
	
	static void chargerDescription();
	static char const *nomBalise(elementNiveau_t cat);
};


ElementNiveau::elementNiveau_t &operator++(ElementNiveau::elementNiveau_t &c);
ElementNiveau::elementNiveau_t operator+(ElementNiveau::elementNiveau_t, int i);

#endif
