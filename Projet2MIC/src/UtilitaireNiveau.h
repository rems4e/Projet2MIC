//
//  UtilitaireNiveau.h
//  Projet2MIC
//
//  Created by Rémi Saurel on 11/03/12.
//  Copyright (c) 2012 Rémi Saurel. All rights reserved.
//

#ifndef Projet2MIC_UtilitaireNiveau_h
#define Projet2MIC_UtilitaireNiveau_h

#include <cstdlib>
#include "ElementNiveau.h"
#include "Geometrie.h"

uint16_t entite(ElementNiveau::elementNiveau_t cat, index_t index);
uint16_t entite(index_t loiProbabilite);

void obtenirInfosEntites(uint16_t valeur, bool &proba, index_t &indexProba, ElementNiveau::elementNiveau_t &cat, index_t &index);

Coordonnees referentielNiveauVersEcran(Coordonnees const &pos);
Coordonnees referentielEcranVersNiveau(Coordonnees const &pos);

#endif
