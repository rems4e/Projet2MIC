/*
 *  BoiteDialogue.h
 *  Jeu C++
 *
 *  Created by Rémi on 18/01/11.
 *  Copyright 2011 Rémi Saurel. All rights reserved.
 *
 */

#ifndef BOITE_DIALOGUE_H
#define BOITE_DIALOGUE_H

#include "Unichar.h"

namespace BoiteDialogue {
	Unichar afficher(Unichar const &texte, Unichar const &description, Unichar const &bouton1 = "OK", Unichar const &bouton2 = "Annuler", Unichar const &bouton3 = "");
}

#endif
