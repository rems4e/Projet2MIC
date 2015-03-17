//
//  Session.cpp
//  Projet2MIC
//
//  Créé par Marc Promé et Rémi Saurel.
//  Ce fichier et son contenu sont librement distribuables, modifiables et utilisables pour toute œuvre non commerciale, à condition d'en citer les auteurs.
//

#include "Session.h"
#include "Affichage.h"
#include "Texte.h"
#include "SDL/SDL.h"
#include "Image.h"
#include "Partie.h"
#include "Shader.h"
#include <ctime>
#include "Menu.h"
#include "tinyxml.h"
#include "Audio.h"
#include "VueInterface.h"
#include <stack>

#define FREQUENCE_RAFRAICHISSEMENT (Parametres::limiteIPS() ? 100 : 10000)

// Répétition événements
#define DELAI_AVANT_REPETITION 500
#define DELAI_REPETITION 80
#define DELAI_REPETITION_CLIC 0.3f

namespace Ecran {
	// Met à jour l'affichage en fonction des éléments affichés depuis le dernier appel à la fonction
	void maj();
	void finaliser();
	// Remplis l'écran avec une couleur unie
	void effacer();
	void init(unsigned int largeur, unsigned int hauteur, bool pleinEcran);
	void nettoyagePreliminaire();
	void nettoyageFinal();
	void majVue(int largeur, int hauteur);
	void ajouterVerrouTransformations();
	// On ne doit pas supprimer la première case de la pile.
	void supprimerVerrouTransformations() throw(int);
	void retourVerrouTransformations();
}

namespace TexturePrive {
	void nettoyer();
}

namespace Affichage {
	void initialiser();
}

namespace Parametres {
	void charger();
	void enregistrer();
	void nettoyer();
}

namespace Audio {
	void initialiser();
	void nettoyer();
}

namespace Session {
	horloge_t _horlogeBoucle = 0.0f, _horlogeSurvol = 0.0f, _horlogeClic = 0.0f;
	std::string _chemin;
	bool _init = false;
	
	int _nbClic[2] = {0};
	glm::vec2 _sourisClic = vec2Aucun;
	bool _evenements[nombreEvenements] = {false};
	modificateur_touche_t _modificateurTouches = M_AUCUN;
	glm::vec2 _souris, _sourisSurvol;
	Traducteur *_traducteur = nullptr;
	
	VueInterface *_vueFenetre = nullptr, *_vueTemp = nullptr;
	bool _retourHierarchie, _continuer;
	bool _horlogesAjoutees;

	std::list<HorlogeLocale *> _horloges;
	std::map<VueInterface *, horloge_t> _derniersTemps;
	std::stack<VueInterface *> _vues;
	std::list<std::pair<VueInterface *, bool> > _vuesASupprimer;

	Traducteur const &traducteur() {
		return *_traducteur;
	}
	
	bool modificateurTouches(modificateur_touche_t const &mod) { return _modificateurTouches & mod; }
	modificateur_touche_t const &modificateurTouches() { return _modificateurTouches; }
	bool evenement(evenement_t const &e) {
		if(e == aucunEvenement)
			return false;
		return _evenements[e];
	}
	
	bool evenementDiscret(evenement_t const &e) {
		return _evenements[e] >= 2000.0f;
	}
	void traiterEvenementDiscret(evenement_t const &e, bool actif);
	
	glm::vec2 const &souris() { return _souris; }
	
	void reinitialiser(evenement_t const &e) {
		if(e == aucunEvenement)
			return;
		_evenements[e] = false;
	}
	void definirQuitter(bool q) { _evenements[QUITTER] = q; }
	
	bool gestionEvenements();
	void reinitialiserEvenementsClavier();
	
	void rechargerLangue();
}

//#if defined(__MACOSX__) && !defined(DEVELOPPEMENT)
extern "C" {
#include <CoreFoundation/CoreFoundation.h>
}
//#endif

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
	
#if defined(DEVELOPPEMENT)
	std::cout << "Version de développement !" << std::endl;
#endif
	
	// Ici on règle le problème des chemins de fichiers différents suivant le système d'exploitation
/*#if defined(__MACOSX__)
#if defined(DEVELOPPEMENT)*/
	_chemin = "/Users/remi/Documents/INSA/2e année MIC/Projet/gitProjet/Projet2MIC/";
