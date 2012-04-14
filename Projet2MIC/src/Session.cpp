/*
 *  Session.cpp
 *  Jeu C++
 *
 *  Created by Rémi on 24/07/07.
 *  Copyright 2007 Rémi Saurel. All rights reserved.
 *
 */

#include "Session.h"
#include "Ecran.h"
#include "Texte.h"
#include "SDL/SDL.h"
#include "Image.h"
#include "Partie.h"
#include "Shader.h"

namespace Ecran {
	void init(unsigned int largeur, unsigned int hauteur, unsigned int profondeur, bool pleinEcran);
	void nettoyagePreliminaire();
	void nettoyageFinal();
}

namespace ImagesBase {
	void initialiser();
	void nettoyer();
	void changerTexture(GLint tex);
}

namespace Parametres {
	void charger();
	void enregistrer();
	void nettoyer();
}

namespace Session {
	horloge_t _horlogeBoucle = 0.0f;
	std::string _chemin;
	bool _init = false;
	
	bool _evenements[nombreEvenements] = {false};
	modificateur_touche_t _modificateurTouches = M_AUCUN;
	Coordonnees _souris;
	
	bool modificateurTouches(modificateur_touche_t const &mod) { return _modificateurTouches & mod; }
	modificateur_touche_t const &modificateurTouches() { return _modificateurTouches; }
	bool evenement(evenement_t const &e) {
		if(e == aucunEvenement)
			return false;
		return _evenements[e];
	}
	Coordonnees const &souris() { return _souris; }
	
	void reinitialiser(evenement_t const &e) {
		if(e == aucunEvenement)
			return;
		_evenements[e] = false;
	}
	void definirQuitter(bool q) { _evenements[QUITTER] = q; }
	
	void gestionEvenements();
	void reinitialiserEvenementsClavier();
}

#if defined(__MACOSX__) && !defined(DEVELOPPEMENT)
extern "C" {
#include <CoreFoundation/CoreFoundation.h>
}
#endif

std::string const &Session::cheminRessources() {
	return _chemin;
}

void Session::initialiser() {
	if(Session::_init)
		return;
	Session::_init = true;

	std::srand(static_cast<unsigned int>(std::time(0)));
	if(SDL_Init(SDL_INIT_VIDEO) < 0) {
		std::cerr << "Impossible d'initialiser la SDL : " << SDL_GetError() << std::endl;
		exit(1);
	}
	SDL_EnableUNICODE(1);
	
	// Ici on règle le problème des chemins de fichiers différents suivant le système d'exploitation
#if defined(__MACOSX__)
#if defined(DEVELOPPEMENT)
	_chemin = "/Users/remi/Documents/INSA/2e année MIC/Projet/gitProjet/Projet2MIC/";
#else
	CFBundleRef bun = CFBundleGetMainBundle();
	CFURLRef r = CFBundleCopyResourcesDirectoryURL(bun);
	CFURLRef res = CFURLCopyAbsoluteURL(r);
	CFStringRef str = CFURLCopyFileSystemPath(res, 0);
	CFIndex taille = CFStringGetLength(str);
	char * cStr = new char[taille + 1];
	CFStringGetCString(str, cStr, taille + 1, kCFStringEncodingUTF8);
	chemin += cStr;
	chemin += "/";
	delete[] cStr;
	
	CFRelease(str);
	CFRelease(res);
	CFRelease(r);
#endif
#elif defined(__LINUX__)
	
#elif defined(__WIN32__)
	
#else
	
#endif
	_chemin += "data/";

	ImagesBase::initialiser();
	Ecran::init(LARGEUR_ECRAN, HAUTEUR_ECRAN, PROFONDEUR_COULEURS, PLEIN_ECRAN);
	Shader::initialiser();
	
	Texte txt("Chargement programme", POLICE_DECO, TAILLE_TEXTE_CHARGEMENT, Couleur::blanc);
	Ecran::afficherRectangle(Ecran::ecran(), Couleur::noir);
	txt.afficher(Coordonnees((Ecran::dimensions().x - txt.dimensions().x) / 2, (Ecran::dimensions().y - txt.dimensions().y) / 2 - 50));
	Ecran::maj();
	
	Parametres::charger();
	
	Session::reinitialiserEvenements();
}

