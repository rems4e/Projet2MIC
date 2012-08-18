//
//  main.cpp
//  Projet2MIC
//
//  Créé par Marc Promé et Rémi Saurel.
//  Ce fichier et son contenu sont librement distribuables, modifiables et utilisables pour toute œuvre non commerciale, à condition d'en citer les auteurs.
//

#include "Session.h"
#include "jeu.h"
#include "testJeu.h"

#ifdef __cplusplus
extern "C"
#endif


int main(int, char **) {	
	// Initialise tous les sous-programmes requis pour le jeu : affichage (images, textes, écran…), audio, événements, paramètres utilisateur…
	Session::initialiser();
	
	jeu();
	
	// Passé ce point, toutes les ressources doivent être déjà désallouées, car à cause du système de gestion de ressources partagées, leur désallocation après le nettoyage entrainerait un plantage du programme.
	Session::nettoyer();
	
	return 0;
}