/*#else
	CFBundleRef bun = CFBundleGetMainBundle();
	CFURLRef r = CFBundleCopyResourcesDirectoryURL(bun);
	CFURLRef res = CFURLCopyAbsoluteURL(r);
	CFStringRef str = CFURLCopyFileSystemPath(res, 0);
	CFIndex taille = CFStringGetLength(str);
	
	// La longueur retournée est le nombre de caractères UTF-16, ne connaissant pas assez bien la norme, je fais le supposition suivante :
	// On considère le pire des cas ou le caractere UTF-16 est codé sur 2 octet et celui en UTF-8 sur 4 octets. D'où le taille * 2.
	char * cStr = new char[2 * taille + 1];
	CFStringGetCString(str, cStr, 2 * taille + 1, kCFStringEncodingUTF8);
	_chemin += cStr;
	_chemin += "/";
	delete[] cStr;
	
	CFRelease(str);
	CFRelease(res);
	CFRelease(r);
#endif
#elif defined(__LINUX__)
	
#elif defined(__WIN32__)
	
#else
	
#endif*/
	_chemin += "data/";
		
	Audio::initialiser();
	Affichage::initialiser();
	Parametres::charger();
		
	Ecran::init(Parametres::largeurEcran(), Parametres::hauteurEcran(), Parametres::pleinEcran());
	
	Texte txt(TRAD("gen Chargement…"), POLICE_DECO, TAILLE_TEXTE_CHARGEMENT, Couleur::blanc);
	Affichage::afficherRectangle(Ecran::ecran(), Couleur::noir);
	txt.afficher(glm::vec2((Ecran::dimensions().x - txt.dimensions().x) / 2, (Ecran::dimensions().y - txt.dimensions().y) / 2 - 50));
	Ecran::maj();
		
	_vues.push(0);
	_retourHierarchie = false;
	_horlogesAjoutees = false;

	Session::reinitialiserEvenements();
}

void Session::nettoyer() {
	Parametres::nettoyer();
	
	Audio::nettoyer();
	
	Ecran::nettoyagePreliminaire();
	Texte::nettoyer();
	TexturePrive::nettoyer();
	Shader::nettoyer();
	Ecran::nettoyageFinal();
	
	SDL_Quit();
}

void Session::rechargerLangue() {
	delete _traducteur;
	_traducteur = new Traducteur(Session::cheminRessources() + Parametres::langue() + ".trad");
}

