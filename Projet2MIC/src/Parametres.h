//
//  Parametres.h
//  Projet2MIC
//
//  Créé par Marc Promé et Rémi Saurel.
//  Ce fichier et son contenu sont librement distribuables, modifiables et utilisables pour toute œuvre non commerciale, à condition d'en citer les auteurs.
//

#ifndef Projet2MIC_Parametres_h
#define Projet2MIC_Parametres_h

#include "Session.h"
#include "Shader.h"
#include "Traducteur.h"
#include <string>

class Image;

namespace Parametres {
	enum action_t {premiereAction, depGauche = premiereAction, depDroite, depHaut, depBas, afficherInventaire, interagir, remplirVie, nbActions};
		
	Session::evenement_t evenementAction(action_t action);
	float volumeMusique();
	float volumeEffets();
	
	std::string const &langue();
	
	int largeurEcran();
	int hauteurEcran();
	bool pleinEcran();
	
	bool ips();
	bool limiteIPS();
	bool synchroVerticale();
	bool rechercherMaj();
	
	void editerParametres(Image const &fond, Shader const &s = Shader::flou(1.0f));
	
	void afficherCredits(Image const &fond, Shader const &s);
	
	char const *versionTexte();
	bool majDisponible();
	char const *URLMaj();
}

inline Parametres::action_t &operator++(Parametres::action_t &e) { return e = static_cast<Parametres::action_t>(static_cast<int>(e) + 1); }

#endif
