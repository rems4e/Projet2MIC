//
//  Joueur.h
//  Projet2MIC
//
//  Créé par Marc Promé et Rémi Saurel.
//  Ce fichier et son contenu sont librement distribuables, modifiables et utilisables pour toute œuvre non commerciale, à condition d'en citer les auteurs.
//


#ifndef Projet2MIC_Joueur_h
#define Projet2MIC_Joueur_h

#include "Personnage.h"
#include "Inventaire.h"

class TiXmlElement;

struct Joueur : public Personnage {
	friend Joueur *ElementNiveau::elementNiveau<Joueur>(bool decoupagePerspective, Niveau *n, uindex_t i, ElementNiveau::elementNiveau_t) throw(ElementNiveau::Exc_EntiteIndefinie, ElementNiveau::Exc_DefinitionEntiteIncomplete);
public:
	enum interactionJoueur_t {ij_aucune, ij_attaquer, ij_commerce, ij_ramasser, ij_finirNiveau};
	
	virtual ~Joueur();
	
	virtual void animer();
	virtual bool interagir(Personnage *p, bool test);
	
	void afficher(index_t deltaY, Coordonnees const &d) const;
	
	virtual bool joueur() const;
	virtual categorie_t categorieMobile() const;
	
	bool inventaireAffiche() const;
	void definirInventaireAffiche(bool af);
	
	bool peutTerminerNiveau() const;
		
	void renaitre();

	TiXmlElement *sauvegarde() const;
	void restaurer(TiXmlElement *sauvegarde);
	
	interactionJoueur_t interaction() const;
	std::pair<size_t, size_t> nombreObjets() const;
	Personnage *attaque() const;
	
	size_t xp() const;
	size_t xpTotale() const;
	index_t niveauXp() const;

	void gagnerXp(Personnage *p);
	bool invincible() const;
	void definirInvicible(bool i);

	void modifierVieActuelle(int delta);
			
protected:
	static ElementNiveau::elementNiveau_t cat() { return ElementNiveau::ennemi; }

	Joueur(bool decoupagePerspective, Niveau *n, uindex_t index, ElementNiveau::elementNiveau_t);
	Joueur(Joueur const &);
	Joueur &operator=(Joueur const &);
	
	virtual void mourir();
	
	virtual void jeterObjets();

private:
	bool _inventaireAffiche;
	
	interactionJoueur_t _interaction;
	std::pair<size_t, size_t> _nombreObjets;
	Personnage *_attaque;
	
	size_t _xp;
	index_t _niveauXp;
	bool _invincible;
};

#endif
