//
//  Ennemi.h
//  Projet2MIC
//
//  Créé par Marc Promé et Rémi Saurel.
//  Ce fichier et son contenu sont librement distribuables, modifiables et utilisables pour toute œuvre non commerciale, à condition d'en citer les auteurs.
//

#ifndef Projet2MIC_Ennemi_h
#define Projet2MIC_Ennemi_h

#include "Personnage.h"

class Ennemi : public Personnage {
	friend Ennemi *ElementNiveau::elementNiveau<Ennemi>(bool decoupagePerspective, Niveau *n, uindex_t i, ElementNiveau::elementNiveau_t) throw(ElementNiveau::Exc_EntiteIndefinie, ElementNiveau::Exc_DefinitionEntiteIncomplete);
public:
	
	virtual ~Ennemi();
	
	virtual void animer();
	virtual bool interagir(Personnage *p, bool test);
	
	virtual index_t porteeVision() const;
	virtual double vitesse() const;
	virtual categorie_t categorieMobile() const;

	void equilibrerAvecJoueur();

protected:
	static ElementNiveau::elementNiveau_t cat() { return ElementNiveau::ennemi; }

	Ennemi(bool decoupagePerspective, Niveau *n, uindex_t index, ElementNiveau::elementNiveau_t);
	Ennemi(Ennemi const &);
	Ennemi &operator=(Ennemi const &);

	virtual void jeterObjets();
	void mourir();

private:
	glm::vec2 _cible;
	bool _recherche;
	
	double _coef[Personnage::nbCompetences];
};

#endif