bool Session::gestionEvenements() {
	static SDL_Event evenement;
	static Uint16 correspondances[1 << (sizeof(static_cast<SDL_Event *>(0)->key.keysym.scancode) * 8)] = {0};
	static bool premierSouris = false;
	
	
	if(!SDL_PollEvent(&evenement)) {
		for(evenement_t ee = PREMIER_EVENEMENT_CLAVIER; ee <= PREMIER_EVENEMENT_CLAVIER; ++ee) {
			if(_evenements[ee]) {
				Session::traiterEvenementDiscret(ee, true);
			}
		}

		return false;
	}
	
	switch(evenement.type) {
		case SDL_VIDEORESIZE:
			//Ecran::majVue(evenement.resize.w, evenement.resize.h);
			break;
		case SDL_MOUSEMOTION:
			_souris.x = evenement.motion.x;
			_souris.y = evenement.motion.y;
			if(premierSouris)
				_evenements[SOURIS] = true;
			premierSouris = true;
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
				Session::traiterEvenementDiscret(evenement_t(T_a + temp - 'a'), evenement.type == SDL_KEYDOWN);
			}
			else if(temp >= 'A' && temp <= 'Z') {
				Session::traiterEvenementDiscret(evenement_t(T_a + temp - 'A'), evenement.type == SDL_KEYDOWN);
			}
			else if(temp >= '0' && temp <= '9') {
				Session::traiterEvenementDiscret(evenement_t(T_0 + temp - '0'), evenement.type == SDL_KEYDOWN);
			}
			
			/* Opérations, comparaisons */
			else if(temp == '+')
				Session::traiterEvenementDiscret(T_PLUS, evenement.type == SDL_KEYDOWN);
			else if(temp == '-')
				Session::traiterEvenementDiscret(T_MOINS, evenement.type == SDL_KEYDOWN);
			else if(temp == '/')
				Session::traiterEvenementDiscret(T_SLASH, evenement.type == SDL_KEYDOWN);
			else if(temp == '*')
				Session::traiterEvenementDiscret(T_ASTERISQUE, evenement.type == SDL_KEYDOWN);
			else if(temp == '<')
				Session::traiterEvenementDiscret(T_INFERIEUR, evenement.type == SDL_KEYDOWN);
			else if(temp == '>')
				Session::traiterEvenementDiscret(T_SUPERIEUR, evenement.type == SDL_KEYDOWN);
			else if(temp == '=')
				Session::traiterEvenementDiscret(T_EGAL, evenement.type == SDL_KEYDOWN);
			
			/* Ponctuation */
			else if(temp == '_')
				Session::traiterEvenementDiscret(T_TIRET_BAS, evenement.type == SDL_KEYDOWN);
			else if(temp == '.')
				Session::traiterEvenementDiscret(T_POINT, evenement.type == SDL_KEYDOWN);
			else if(temp == ',')
				Session::traiterEvenementDiscret(T_VIRGULE, evenement.type == SDL_KEYDOWN);
			else if(temp == ':')
				Session::traiterEvenementDiscret(T_DEUX_POINTS, evenement.type == SDL_KEYDOWN);
			else if(temp == ';')
				Session::traiterEvenementDiscret(T_POINT_VIRGULE, evenement.type == SDL_KEYDOWN);
			else if(temp == '!')
				Session::traiterEvenementDiscret(T_POINT_EXCLAMATION, evenement.type == SDL_KEYDOWN);
			else if(temp == '?')
				Session::traiterEvenementDiscret(T_POINT_INTERROGATION, evenement.type == SDL_KEYDOWN);
			else if(temp == '&')
				Session::traiterEvenementDiscret(T_ESPERLUETTE, evenement.type == SDL_KEYDOWN);
			else if(temp == '"')
				Session::traiterEvenementDiscret(T_GUILLEMETS, evenement.type == SDL_KEYDOWN);
			else if(temp == '\'')
				Session::traiterEvenementDiscret(T_APOSTROPHE, evenement.type == SDL_KEYDOWN);
			else if(temp == '$')
				Session::traiterEvenementDiscret(T_DOLLAR, evenement.type == SDL_KEYDOWN);
			else if(temp == '#')
				Session::traiterEvenementDiscret(T_HASH, evenement.type == SDL_KEYDOWN);
			else if(temp == '\\')
				Session::traiterEvenementDiscret(T_BACKSLASH, evenement.type == SDL_KEYDOWN);
			
			/* Parenthèses */
			else if(temp == '(')
				Session::traiterEvenementDiscret(T_PARENTHESE_G, evenement.type == SDL_KEYDOWN);
			else if(temp == ')')
				Session::traiterEvenementDiscret(T_PARENTHESE_D, evenement.type == SDL_KEYDOWN);
			else if(temp == '[')
				Session::traiterEvenementDiscret(T_CROCHET_G, evenement.type == SDL_KEYDOWN);
			else if(temp == ']')
				Session::traiterEvenementDiscret(T_CROCHET_D, evenement.type == SDL_KEYDOWN);
			
			else {
				switch(temp2) {
					case SDLK_ESCAPE:
						Session::traiterEvenementDiscret(T_ESC, evenement.type == SDL_KEYDOWN);
						break;
					case SDLK_BACKSPACE:
						Session::traiterEvenementDiscret(T_EFFACER, evenement.type == SDL_KEYDOWN);
						break;
					case SDLK_UP:
						Session::traiterEvenementDiscret(T_HAUT, evenement.type == SDL_KEYDOWN);
						break;
					case SDLK_DOWN:
						Session::traiterEvenementDiscret(T_BAS, evenement.type == SDL_KEYDOWN);
						break;
					case SDLK_LEFT:
						Session::traiterEvenementDiscret(T_GAUCHE, evenement.type == SDL_KEYDOWN);
						break;
					case SDLK_RIGHT:
						Session::traiterEvenementDiscret(T_DROITE, evenement.type == SDL_KEYDOWN);
						break;
					case SDLK_KP_ENTER:
					case SDLK_RETURN:
						Session::traiterEvenementDiscret(T_ENTREE, evenement.type == SDL_KEYDOWN);
						break;
					case SDLK_SPACE:
						Session::traiterEvenementDiscret(T_ESPACE, evenement.type == SDL_KEYDOWN);
						break;
					case SDLK_TAB:
						Session::traiterEvenementDiscret(T_TAB, evenement.type == SDL_KEYDOWN);
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
	
	return true;
}

void Session::traiterEvenementDiscret(evenement_t const &e, bool actif) {
	if(!actif) {
		_evenements[e] = 0;
	}
	else {
		if(_evenements[e] == 0) {
			_evenements[e] = 3000.0f;
			return;
		}
		
		if(_evenements[e] == 3000.0f) {
			_evenements[e] = 0.0f;
		}
		else if(_evenements[e] == 2000.0f)
			_evenements[e] = DELAI_AVANT_REPETITION;
		else if(_evenements[e] >= DELAI_AVANT_REPETITION) {
			_evenements[e] = 2000.0f;
			return;
		}
		_evenements[e] = _evenements[e] + 1000.0f / Ecran::frequenceInstantanee();
		if(_evenements[e] >= DELAI_AVANT_REPETITION + DELAI_REPETITION) {
			_evenements[e] = 2000.0f;
		}
	}
}

/*bool Session::boucle(float freq, bool continuer) {
	if(!continuer) {
		Session::reinitialiserEvenements();
		return false;
	}
	
	Audio::maj();
	
	_evenements[SOURIS] = false;
	while(Session::gestionEvenements());
	
	attendre(1.0f / freq - (horloge() - Session::_horlogeBoucle));
	Session::_horlogeBoucle = horloge();
	
	return continuer;
}*/

VueInterface *Session::vueFenetre() {
	return _vueFenetre;
}

void Session::ajouterVueFenetre(VueInterface *vue) {
	assert(vue);
	
	_vueTemp = vue;
	_continuer = true;
	while(Session::boucle(FREQUENCE_RAFRAICHISSEMENT, _continuer));
}

void Session::supprimerVueFenetre() {
	_continuer = false;
}

bool Session::boucle(float freq, bool continuer) {
	Audio::maj();
	
	for(std::list<std::pair<VueInterface *, bool> >::iterator i = _vuesASupprimer.begin(); i != _vuesASupprimer.end(); ++i) {
		i->first->parent()->supprimerEnfant(*i->first);
		if(i->second)
			delete i->first;
	}
	_vuesASupprimer.clear();
	
	if(!continuer) {
		Session::reinitialiserEvenements();
		Session::_vueFenetre = 0;
		_horlogeSurvol = horloge();
		
		VueInterface::definirVueActuelle(0);
		
		_vues.pop();
		_vueFenetre = _vues.top();
		_vueTemp = _vueFenetre;
		_retourHierarchie = true;
		_continuer = true;
		
		return false;
	}
	if(_horlogesAjoutees) {
		for(std::list<Session::HorlogeLocale *>::iterator i = Session::_horloges.begin(); i != Session::_horloges.end(); ++i) {
			if((*i)->_v == 0) {
				(*i)->_v = _vueTemp;
			}
		}
		_horlogesAjoutees = false;
	}
	if(Session::_vueFenetre != _vueTemp || _retourHierarchie) {
		if(!_retourHierarchie || _vues.top() != _vueTemp) {
			_vues.push(_vueTemp);
		}
		
		_retourHierarchie = false;
		Session::reinitialiserEvenements();
		_horlogeSurvol = horloge();
		
		horloge_t tpsV = _derniersTemps[_vueTemp];
		horloge_t tps = horloge();
		for(std::list<Session::HorlogeLocale *>::iterator i = Session::_horloges.begin(); i != Session::_horloges.end(); ++i) {
			if((*i)->_v == _vueTemp && ***i != 0.0) {
				***i += tps - tpsV;
			}
		}
		
		_vueFenetre = _vueTemp;
		VueInterface::definirVueActuelle(_vueFenetre);
	}
	
	Ecran::effacer();
	int couche = 0, ancien;
	do {
		ancien = couche;
		_vueFenetre->preparationDessin();
		Ecran::ajouterCadreAffichage(_vueFenetre->cadre());
		Ecran::ajouterVerrouTransformations();

		_vueFenetre->afficher(glm::vec2(0), ancien, couche);
		glDisable(GL_DEPTH_TEST);

		Ecran::retourVerrouTransformations();
		Ecran::supprimerVerrouTransformations();
		Ecran::supprimerCadreAffichage();
	} while(ancien != couche);
	
	Ecran::finaliser();

	bool const aClic[2] = {_evenements[B_GAUCHE], _evenements[B_DROIT]};

	_evenements[SOURIS] = false;

	while(Session::gestionEvenements());
	
	if(_souris != _sourisClic) {
		_nbClic[0] = _nbClic[1] = 0;
	}
	if(horloge() - _horlogeClic > DELAI_REPETITION_CLIC) {
		_nbClic[0] = _nbClic[1] = 0;
	}
	
	if(_evenements[B_GAUCHE] && !aClic[0]) {
		_nbClic[1] = 0;
		_sourisClic = _souris;
		_horlogeClic = horloge();
	}
	else if(!_evenements[B_GAUCHE] && aClic[0]) {
		++_nbClic[0];
	}
	
	if(_souris != _sourisSurvol && horloge() - _horlogeSurvol < 1.0f) {
		_horlogeSurvol = horloge();
		_sourisSurvol = _souris;
	}
	
	if(_vueFenetre) {
		_derniersTemps[_vueTemp] = horloge();
		
		if(VueInterface *a = _vueFenetre->gestionClic()) {
			VueInterface::definirVueActuelle(a);
		}
		_vueFenetre->gestionGestionClavier();
		
		if(VueInterface *tmp = VueInterface::vueActuelle()) {
			tmp->gestionSouris(true, Session::souris() - tmp->positionAbsolue(), _evenements[B_GAUCHE], _evenements[B_DROIT]);
		}
		if(!_evenements[B_GAUCHE] && !_evenements[B_DROIT]) {
			if(VueInterface *tmp = _vueFenetre->gestionSurvol()) {
				tmp->gestionSouris(false, Session::souris() - tmp->positionAbsolue(), _evenements[B_GAUCHE], _evenements[B_DROIT]);
				if(tmp->description().empty())
					_horlogeSurvol = horloge();
				else if(horloge() - _horlogeSurvol >= 1.0f) {
					Ecran::definirBulleAide(tmp->description());
				}
			}
			else {
				_horlogeSurvol = horloge();
			}
		}
		else {
			_horlogeSurvol = horloge();
		}
		
		Ecran::maj();
	}
	
	_vueTemp = _vueFenetre;
	
	attendre(1.0f / freq - (horloge() - Session::_horlogeBoucle));
	Session::_horlogeBoucle = horloge();
	
	return continuer;
}

void Session::supprimerVueProchaineIteration(VueInterface *vue, bool sup) {
	if(vue) {
		_vuesASupprimer.push_back(std::make_pair(vue, sup));
		if(vue == VueInterface::vueActuelle())
			VueInterface::definirVueActuelle(0);
	}
}

Unichar Session::transcriptionEvenement(Session::evenement_t const &e, bool trad) {
	if(e >= T_a && e <= T_z) {
		char t[2] = {static_cast<char>('a' + (e - T_a))};
		return Unichar(t);
	}
	else if(e >= T_0 && e <= T_9) {
		char t[2] = {static_cast<char>('0' + e - T_0)};
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
		return trad ? TRAD("tev espace") : "espace";
	else if(e == T_TAB)
		return trad ? TRAD("tev tab") : "tab";
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
		return trad ? TRAD("tev gauche") : "gauche";
	else if(e == T_DROITE)
		return trad ? TRAD("tev droite") : "droite";
	else if(e == T_HAUT)
		return trad ? TRAD("tev haut") : "haut";
	else if(e == T_BAS)
		return trad ? TRAD("tev bas") : "bas";
	
	else if(e == T_ESC)
		return trad ? TRAD("tev esc") : "esc";
	else if(e == T_EFFACER)
		return trad ? TRAD("tev effacer") : "effacer";
	else if(e == T_ENTREE)
		return trad ? TRAD("tev entrée") : "entrée";
	else if(e == QUITTER)
		return trad ? TRAD("tev quitter") : "quitter";
	else if(e == B_GAUCHE)
		return trad ? TRAD("tev souris gauche") : "souris gauche";
	else if(e == B_DROIT)
		return trad ? TRAD("tev souris droite") : "souris droite";
	else if(e == SOURIS)
		return trad ? TRAD("tev déplacement souris") : "déplacement souris";

	return trad ? TRAD("tev aucun événement") : "<aucun événement>";
}

Unichar Session::transcriptionModificateur(Session::modificateur_touche_t const &m, bool trad) {
	Unichar retour;
	
	if(m & M_CONTROLE)
		retour += trad ? TRAD("tmod ctrl") : "ctrl";
	if(m & M_OPTION)
		retour += (!retour.empty() ? Unichar("+") : Unichar::uninull) + (trad ? TRAD("tmod alt") : "alt");
	if(m & M_MAJ)
		retour += (!retour.empty() ? Unichar("+") : Unichar::uninull) + (trad ? TRAD("tmod maj") : "maj");
	if(m & M_COMMANDE)
		retour += (!retour.empty() ? Unichar("+") : Unichar::uninull) + (trad ? TRAD("tmod cmd") : "cmd");
	
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
