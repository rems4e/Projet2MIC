//
//  Audio.cpp
//  Projet2MIC
//
//  Créé par Marc Promé et Rémi Saurel.
//  Ce fichier et son contenu sont librement distribuables, modifiables et utilisables pour toute œuvre non commerciale, à condition d'en citer les auteurs.
//

#include "Audio.h"
#include "fmod/fmod.hpp"
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
	
	FMOD::System *_systeme = 0;
	FMOD::Channel *_canaux[NB_CANAUX];
	FMOD::Channel *_canalMusique = 0;
	
	audio_t _musique = 0;

	FMOD_RESULT sonStoppe(FMOD_CHANNEL *channel, FMOD_CHANNEL_CALLBACKTYPE type, void *commanddata1, void *commanddata2);

}

FMOD_RESULT Audio::sonStoppe(FMOD_CHANNEL *channel, FMOD_CHANNEL_CALLBACKTYPE type, void *commanddata1, void *commanddata2) {
	int nb;
	FMOD_Channel_GetIndex(channel, &nb);
	
	FMOD::Channel *c;
	_systeme->getChannel(nb, &c);
	
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
	FMOD::System_Create(&_systeme);
	resultat = _systeme->init(NB_CANAUX, FMOD_INIT_NORMAL, 0);
	erreur(resultat, 1);

	for(int i = 0; i < NB_CANAUX; ++i) {
		_canaux[i] = 0;
	}
}

void Audio::nettoyer() {
	resultat = _systeme->close();
	erreur(resultat, 2);

	resultat = _systeme->release();
	erreur(resultat, 3);
}

Audio::audio_t Audio::chargerSon(std::string const &chemin) throw(Audio::Exc_Son) {
	audio_t son = 0;
	resultat = _systeme->createSound(chemin.c_str(), FMOD_CREATESAMPLE | FMOD_2D | FMOD_LOOP_NORMAL, 0, &son);
	erreur(resultat, 4);
	

	return son;
}

Audio::audio_t Audio::chargerMusique(std::string const &chemin) throw(Audio::Exc_Son) {
	audio_t son = 0;
	resultat = _systeme->createSound(chemin.c_str(), FMOD_SOFTWARE | FMOD_2D | FMOD_CREATESTREAM | FMOD_LOOP_NORMAL, 0, &son);
	erreur(resultat, 5);
	
	son->setLoopCount(-1);
	
	return son;
}

void Audio::libererSon(audio_t son) {
	resultat = son->release();
	erreur(resultat, 6);
}

void Audio::jouerSon(audio_t son, bool boucle) throw(Audio::Exc_Son) {
	if(boucle)
		son->setLoopCount(-1);
	else
		son->setLoopCount(0);

	FMOD::Channel *c = 0;
	resultat = _systeme->playSound(FMOD_CHANNEL_FREE, son, false, &c);
	erreur(resultat, 7);
	
	int nb;
	c->getIndex(&nb);
	_canaux[nb] = c;
	
	c->setCallback(&sonStoppe);
	c->setVolume(Parametres::volumeEffets());
}

void Audio::definirMusique(audio_t m) {
	if(_musique == m)
		return;
	
	if(_canalMusique) {
		_canalMusique->stop();
		_canalMusique = 0;
	}
	
	_musique = m;
}

void Audio::jouerMusique() throw(Audio::Exc_Son) {
	if(_canalMusique) {
		bool pause;
		_canalMusique->getPaused(&pause);
		if(!pause)
			return;
	}
	
	resultat = _systeme->playSound(FMOD_CHANNEL_FREE, _musique, false, &_canalMusique);
	erreur(resultat, 8);
	
	int nb;
	_canalMusique->getIndex(&nb);
	_canaux[nb] = _canalMusique;
		
	_canalMusique->setVolume(Parametres::volumeMusique());
}

void Audio::pauseMusique() {
	if(!_canalMusique)
		return;
	
	bool pause;
	_canalMusique->getPaused(&pause);
	
	_canalMusique->setPaused(!pause);
}

void Audio::definirVolumeMusique(float v) {
	if(!_canalMusique)
		return;
		
	_canalMusique->setVolume(v);
}

void Audio::definirVolumeEffets(float v) {
	for(int i = 0; i < NB_CANAUX; ++i) {
		if(_canaux[i] && _canaux[i] != _canalMusique) {
				
			_canaux[i]->setVolume(v);
		}
	}
}

void Audio::maj() throw(Exc_Son) {
	resultat = _systeme->update();
	erreur(resultat, 9);
}

