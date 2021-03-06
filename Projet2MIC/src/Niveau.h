//
//  Niveau.h
//  Projet2MIC
//
//  Créé par Marc Promé et Rémi Saurel.
//  Ce fichier et son contenu sont librement distribuables, modifiables et utilisables pour toute œuvre non commerciale, à condition d'en citer les auteurs.
//

#ifndef Projet2MIC_Niveau_h
#define Projet2MIC_Niveau_h

#include "Geometrie.h"
#include "Constantes.h"
#include <list>
#include "horloge.h"
#include <exception>
#include "Image.h"
#include "Shader.h"
#include "Audio.h"

class ElementNiveau;
class EntiteStatique;
class EntiteMobile;
class Personnage;
class Joueur;

#define LARGEUR_CASE 64
#define HAUTEUR_CASE 32

#define NB_VALEURS_PROBA_ENTITES 64 * 64
#define BASE_VALEURS_PROBA_ENTITES 64
#define CHIFFRES_VALEURS_PROBA_ENTITES 2

#define TRANSITION_GAUCHE 0
#define TRANSITION_BAS 1
#define TRANSITION_ANGLE 2

class Niveau {
	friend class Editeur;
public:
	struct Entite {
		Entite(ElementNiveau *e, bool entiteExterieure) : _entite(e), _exterieure(entiteExterieure) {
			
		}
		
		bool operator==(Entite const &e) const {
			return _entite == e._entite;
		}
		bool operator==(ElementNiveau const *e) const {
			return _entite == e;
		}
		
		ElementNiveau *_entite;
		bool _exterieure;
	};
	
	enum couche_t {premiereCouche, cn_sol = premiereCouche, cn_sol2, cn_objetsInventaire, cn_objet, nbCouches};
	typedef std::list<Entite> elements_t;
	typedef std::pair<elements_t::iterator, elements_t::iterator> listeElements_t;
	typedef std::pair<elements_t::const_iterator, elements_t::const_iterator> const_listeElements_t;
	
	class Exc_CreationNiveau : public std::exception {
	public:
		Exc_CreationNiveau(std::string const &v) throw() : std::exception(), _valeur(v) {
			std::cerr << this->what() << std::endl;
		}
		virtual ~Exc_CreationNiveau() throw() { }
		virtual const char* what() const throw() { return _valeur.c_str(); }
	private:
		std::string _valeur;
	};
		
	static ElementNiveau * const aucunElement;

	Niveau(Joueur *j, std::string const &nomFichier);
	virtual ~Niveau();
	
	Audio::audio_t musique();
	Audio::audio_t attaque();
	
	Joueur *joueur();
	
	// Dimensions du niveau en nombre de cases
	size_t dimX() const;
	size_t dimY() const;
	
	// Obtention du contenu d'une case en un point donné. Si ce point est en dehors du niveau, retourne Niveau::aucunElement.
	listeElements_t elements(index_t x, index_t y, couche_t couche);
	const_listeElements_t elements(index_t x, index_t y, couche_t couche) const;
	bool collision(index_t x, index_t y, couche_t couche, ElementNiveau *e) const;
	
	ssize_t monnaie(index_t x, index_t y) const;
	void modifierMonnaie(index_t x, index_t y, ssize_t delta);

	void ajouterElement(index_t x, index_t y, couche_t couche, ElementNiveau *elem);
	void supprimerElement(ElementNiveau *e, index_t x, index_t y, couche_t couche, bool deleteElement);
	elements_t::iterator supprimerElement(elements_t::iterator i, index_t x, index_t y, couche_t couche, bool deleteElement);
	
	void notifierDeplacement(EntiteMobile *e, index_t ancienX, index_t ancienY, couche_t ancienneCouche);
	
	double zoom() const;
	void definirZoom(double z);
	
	void animer();
	void afficher();
	
	static Unichar nomCouche(couche_t couche);
	static char const *nomBaliseCouche(couche_t couche);
		
protected:
	Niveau(Joueur *p, Niveau const &niveau);
	Niveau &operator=(Niveau const &);
	
	void afficherObjetsInventaire(glm::vec2 const &cam);
	void afficherCouche(couche_t couche, glm::vec2 const &cam);
	void afficherTransitionsSol(glm::vec2 const &cam);
	void afficherBordure(int cote, glm::vec2 const &cam);
	
	void definirContenuCase(index_t x, index_t y, couche_t couche, ElementNiveau *e);
	
	void allocationCases();
	void remplissageBordures();
	void remplissageTransitionsSol();
	void transitionSol(index_t x, index_t y, int position);
	
	ssize_t longueurBordure(int cote);
	static ssize_t epaisseurBordure();
	static bool collision(couche_t couche);
	void definirJoueur(Joueur *j);

private:
	struct TransitionSol {
		TransitionSol() : _element(0), _dim(1), _exterieure(true) { 
			
		}
		
		ElementNiveau *_element;
		bool _exterieure;
		size_t _dim;
	};
	struct Case {
		Case();
		
		ssize_t _monnaie;
		elements_t _entites[nbCouches];
		TransitionSol _transitions[3];
	};
	struct CaseMobile {
		CaseMobile(EntiteMobile *e) : _e(e), _pos(0) {
			
		}
		
		EntiteMobile *_e;
		Case *_pos;
		
		
		bool operator==(CaseMobile const &e) const {
			return _e == e._e;
		}
		bool operator==(EntiteMobile const *e) const {
			return _e == e;
		}
	};

	Case **_elements;
	ElementNiveau *_solBordures;
	ElementNiveau **_bordures[4];	
	TransitionSol *_transitionsBordures[2];
	TransitionSol *_anglesTransitionsBordures[2];

	size_t _dimX, _dimY;
	double _zoom;
	std::list<CaseMobile> _entitesMobiles;
	Joueur *_perso;
	Image _objet;
	Image _objets;
	Image _monnaie;
	
	Audio::audio_t _tonnerre;
	Audio::audio_t _musique;
	Audio::audio_t _attaque;
	Audio::audio_t _sonPluie;
	
	glm::vec2 _persoInit;
	Shader _pluie;
	Shader _transitionSol;
	
	std::list<ElementNiveau *> _aEffacer;
};

Niveau::couche_t &operator++(Niveau::couche_t &c);
Niveau::couche_t operator+(Niveau::couche_t, int i);

#endif
