//
//  Audio.h
//  Projet2MIC
//
//  Créé par Marc Promé et Rémi Saurel.
//  Ce fichier et son contenu sont librement distribuables, modifiables et utilisables pour toute œuvre non commerciale, à condition d'en citer les auteurs.
//

#ifndef Projet2MIC_Audio_h
#define Projet2MIC_Audio_h

#include <string>
#include <exception>
#include <iostream>


struct FMOD_SOUND;

namespace Audio {
	typedef FMOD_SOUND *audio_t;
	
	class Exc_Son : public std::exception {
	public:
		Exc_Son(std::string const &v) throw() : std::exception(), _valeur(v) {
			std::cerr << v << std::endl;
		}
		virtual ~Exc_Son() throw() { }
		virtual const char* what() const throw() { return _valeur.c_str(); }
	private:
		std::string _valeur;
	};

	audio_t chargerSon(std::string const &chemin) throw(Exc_Son);
	audio_t chargerMusique(std::string const &chemin) throw(Exc_Son);
	
	void libererSon(audio_t son);
	
	void jouerSon(audio_t son, bool boucle = false) throw(Exc_Son);
	
	void definirMusique(audio_t m);
	void jouerMusique() throw(Exc_Son);
	void pauseMusique();

	void definirVolumeMusique(float v);
	void definirVolumeEffets(float v);
	
	void maj() throw(Exc_Son);
}



#endif
