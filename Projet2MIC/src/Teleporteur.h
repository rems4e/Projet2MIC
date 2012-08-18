//
//  Teleporteur.h
//  Projet2MIC
//
//  Créé par Marc Promé et Rémi Saurel.
//  Ce fichier et son contenu sont librement distribuables, modifiables et utilisables pour toute œuvre non commerciale, à condition d'en citer les auteurs.
//

#ifndef Projet2MIC_Teleporteur_h
#define Projet2MIC_Teleporteur_h

#include "EntiteStatiqueAnimee.h"

class Teleporteur : public EntiteStatiqueAnimee {
	friend Teleporteur *ElementNiveau::elementNiveau<Teleporteur>(bool decoupagePerspective, Niveau *n, uindex_t i, ElementNiveau::elementNiveau_t) throw(ElementNiveau::Exc_EntiteIndefinie, ElementNiveau::Exc_DefinitionEntiteIncomplete);
	friend class ElementNiveau;
public:
	virtual ~Teleporteur();
		
protected:
	static ElementNiveau::elementNiveau_t cat() { return ElementNiveau::teleporteur; }
	Teleporteur(bool decoupagePerspective, Niveau *n, uindex_t index, ElementNiveau::elementNiveau_t);
	
private:
	glm::vec2 _destination;
	bool _actif;
	
	Teleporteur(Teleporteur const &);
	Teleporteur &operator=(Teleporteur const &);
};

#endif
