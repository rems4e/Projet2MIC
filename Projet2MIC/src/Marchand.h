//
//  Marchand.h
//  Projet2MIC
//
//  Créé par Marc Promé et Rémi Saurel.
//  Ce fichier et son contenu sont librement distribuables, modifiables et utilisables pour toute œuvre non commerciale, à condition d'en citer les auteurs.
//


#ifndef Projet2MIC_Marchand_h
#define Projet2MIC_Marchand_h

#include "Personnage.h"
#include "Inventaire.h"

class Marchand : public Personnage {
	friend Marchand *ElementNiveau::elementNiveau<Marchand>(bool decoupagePerspective, Niveau *n, uindex_t i, ElementNiveau::elementNiveau_t) throw(ElementNiveau::Exc_EntiteIndefinie, ElementNiveau::Exc_DefinitionEntiteIncomplete);
public:
	
	virtual ~Marchand();
	
	virtual bool collision(index_t x, index_t y) const;

	virtual void animer();
	virtual bool interagir(Personnage *p, bool test);
	
	virtual categorie_t categorieMobile() const;
	
	// Combien le marchand va acheter l'objet
	ssize_t prixAchat(ObjetInventaire *o);
	// Combien le marchand va vendre l'objet au joueur
	ssize_t prixVente(ObjetInventaire *o);
	
protected:
	static ElementNiveau::elementNiveau_t cat() { return ElementNiveau::marchand; }
	
	Marchand(bool decoupagePerspective, Niveau *n, uindex_t index, ElementNiveau::elementNiveau_t);
	Marchand(Marchand const &);
	Marchand &operator=(Marchand const &);

	virtual void jeterObjets();
	
private:

};

#endif
