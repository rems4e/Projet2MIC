//
//  horloge.h
//  Projet2MIC
//
//  Créé le 06/02/12.
//

#ifndef Projet2MIC_horloge_h
#define Projet2MIC_horloge_h

typedef float horloge_t;

#ifdef __cplusplus
extern "C" {
#endif

// Retourne le nombre de secondes écoulées depuis le lancement du programme
horloge_t horloge(void);

// Met le thread appelant en pause pour une certaine durée
void attendre(horloge_t sec);
	
#ifdef __cplusplus
}
#endif

#endif
