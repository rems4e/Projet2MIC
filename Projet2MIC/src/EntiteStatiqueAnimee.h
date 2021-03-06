//
//  EntiteStatiqueAnimee.h
//  Projet2MIC
//
//  Créé par Marc Promé et Rémi Saurel.
//  Ce fichier et son contenu sont librement distribuables, modifiables et utilisables pour toute œuvre non commerciale, à condition d'en citer les auteurs.
//

#ifndef Projet2MIC_EntiteStatiqueAnimee_h
#define Projet2MIC_EntiteStatiqueAnimee_h

#include "EntiteStatique.h"
#include "Geometrie.h"

class EntiteStatiqueAnimee : public EntiteStatique {
	friend EntiteStatiqueAnimee *ElementNiveau::elementNiveau<EntiteStatiqueAnimee>(bool decoupagePerspective, Niveau *n, uindex_t i, ElementNiveau::elementNiveau_t) throw(ElementNiveau::Exc_EntiteIndefinie, ElementNiveau::Exc_DefinitionEntiteIncomplete);
	friend class ElementNiveau;
public:
	virtual ~EntiteStatiqueAnimee();
	
	virtual void afficher(index_t deltaY, glm::vec2 const &decalage) const;
	virtual void animer();
	
	Rectangle const &cadre() const;

protected:
	static ElementNiveau::elementNiveau_t cat() { return ElementNiveau::entiteStatiqueAnimee; }
	EntiteStatiqueAnimee(bool decoupagePerspective, Niveau *n, uindex_t index, ElementNiveau::elementNiveau_t);

private:
	size_t _nbImages;
	Rectangle *_cadres;
	horloge_t _tempsAffichage;
		
	uindex_t _imageActuelle;
	
	horloge_t _tempsPrecedent;

	
	EntiteStatiqueAnimee(EntiteStatiqueAnimee const &);
	EntiteStatiqueAnimee &operator=(EntiteStatiqueAnimee const &);
};

#endif
