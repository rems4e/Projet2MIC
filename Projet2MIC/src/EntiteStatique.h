//
//  EntiteStatique.h
//  Projet2MIC
//
//  Créé par Marc Promé et Rémi Saurel.
//  Ce fichier et son contenu sont librement distribuables, modifiables et utilisables pour toute œuvre non commerciale, à condition d'en citer les auteurs.
//

#ifndef Projet2MIC_EntiteStatique_h
#define Projet2MIC_EntiteStatique_h

#include "ElementNiveau.h"

class EntiteStatique : public ElementNiveau {
	friend EntiteStatique *ElementNiveau::elementNiveau<EntiteStatique>(bool decoupagePerspective, Niveau *n, uindex_t i, ElementNiveau::elementNiveau_t) throw(ElementNiveau::Exc_EntiteIndefinie, ElementNiveau::Exc_DefinitionEntiteIncomplete);
	friend class ElementNiveau;
public:
	virtual ~EntiteStatique();
	
	virtual void afficher(index_t deltaY, Coordonnees const &decalage) const;
	virtual void animer();

	Image const &image() const;

protected:
	static ElementNiveau::elementNiveau_t cat() { return ElementNiveau::entiteStatique; }
	
	EntiteStatique(bool decoupagePerspective, Niveau *n, uindex_t index, ElementNiveau::elementNiveau_t);
	EntiteStatique(EntiteStatique const &);
	EntiteStatique &operator=(EntiteStatique const &);
		
private:
	Image _image;
	Rectangle *_cadres;
	size_t _nb;
};

#endif
