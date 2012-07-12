//
//  Audio.cpp
//  Projet2MIC
//
//  Créé par Marc Promé et Rémi Saurel.
//  Ce fichier et son contenu sont librement distribuables, modifiables et utilisables pour toute œuvre non commerciale, à condition d'en citer les auteurs.
//

#include "Constantes.h"
#ifdef __WIN32__

// Pour éviter la pollution des symboles (notamment Rectangle)
namespace WinWin {
#include <windows.h>
}
#endif

#include "Audio.h"
#include "fmod/fmod.h"
#include "Parametres.h"
#include <iostream>
#include "fmod/fmod_errors.h"
#include "horloge.h"
#include "fonctions.h"

#define NB_CANAUX 32

static FMOD_RESULT resultat = FMOD_OK;

namespace Audio {
	void initialiser();
	void nettoyer();
	
	// Affiche l'erreur rencontrée par l'API. La variable nb est là pour voir de quelle fonction provient l'erreur... Pas très joli, mais efficace :p
	bool erreur(FMOD_RESULT code, int nb) throw(Exc_Son);
	
	FMOD_SYSTEM *_systeme = 0;
	FMOD_CHANNEL *_canaux[NB_CANAUX];
	FMOD_CHANNEL *_canalMusique = 0;
	
	audio_t _musique = 0;

	F_CALLBACK FMOD_RESULT sonStoppe(FMOD_CHANNEL *channel, FMOD_CHANNEL_CALLBACKTYPE type, void *commanddata1, void *commanddata2);

}

F_CALLBACK FMOD_RESULT Audio::sonStoppe(FMOD_CHANNEL *channel, FMOD_CHANNEL_CALLBACKTYPE type, void *commanddata1, void *commanddata2) {
	int nb;
	FMOD_Channel_GetIndex(channel, &nb);
	
	FMOD_CHANNEL *c;
	FMOD_System_GetChannel(_systeme, nb, &c);
	
	if(c != _canalMusique) {
		Audio::_canaux[nb] = 0;
	}
			
	return FMOD_OK;
}

bool Audio::erreur(FMOD_RESULT code, int nb) throw(Exc_Son) {
	if(code != FMOD_OK) {
		std::string v = "Problème FMOD : " + nombreVersTexte(code) + " " + FMOD_ErrorString(code);
		throw Exc_Son(v);
	}
	
	return false;
}

void Audio::initialiser() {
	FMOD_System_Create(&_systeme);
	resultat = FMOD_System_Init(_systeme, NB_CANAUX, FMOD_INIT_NORMAL, 0);
	erreur(resultat, 1);

	for(int i = 0; i < NB_CANAUX; ++i) {
		_canaux[i] = 0;
	}
}

void Audio::nettoyer() {
	resultat = FMOD_System_Close(_systeme);
	erreur(resultat, 2);

	resultat = FMOD_System_Release(_systeme);
	erreur(resultat, 3);
}

Audio::audio_t Audio::chargerSon(std::string const &chemin) throw(Audio::Exc_Son) {
	audio_t son = 0;
	resultat = FMOD_System_CreateSound(_systeme, chemin.c_str(), FMOD_CREATESAMPLE | FMOD_2D | FMOD_LOOP_NORMAL, 0, &son);
	erreur(resultat, 4);
	

	return son;
}

Audio::audio_t Audio::chargerMusique(std::string const &chemin) throw(Audio::Exc_Son) {
	audio_t son = 0;
	resultat = FMOD_System_CreateSound(_systeme, chemin.c_str(), FMOD_SOFTWARE | FMOD_2D | FMOD_CREATESTREAM | FMOD_LOOP_NORMAL, 0, &son);
	erreur(resultat, 5);
	
	FMOD_Sound_SetLoopCount(son, -1);
	
	return son;
}

void Audio::libererSon(audio_t son) {
	resultat = FMOD_Sound_Release(son);
	erreur(resultat, 6);
}

void Audio::jouerSon(audio_t son, bool boucle) throw(Audio::Exc_Son) {
	if(boucle)
		FMOD_Sound_SetLoopCount(son, -1);
	else
		FMOD_Sound_SetLoopCount(son, 0);

	FMOD_CHANNEL *c = 0;
	resultat = FMOD_System_PlaySound(_systeme, FMOD_CHANNEL_FREE, son, false, &c);
	erreur(resultat, 7);
	
	int nb;
	FMOD_Channel_GetIndex(c, &nb);
	_canaux[nb] = c;
	
	FMOD_Channel_SetCallback(c, &sonStoppe);
	FMOD_Channel_SetVolume(c, Parametres::volumeEffets());
}

void Audio::definirMusique(audio_t m) {
	if(_musique == m)
		return;
	
	if(_canalMusique) {
		FMOD_Channel_Stop(_canalMusique);
		_canalMusique = 0;
	}
	
	_musique = m;
}

void Audio::jouerMusique() throw(Audio::Exc_Son) {
	if(_canalMusique) {
		FMOD_BOOL pause;
		FMOD_Channel_GetPaused(_canalMusique, &pause);
		if(!pause)
			return;
	}
	
	resultat = FMOD_System_PlaySound(_systeme, FMOD_CHANNEL_FREE, _musique, false, &_canalMusique);
	erreur(resultat, 8);
	
	int nb;
	FMOD_Channel_GetIndex(_canalMusique, &nb);
	_canaux[nb] = _canalMusique;
		
	FMOD_Channel_SetVolume(_canalMusique, Parametres::volumeMusique());
}

void Audio::pauseMusique() {
	if(!_canalMusique)
		return;
	
	FMOD_BOOL pause;
	FMOD_Channel_GetPaused(_canalMusique, &pause);
	
	FMOD_Channel_SetPaused(_canalMusique, !pause);
}

void Audio::definirVolumeMusique(float v) {
	if(!_canalMusique)
		return;
		
	FMOD_Channel_SetVolume(_canalMusique, v);
}

void Audio::definirVolumeEffets(float v) {
	for(int i = 0; i < NB_CANAUX; ++i) {
		if(_canaux[i] && _canaux[i] != _canalMusique) {
				
			FMOD_Channel_SetVolume(_canaux[i], v);
		}
	}
}

void Audio::maj() throw(Exc_Son) {
	resultat = FMOD_System_Update(_systeme);
	erreur(resultat, 9);
}

