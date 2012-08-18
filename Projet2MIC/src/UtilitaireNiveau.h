//
//  UtilitaireNiveau.h
//  Projet2MIC
//
//  Créé par Marc Promé et Rémi Saurel.
//  Ce fichier et son contenu sont librement distribuables, modifiables et utilisables pour toute œuvre non commerciale, à condition d'en citer les auteurs.
//

#ifndef Projet2MIC_UtilitaireNiveau_h
#define Projet2MIC_UtilitaireNiveau_h

#include <cstdlib>
#include "ElementNiveau.h"
#include "Geometrie.h"

uint16_t entite(ElementNiveau::elementNiveau_t cat, index_t index);
uint16_t entite(index_t loiProbabilite);

void obtenirInfosEntites(uint16_t valeur, bool &proba, index_t &indexProba, ElementNiveau::elementNiveau_t &cat, index_t &index);

glm::vec2 referentielNiveauVersEcran(glm::vec2 const &pos);
glm::vec2 referentielEcranVersNiveau(glm::vec2 const &pos);

#endif
