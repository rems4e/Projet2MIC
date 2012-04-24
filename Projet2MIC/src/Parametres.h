//
//  Parametres.h
//  Projet2MIC
//
//  Created by Rémi Saurel on 13/04/12.
//  Copyright (c) 2012 Rémi Saurel. All rights reserved.
//

#ifndef Projet2MIC_Parametres_h
#define Projet2MIC_Parametres_h

#include "Session.h"

class Image;

namespace Parametres {
	enum action_t {premiereAction, depGauche = premiereAction, depDroite, depHaut, depBas, afficherJournal, afficherInventaire, ramasserObjet, interagir, sort1, sort2, sort3, sort4, nbActions};
	Session::evenement_t evenementAction(action_t action);
	float volumeMusique();
	float volumeEffets();
	
	void editerParametres(Image const &fond);
}

inline Parametres::action_t &operator++(Parametres::action_t &e) { return e = static_cast<Parametres::action_t>(static_cast<int>(e) + 1); }

#endif