void Session::nettoyer() {
	Parametres::nettoyer();
	
	Ecran::nettoyagePreliminaire();
	Texte::nettoyer();
	ImagesBase::nettoyer();
	Shader::nettoyer();
	Ecran::nettoyageFinal();

	SDL_Quit();
}

void Session::menu() {
	Partie *nouvellePartie = Partie::creerPartie();
	nouvellePartie->commencer();
	delete nouvellePartie;
}

void Session::gestionEvenements() {
	static SDL_Event evenement;
	static Uint16 correspondances[1 << (sizeof(static_cast<SDL_Event *>(0)->key.keysym.scancode) * 8)] = {0};
	
	if(!SDL_PollEvent(&evenement)) {
		return;
	}
	
	_evenements[SOURIS] = false;
	
	switch(evenement.type) {
		case SDL_MOUSEMOTION:
			_souris.x = evenement.motion.x;
			_souris.y = evenement.motion.y;
			_evenements[SOURIS] = true;
			break;
		case SDL_MOUSEBUTTONDOWN:
			_souris.x = evenement.button.x;
			_souris.y = evenement.button.y;
			switch(evenement.button.button) {
				case SDL_BUTTON_LEFT:
					_evenements[B_GAUCHE] = true;
					break;
				case SDL_BUTTON_RIGHT:
					_evenements[B_DROIT] = true;
					break;
			}
			break;
		case SDL_MOUSEBUTTONUP:
			_souris.x = evenement.button.x;
			_souris.y = evenement.button.y;
			switch(evenement.button.button) {
				case SDL_BUTTON_LEFT:
					_evenements[B_GAUCHE] = false;
					break;
				case SDL_BUTTON_RIGHT:
					_evenements[B_DROIT] = false;
					break;
			}
			break;
		case SDL_KEYDOWN:
			if(evenement.key.keysym.unicode != 0) {
				correspondances[evenement.key.keysym.scancode] = evenement.key.keysym.unicode;
			}
			
		case SDL_KEYUP: {
			int temp = evenement.key.keysym.unicode;
			int temp2 = evenement.key.keysym.sym;
			
			if(evenement.type == SDL_KEYUP && (evenement.key.keysym.scancode != 0 || (evenement.key.keysym.sym < SDLK_F15))) {
				temp = correspondances[evenement.key.keysym.scancode];
			}
			_modificateurTouches = M_AUCUN;
			
			if((evenement.key.keysym.mod & KMOD_LSHIFT) || (evenement.key.keysym.mod & KMOD_RSHIFT)) {
				_modificateurTouches |= M_MAJ;
			}
			if((evenement.key.keysym.mod & KMOD_LCTRL) || (evenement.key.keysym.mod & KMOD_RCTRL)) {
				_modificateurTouches |= M_CONTROLE;
			}
			if((evenement.key.keysym.mod & KMOD_LALT) || (evenement.key.keysym.mod & KMOD_RALT)) {
				_modificateurTouches |= M_OPTION;
			}
			if((evenement.key.keysym.mod & KMOD_LMETA) || (evenement.key.keysym.mod & KMOD_RMETA)) {
				_modificateurTouches |= M_COMMANDE;
			}
			
			/* Lettres, chiffres */
			if(temp >= 'a' && temp <= 'z') {
				_evenements[evenement_t(T_a + temp - 'a')] = evenement.type == SDL_KEYDOWN;
			}
			else if(temp >= 'A' && temp <= 'Z') {
				_evenements[evenement_t(T_a + temp - 'A')] = evenement.type == SDL_KEYDOWN;
			}
			else if(temp >= '0' && temp <= '9') {
				_evenements[evenement_t(T_0 + temp - '0')] = evenement.type == SDL_KEYDOWN;
			}
			
			/* Opérations, comparaisons */
			else if(temp == '+')
				_evenements[T_PLUS] = evenement.type == SDL_KEYDOWN;
			else if(temp == '-')
				_evenements[T_MOINS] = evenement.type == SDL_KEYDOWN;
			else if(temp == '/')
				_evenements[T_SLASH] = evenement.type == SDL_KEYDOWN;
			else if(temp == '*')
				_evenements[T_ASTERISQUE] = evenement.type == SDL_KEYDOWN;
			else if(temp == '<')
				_evenements[T_INFERIEUR] = evenement.type == SDL_KEYDOWN;
			else if(temp == '>')
				_evenements[T_SUPERIEUR] = evenement.type == SDL_KEYDOWN;
			else if(temp == '=')
				_evenements[T_EGAL] = evenement.type == SDL_KEYDOWN;
			
			/* Ponctuation */
			else if(temp == '_')
				_evenements[T_TIRET_BAS] = evenement.type == SDL_KEYDOWN;
			else if(temp == '.')
				_evenements[T_POINT] = evenement.type == SDL_KEYDOWN;
			else if(temp == ',')
				_evenements[T_VIRGULE] = evenement.type == SDL_KEYDOWN;
			else if(temp == ':')
				_evenements[T_DEUX_POINTS] = evenement.type == SDL_KEYDOWN;
			else if(temp == ';')
				_evenements[T_POINT_VIRGULE] = evenement.type == SDL_KEYDOWN;
			else if(temp == '!')
				_evenements[T_POINT_EXCLAMATION] = evenement.type == SDL_KEYDOWN;
			else if(temp == '?')
				_evenements[T_POINT_INTERROGATION] = evenement.type == SDL_KEYDOWN;
			else if(temp == '&')
				_evenements[T_ESPERLUETTE] = evenement.type == SDL_KEYDOWN;
			else if(temp == '"')
				_evenements[T_GUILLEMETS] = evenement.type == SDL_KEYDOWN;
			else if(temp == '\'')
				_evenements[T_APOSTROPHE] = evenement.type == SDL_KEYDOWN;
			else if(temp == '$')
				_evenements[T_DOLLAR] = evenement.type == SDL_KEYDOWN;
			else if(temp == '#')
				_evenements[T_HASH] = evenement.type == SDL_KEYDOWN;
			else if(temp == '\\')
				_evenements[T_BACKSLASH] = evenement.type == SDL_KEYDOWN;
			
			/* Parenthèses */
			else if(temp == '(')
				_evenements[T_PARENTHESE_G] = evenement.type == SDL_KEYDOWN;
			else if(temp == ')')
				_evenements[T_PARENTHESE_D] = evenement.type == SDL_KEYDOWN;
			else if(temp == '[')
				_evenements[T_CROCHET_G] = evenement.type == SDL_KEYDOWN;
			else if(temp == ']')
				_evenements[T_CROCHET_D] = evenement.type == SDL_KEYDOWN;
			
			else {
				switch(temp2) {
					case SDLK_ESCAPE:
						_evenements[T_ESC] = evenement.type == SDL_KEYDOWN;
						break;
					case SDLK_BACKSPACE:
						_evenements[T_EFFACER] = evenement.type == SDL_KEYDOWN;
						break;
					case SDLK_UP:
						_evenements[T_HAUT] = evenement.type == SDL_KEYDOWN;
						break;
					case SDLK_DOWN:
						_evenements[T_BAS] = evenement.type == SDL_KEYDOWN;
						break;
					case SDLK_LEFT:
						_evenements[T_GAUCHE] = evenement.type == SDL_KEYDOWN;
						break;
					case SDLK_RIGHT:
						_evenements[T_DROITE] = evenement.type == SDL_KEYDOWN;
						break;
					case SDLK_KP_ENTER:
					case SDLK_RETURN:
						_evenements[T_ENTREE] = evenement.type == SDL_KEYDOWN;
						break;
					case SDLK_SPACE:
						_evenements[T_ESPACE] = evenement.type == SDL_KEYDOWN;
						break;
					case SDLK_TAB:
						_evenements[T_TAB] = evenement.type == SDL_KEYDOWN;
						break;
					case SDLK_LSHIFT:
					case SDLK_RSHIFT:
						_modificateurTouches |= static_cast<modificateur_touche_t>(M_MAJ * evenement.type == SDL_KEYDOWN);
						break;
					case SDLK_LMETA:
					case SDLK_RMETA:
						_modificateurTouches |= static_cast<modificateur_touche_t>(M_COMMANDE * evenement.type == SDL_KEYDOWN);
						break;
					case SDLK_LCTRL:
					case SDLK_RCTRL:
						_modificateurTouches |= static_cast<modificateur_touche_t>(M_CONTROLE * evenement.type == SDL_KEYDOWN);
						break;
					case SDLK_LALT:
					case SDLK_RALT:
						_modificateurTouches |= static_cast<modificateur_touche_t>(M_OPTION * evenement.type == SDL_KEYDOWN);
						break;
				}
			}
			break;
		}
		case SDL_QUIT:
			_evenements[QUITTER] = true;
			break;
		default:
			break;
	}
}

