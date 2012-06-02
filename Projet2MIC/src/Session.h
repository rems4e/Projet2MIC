//
//  Session.h
//  Projet2MIC
//
//  Créé par Marc Promé et Rémi Saurel.
//  Ce fichier et son contenu sont librement distribuables, modifiables et utilisables pour toute œuvre non commerciale, à condition d'en citer les auteurs.
//

#ifndef EN_TETE_SESSION
#define EN_TETE_SESSION

#include "Constantes.h"
#include "Geometrie.h"
#include <string>
#include "horloge.h"
#include <list>
#include "Unichar.h"

namespace Session {
	class horloge_locale_t;
	
	// Le chemin vers le répertoire où sont contenues les ressources (image, son, xml…) du programme.
	// Il est obligatoire pour charger une ressource d'utiliser cette fonction : 'std::string chemin = Session::cheminRessources + "monFichier.xml";'
	std::string const &cheminRessources();

	enum evenement_t {
		premierEvenement,
		SOURIS = premierEvenement, B_GAUCHE, B_DROIT,
		PREMIER_EVENEMENT_CLAVIER,
		/* Caractères imprimables */
		PREMIER_EVENEMENT_IMPRIMABLE = PREMIER_EVENEMENT_CLAVIER,
		
		/* Lettres, chiffres */
		T_a = PREMIER_EVENEMENT_IMPRIMABLE, T_b, T_c, T_d, T_e, T_f, T_g, T_h, T_i, T_j, T_k, T_l, T_m, T_n, T_o, T_p, T_q, T_r, T_s, T_t, T_u, T_v, T_w, T_x, T_y, T_z,
		T_0, T_1, T_2, T_3, T_4, T_5, T_6, T_7, T_8, T_9,
		
		/* Opérations, comparaisons */
		T_MOINS, T_PLUS, T_SLASH, T_ASTERISQUE, T_EGAL, T_INFERIEUR, T_SUPERIEUR,
		
		/* Ponctuation */
		T_ESPACE, T_TAB, T_TIRET_BAS, T_POINT, T_VIRGULE, T_DEUX_POINTS, T_POINT_VIRGULE, T_POINT_EXCLAMATION, T_POINT_INTERROGATION,
		T_ESPERLUETTE, T_GUILLEMETS, T_APOSTROPHE,
		T_DOLLAR, T_HASH, T_BACKSLASH,
		
		/* Parenthèses */
		T_PARENTHESE_G, T_PARENTHESE_D, T_CROCHET_G, T_CROCHET_D,
		DERNIER_EVENEMENT_IMPRIMABLE = T_CROCHET_D, 
		
		
		T_GAUCHE, T_DROITE, T_HAUT, T_BAS,
		T_ESC, T_EFFACER, T_ENTREE, 
		DERNIER_EVENEMENT_CLAVIER = T_ENTREE,
		
		QUITTER,
		
		nombreEvenements, aucunEvenement = nombreEvenements
	};
	
	enum modificateur_touche_t {
		M_AUCUN,
		M_MAJ = 1 << 0,
		M_COMMANDE = 1 << 1,
		M_CONTROLE = 1 << 2,
		M_OPTION = 1 << 3
	};
	
	inline modificateur_touche_t operator|(modificateur_touche_t const &m1, modificateur_touche_t const &m2) { return static_cast<modificateur_touche_t>(static_cast<int>(m1) | static_cast<int>(m2)); }
	inline modificateur_touche_t &operator|=(modificateur_touche_t &m1, modificateur_touche_t const &m2) { return m1 = m1 | m2; }

	bool modificateurTouches(modificateur_touche_t const &mod);
	modificateur_touche_t const &modificateurTouches();
	
	// Retourne si l'événement demandé est actif
	bool evenement(evenement_t const &e);
	// Position souris
	Coordonnees const &souris();

	// Définit l'événément à faux, même si la touche est encore enfoncée. L'utilisateur devra la relâcher puis la re-enfoncer pour réactiver l'événement.
	void reinitialiser(evenement_t const &e);
	// Fait de même pour TOUS les événements
	void reinitialiserEvenements();

	void definirQuitter(bool q);

	// À placer dans une boucle while, pour que cette boucle ce poursuive tant que 'continuer' est vrai. Cela est requis car c'est dans cette fonction
	// que la gestion des événements est effectuée, elle doit donc être appelée à intervalle régulier.
	// L'intervalle de temps entre 2 itérations de la boucle est défini par 1 / 'freq'.
	bool boucle(horloge_t const freq, bool continuer);
	
	// À n'appeller qu'une seule fois en début de programme
	void initialiser();
	// À n'appeller qu'une seule fois en fin de programme
	void nettoyer();
	
	// Retourne une chaîne de caractères associée à l'événement ou au modificateur demandé
	Unichar transcriptionEvenement(evenement_t const &);
	Unichar transcriptionModificateur(modificateur_touche_t const &);
}

inline Session::evenement_t &operator++(Session::evenement_t &e) { return e = static_cast<Session::evenement_t>(static_cast<int>(e) + 1); }

#include "Parametres.h"

#endif
