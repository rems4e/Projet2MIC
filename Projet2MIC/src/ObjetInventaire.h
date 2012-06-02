//
//  ObjetInventaire.h
//  Projet2MIC
//
//  Créé par Marc Promé et Rémi Saurel.
//  Ce fichier et son contenu sont librement distribuables, modifiables et utilisables pour toute œuvre non commerciale, à condition d'en citer les auteurs.
//

#ifndef Projet2MIC_ObjetInventaire_h
#define Projet2MIC_ObjetInventaire_h

#include "EntiteStatique.h"
#include "Personnage.h"

class TiXmlElement;

class ObjetInventaire : public EntiteStatique {
	friend ObjetInventaire *ElementNiveau::elementNiveau<ObjetInventaire>(bool decoupagePerspective, Niveau *n, uindex_t i, ElementNiveau::elementNiveau_t) throw(ElementNiveau::Exc_EntiteIndefinie, ElementNiveau::Exc_DefinitionEntiteIncomplete);
	friend class ElementNiveau;
public:
	enum categorie_t {arme, bouclier, armure, casque, gants, bottes, potion, cle};
	
	virtual ~ObjetInventaire();
	
	virtual void afficher(index_t deltaY, Coordonnees const &decalage) const;
	virtual void animer();
	virtual Coordonnees dimensions() const;
	
	categorie_t categorieObjet() const;
	bool tenue() const;
	Coordonnees dimensionsInventaire() const;
	
	int defense() const;
	int attaque() const;
	int vie() const;
	void supprimerVie(int delta);
	
	void definirDefense(int d);
	void definirAttaque(int a);
	void definirVie(int v);
	
	Personnage::Competences const &competencesRequises() const;
	void definirCompetencesRequises(Personnage::Competences const &);
	
	void equilibrerAvecJoueur();
	
	char const *nomCategorieObjet() const;
		
protected:
	static ElementNiveau::elementNiveau_t cat() { return ElementNiveau::objetInventaire; }
	
	ObjetInventaire(bool decoupagePerspective, Niveau *n, uindex_t index, ElementNiveau::elementNiveau_t);
	ObjetInventaire(EntiteStatique const &);
	ObjetInventaire &operator=(ObjetInventaire const &);
	
	Coordonnees _dimInventaire;
	Personnage::Competences _competencesRequises;
	categorie_t _categorie;
	
	int _attaque, _defense, _vie;
};

#endif