bool Session::boucle(float freq, bool continuer) {
	if(!continuer) {
		Session::reinitialiserEvenements();
		return false;
	}
		
	Session::gestionEvenements();
	
	attendre(1.0f / freq - (horloge() - Session::_horlogeBoucle));
	Session::_horlogeBoucle = horloge();
	
	return continuer;
}

Unichar Session::transcriptionEvenement(Session::evenement_t const &e) {
	if(e >= T_a && e <= T_z) {
		char t[2] = {'a' + (e - T_a)};
		return Unichar(t);
	}
	else if(e >= T_0 && e <= T_9) {
		char t[2] = {'0' + e - T_0};
		return Unichar(t);
	}
	
	else if(e == T_MOINS)
		return "-";
	else if(e == T_PLUS)
		return "+";
	else if(e == T_SLASH)
		return "/";
	else if(e == T_ASTERISQUE)
		return "*";
	else if(e == T_EGAL)
		return "=";
	else if(e == T_INFERIEUR)
		return "<";
	else if(e == T_SUPERIEUR)
		return ">";
	
	else if(e == T_ESPACE)
		return "espace";
	else if(e == T_TAB)
		return "tab";
	else if(e == T_TIRET_BAS)
		return "_";
	else if(e == T_POINT)
		return ".";
	else if(e == T_VIRGULE)
		return ",";
	else if(e == T_DEUX_POINTS)
		return ":";
	else if(e == T_POINT_VIRGULE)
		return ";";
	else if(e == T_POINT_EXCLAMATION)
		return "!";
	else if(e == T_POINT_INTERROGATION)
		return "?";
	else if(e == T_ESPERLUETTE)
		return "&";
	else if(e == T_GUILLEMETS)
		return "\"";
	else if(e == T_APOSTROPHE)
		return "'";
	else if(e == T_DOLLAR)
		return "$";
	else if(e == T_HASH)
		return "#";
	else if(e == T_BACKSLASH)
		return "\\";
	
	else if(e == T_PARENTHESE_G)
		return "(";
	else if(e == T_PARENTHESE_D)
		return ")";
	else if(e == T_CROCHET_G)
		return "[";
	else if(e == T_CROCHET_D)
		return "]";
	
	else if(e == T_GAUCHE)
		return "gauche";
	else if(e == T_DROITE)
		return "droite";
	else if(e == T_HAUT)
		return "haut";
	else if(e == T_BAS)
		return "bas";
	
	else if(e == T_ESC)
		return "esc";
	else if(e == T_EFFACER)
		return "effacer";
	else if(e == T_ENTREE)
		return "entrée";
	else if(e == QUITTER)
		return "quitter";
	
	// FIXME: trad
	std::cerr << "L'événement " << e << " n'a pas de transcription textuelle !" << std::endl;
	assert(0);
	return "aucun événement";
}

Unichar Session::transcriptionModificateur(Session::modificateur_touche_t const &m) {
	Unichar retour;
	
	if(m & M_CONTROLE)
		retour += "ctrl";
	if(m & M_OPTION)
		retour += (!retour.empty() ? Unichar("+") : Unichar()) + "alt";
	if(m & M_MAJ)
		retour += (!retour.empty() ? Unichar("+") : Unichar()) + "maj";
	if(m & M_COMMANDE)
		retour += (!retour.empty() ? Unichar("+") : Unichar()) + "cmd";
	
	return retour;
}

void Session::reinitialiserEvenementsClavier() {
	for(evenement_t i = PREMIER_EVENEMENT_CLAVIER; i < DERNIER_EVENEMENT_CLAVIER; ++i) {
		_evenements[i] = false;
	}
}

void Session::reinitialiserEvenements() {
	for(evenement_t i = premierEvenement; i < nombreEvenements; ++i) {
		_evenements[i] = false;
	}
}

