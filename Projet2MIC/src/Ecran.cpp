//
//  Ecran.cpp
//  Projet2MIC
//
//  Créé par Marc Promé et Rémi Saurel.
//  Ce fichier et son contenu sont librement distribuables, modifiables et utilisables pour toute œuvre non commerciale, à condition d'en citer les auteurs.
//

#include "Ecran.h"
#include "Constantes.h"

#include "SDL/SDL.h"
#include "Texte.h"
#include "Image.h"
#include "Session.h"
#include <limits>
#include <cassert>
#include <algorithm>
#include "Shader.h"
#include <stack>
#include <cmath>
#include "Affichage.h"

#ifdef __MACOSX__
#include <OpenGL/gl.h>
#elif defined(__WIN32__)
#include <GL/glew.h>
#else
#define GL_GLEXT_LEGACY
#include <GL/gl.h>
#define GL_GLEXT_PROTOTYPES
#include <GL/glext.h>
#endif

#define TAILLE_IPS 255

Couleur teinte = Couleur(255, 255, 255);

Couleur const Couleur::noir(0);
Couleur const Couleur::gris(128);
Couleur const Couleur::grisClair(200);
Couleur const Couleur::grisFonce(80);
Couleur const Couleur::blanc(255);
Couleur const Couleur::rouge(255, 0, 0);
Couleur const Couleur::vert(0, 255, 0);
Couleur const Couleur::bleu(0, 0, 255);
Couleur const Couleur::jaune(255, 255, 0);
Couleur const Couleur::transparent(0, 0);

namespace TexturePrive {
	void reinitialiser();
	void preReinitialiser();
}

namespace Ecran {
	struct AttributsEcran {
		Texte *_texte;
		Texture *_vide;
		Image const *_pointeur;
		Image const *_pointeurDefaut;
		glm::vec2 _decalagePointeur;
		bool _pointeurAffiche;
		Unichar _bulleAide;
		
		glm::ivec2 _dimensions;
		bool _pleinEcran;
						
		float _frequence;
		float _frequenceInstantanee;
		
		glm::vec2 _echelle;
		coordonnee_t _echelleMin;
		
		std::stack<Rectangle> _cadresAffichage;
		
		std::stack<size_t> _verrousTransformation;
		std::stack<glm::mat4> _modeleVue;
		glm::mat4 _perspective;

		AttributsEcran();
		~AttributsEcran();
		
	private:
		AttributsEcran(AttributsEcran const &);
		AttributsEcran &operator=(AttributsEcran const &);
	};
	
	AttributsEcran *_attributs;
	
	// Met à jour l'affichage en fonction des éléments affichés depuis le dernier appel à la fonction
	void maj();
	void finaliser();
	// Remplis l'écran avec une couleur unie
	void effacer();

	void init(unsigned int largeur, unsigned int hauteur, bool pleinEcran);
	void nettoyagePreliminaire();
	void nettoyageFinal();
	
	void majVue(int largeur, int hauteur);
	
	void perspective(float angle, float near, float far, bool majShader);
	
	void ajouterVerrouTransformations();
	// On ne doit pas supprimer la première case de la pile.
	void supprimerVerrouTransformations() throw(int);
	void retourVerrouTransformations();
}

void Ecran::init(unsigned int largeur, unsigned int hauteur, bool pleinEcran) {
	_attributs = new AttributsEcran;
	Ecran::_attributs->_modeleVue.push(glm::mat4(1.0f));
	Ecran::ajouterVerrouTransformations();
	
	Ecran::modifierResolution(largeur, hauteur, pleinEcran);

	Ecran::_attributs->_vide = new Texture(Session::cheminRessources() + "vide.png");
	Ecran::_attributs->_pointeurDefaut = new Image(Session::cheminRessources() + "souris.png");
	Ecran::_attributs->_texte = new Texte("", POLICE_NORMALE, 12, Couleur::blanc);

	Ecran::definirPointeur(0);
	SDL_ShowCursor(SDL_DISABLE);
}

Ecran::AttributsEcran::AttributsEcran() : _dimensions(0), _pleinEcran(false), _frequence(1.0f), _frequenceInstantanee(1.0f), _texte(0), _pointeur(0), _pointeurDefaut(0), _decalagePointeur(), _pointeurAffiche() {

}

void Ecran::nettoyagePreliminaire() {
	delete _attributs->_pointeurDefaut;
	delete _attributs->_texte;
	delete _attributs->_vide;
}

void Ecran::nettoyageFinal() {
	delete _attributs;
}

Ecran::AttributsEcran::~AttributsEcran() {
	SDL_ShowCursor(SDL_ENABLE);

	if(_pleinEcran) {
		SDL_SetVideoMode(800, 600, 32, SDL_OPENGL | SDL_ASYNCBLIT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		SDL_GL_SwapBuffers();
	}
}

void Ecran::modifierResolution(unsigned int largeur, unsigned int hauteur, bool pleinEcran) throw(Ecran::Exc_InitialisationImpossible) {
	if(Shader::init()) {
		TexturePrive::preReinitialiser();
		Shader::preReinitialiser();
	}

	Ecran::_attributs->_dimensions.x = largeur;
	Ecran::_attributs->_dimensions.y = hauteur;
	Ecran::_attributs->_pleinEcran = pleinEcran;
	
	Ecran::_attributs->_frequence = 0.0f;
	Ecran::_attributs->_pointeurAffiche = false;
	
	Ecran::_attributs->_echelle = glm::vec2(Ecran::_attributs->_dimensions.x / 800.0f, Ecran::_attributs->_dimensions.y / 600.0f);
	Ecran::_attributs->_echelleMin = std::min(Ecran::_attributs->_echelle.x, Ecran::_attributs->_echelle.y);

	while(Ecran::_attributs->_cadresAffichage.size()) {
		Ecran::_attributs->_cadresAffichage.pop();
	}
	Ecran::_attributs->_cadresAffichage.push(Rectangle(glm::vec2(0), glm::vec2(Ecran::dimensions())));

	while(Ecran::_attributs->_modeleVue.size() > 1) {
		Ecran::_attributs->_modeleVue.pop();
	}
	Ecran::perspective(70, 0.1, 100, false);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, Parametres::synchroVerticale());
	
	SDL_Surface *resultat = 0;
	if(pleinEcran)
		resultat = SDL_SetVideoMode(largeur, hauteur, 32, SDL_OPENGL | SDL_FULLSCREEN);
	else
		resultat = SDL_SetVideoMode(largeur, hauteur, 32, SDL_OPENGL);
	SDL_WM_SetCaption("", NULL);
	
#ifdef __WIN32__
	GLenum err = glewInit();
	if(err != GLEW_OK) {
		std::cerr << "GlewInit : " << glewGetErrorString(err) << std::endl;
	}
#endif
	
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	SDL_GL_SwapBuffers();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	if(resultat == 0) {
		std::cerr << "Impossible de définir l'écran à une résolution de " << largeur << "*" << hauteur << "*" << 32 << " (plein écran : " << pleinEcran << "). Erreur : " << SDL_GetError() << std::endl;
		throw Exc_InitialisationImpossible();
	}
	
	if(!Shader::init()) {
		Shader::initialiser();
		Shader::aucun().activer();
	}
	else {
		Shader::reinitialiser();
		TexturePrive::reinitialiser();
	}
}

void Ecran::majVue(int largeur, int hauteur) {
	if(!Ecran::_attributs->_pleinEcran) {
		Ecran::modifierResolution(largeur, hauteur, false);
	}
}

inline bool ivec2Comp(glm::ivec2 const &v1, glm::ivec2 const &v2) {
	return v1.x < v2.x || (v1.x == v2.x && v1.y < v2.y);
}

std::list<glm::ivec2> Ecran::resolutionsDisponibles(bool pleinEcran) {
	std::list<glm::ivec2> retour;

	Uint32 flags = SDL_OPENGL;
	if(pleinEcran)
		flags |= SDL_FULLSCREEN;
	
	SDL_Rect **liste = SDL_ListModes(0, flags);
	if(liste == 0) {
		std::cerr << "Aucune résolution d'écran disponible (plein écran : " << pleinEcran << "). Plantage attendu…" << std::endl;
		retour.push_back(glm::ivec2(640, 480));
	}
	else if(liste == reinterpret_cast<SDL_Rect **>(-1)) {
		retour.push_back(glm::ivec2(640, 400));
		retour.push_back(glm::ivec2(640, 480));
		retour.push_back(glm::ivec2(720, 480));
		retour.push_back(glm::ivec2(800, 500));
		retour.push_back(glm::ivec2(800, 600));
		retour.push_back(glm::ivec2(1024, 640));
		retour.push_back(glm::ivec2(1024, 768));
		retour.push_back(glm::ivec2(1152, 720));
		retour.push_back(glm::ivec2(1280, 720));
		retour.push_back(glm::ivec2(1280, 800));
		retour.push_back(glm::ivec2(1280, 960));
		retour.push_back(glm::ivec2(1280, 1024));
		retour.push_back(glm::ivec2(1344, 840));
		retour.push_back(glm::ivec2(1360, 850));
		retour.push_back(glm::ivec2(1440, 900));
		retour.push_back(glm::ivec2(1600, 1200));
		retour.push_back(glm::ivec2(1680, 1050));
		retour.push_back(glm::ivec2(1920, 1080));
		retour.push_back(glm::ivec2(1920, 1200));
		retour.push_back(glm::ivec2(2048, 1280));
		retour.push_back(glm::ivec2(2048, 1536));
		retour.push_back(glm::ivec2(2560, 1440));
		retour.push_back(glm::ivec2(2560, 1600));
	}
	else {
		for(int i = 0; liste[i]; ++i) {
			retour.push_back(glm::ivec2(liste[i]->w, liste[i]->h));
		}
		
		retour.sort(&ivec2Comp);
	}
	
	return retour;
}

Image *Ecran::apercu() {
	GLubyte *pixels = new GLubyte[Ecran::_attributs->_dimensions.x * Ecran::_attributs->_dimensions.y * 4];

	glReadPixels(0, 0, Ecran::_attributs->_dimensions.x, Ecran::_attributs->_dimensions.y, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	for(int y = 0; y < Ecran::hauteur(); ++y) {
		for(int x = 0; x < Ecran::largeur(); ++x) {
			pixels[4 * (y * Ecran::largeur() + x) + 3] = 255;
		}
	}
	
	Image *retour = new Image(pixels, Ecran::_attributs->_dimensions.x, Ecran::_attributs->_dimensions.y, 4, true);
	
	delete[] pixels;
	
	return retour;
}

void Ecran::maj() {
	static horloge_t h = 0.0f, h1 = 0.0f;
	static size_t c = 0;
	++c;
	horloge_t f = horloge();
	Ecran::_attributs->_frequenceInstantanee = 1.0f / (f - h1);
	h1 = f;
	if(f - h >= 3.0f) {
		Ecran::_attributs->_frequence = c / (f - h);
		c = 0;
		h = f;
	}

	if(Parametres::ips()) {
		static char ips[TAILLE_IPS];
		
		// FIXME: Trad
		snprintf(ips, TAILLE_IPS, "%.2f s - %.1f ips", horloge(), Ecran::_attributs->_frequence);
		ips[TAILLE_IPS - 1] = 0;
		Ecran::_attributs->_texte->definir(ips);
		glm::vec2 posIps(Ecran::_attributs->_dimensions.x - Ecran::_attributs->_texte->dimensions().x - 2, Ecran::_attributs->_dimensions.y - Ecran::_attributs->_texte->dimensions().y - 2);
		Affichage::afficherRectangle(Rectangle(posIps - glm::vec2(2, 2), Ecran::_attributs->_texte->dimensions() + glm::vec2(4, 4)), Couleur::noir);
		Ecran::_attributs->_texte->definir(Couleur::blanc);
		Ecran::_attributs->_texte->afficher(posIps);
	}
	
	if(!Ecran::_attributs->_bulleAide.empty()) {
		Texte t(Ecran::_attributs->_bulleAide, POLICE_NORMALE, 12, Couleur::noir);
		glm::vec2 dim = glm::vec2(t.dimensions().x + 6, t.dimensions().y + 4);
		glm::vec2 pos = glm::vec2(Session::souris().x + Ecran::pointeur()->dimensions().x + (Ecran::pointeur()->dimensions().x - dim.x) / 2, Session::souris().y + Ecran::pointeur()->dimensions().y);
		pos.x = std::min(std::max<coordonnee_t>(pos.x, 5.0), Ecran::_attributs->_dimensions.x - 5 - dim.x);
		pos.y = std::min(std::max<coordonnee_t>(pos.y, 5.0), Ecran::_attributs->_dimensions.y - 5 - dim.y);
		
		Affichage::afficherRectangle(Rectangle(pos, dim), Couleur(248, 250, 189));
		Affichage::afficherLigne(pos, glm::vec2(pos.x + dim.x, pos.y), Couleur(128));
		Affichage::afficherLigne(glm::vec2(pos.x, pos.y + dim.y), glm::vec2(pos.x + dim.x, pos.y + dim.y), Couleur(128));
		Affichage::afficherLigne(glm::vec2(pos.x, pos.y), glm::vec2(pos.x, pos.y + dim.y), Couleur(128));
		Affichage::afficherLigne(glm::vec2(pos.x + dim.x, pos.y), glm::vec2(pos.x + dim.x, pos.y + dim.y), Couleur(128));
		t.afficher(glm::vec2(pos.x + 3, pos.y + 2));
		Ecran::_attributs->_bulleAide = "";
	}

	if(Ecran::_attributs->_pointeurAffiche && Ecran::_attributs->_pointeur) {
		Ecran::_attributs->_pointeur->afficher(Session::souris() - Ecran::_attributs->_decalagePointeur);
	}
	
	while(Ecran::_attributs->_cadresAffichage.size() > 1) {
		Ecran::_attributs->_cadresAffichage.pop();
	}

	while(Ecran::_attributs->_modeleVue.size() > 1) {
		Ecran::_attributs->_modeleVue.pop();
	}
	Ecran::finaliser();
		
	SDL_GL_SwapBuffers();
}

void Ecran::finaliser() {
	Affichage::changerTexture(-1);
	Shader::aucun().activer();
}

void Ecran::effacer() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

Rectangle Ecran::ecran() {
	return Rectangle(0, 0, Ecran::largeur(), Ecran::hauteur());
}

glm::ivec2 Ecran::dimensions() {
	return Ecran::_attributs->_dimensions;
}

int Ecran::largeur() {
	return Ecran::_attributs->_dimensions.x;
}

int Ecran::hauteur() {
	return Ecran::_attributs->_dimensions.y;
}

bool Ecran::pleinEcran() {
	return Ecran::_attributs->_pleinEcran;
}

float Ecran::frequence() {
	return Ecran::_attributs->_frequence;
}

float Ecran::frequenceInstantanee() {
	return Ecran::_attributs->_frequenceInstantanee;
}

bool Ecran::pointeurAffiche() {
	return Ecran::_attributs->_pointeurAffiche;
}

void Ecran::definirPointeurAffiche(bool af) {
	Ecran::_attributs->_pointeurAffiche = af;
}

Texture const &Ecran::vide() {
	return *Ecran::_attributs->_vide;
}

Image const *Ecran::pointeur() {
	return Ecran::_attributs->_pointeur;
}


void Ecran::definirPointeur(Image const *image, glm::vec2 const &decalage) {
	if(image == 0) {
		Ecran::_attributs->_pointeur = Ecran::_attributs->_pointeurDefaut;
		Ecran::_attributs->_decalagePointeur = glm::vec2(0);
	}
	else {
		Ecran::_attributs->_pointeur = image;
		Ecran::_attributs->_decalagePointeur = decalage;
	}
}

glm::vec2 const &Ecran::echelle() {
	return Ecran::_attributs->_echelle;
}

coordonnee_t Ecran::echelleMin() {
	return Ecran::_attributs->_echelleMin;
}

Rectangle const &Ecran::cadreAffichage() {
	return Ecran::_attributs->_cadresAffichage.top();
}

void Ecran::ajouterCadreAffichage(Rectangle const &r) {
	Affichage::changerTexture(-1);

	Ecran::_attributs->_cadresAffichage.push(r.intersection(Ecran::cadreAffichage()));
	Rectangle const &nouveau = Ecran::cadreAffichage();
	glViewport(nouveau.gauche, Ecran::hauteur() - nouveau.haut - nouveau.hauteur, nouveau.largeur, nouveau.hauteur);
}

void Ecran::supprimerCadreAffichage() {
	Affichage::changerTexture(-1);

	Ecran::_attributs->_cadresAffichage.pop();
	Rectangle const &nouveau = Ecran::cadreAffichage();
	glViewport(nouveau.gauche, Ecran::hauteur() - nouveau.haut - nouveau.hauteur, nouveau.largeur, nouveau.hauteur);
}

Unichar const &Ecran::bulleAide() {
	return Ecran::_attributs->_bulleAide;
}

void Ecran::definirBulleAide(Unichar const &txt) {
	Ecran::_attributs->_bulleAide = txt;
}

void Ecran::ajouterTransformation(glm::mat4 const &transfo) {
	Ecran::_attributs->_modeleVue.push(Ecran::_attributs->_modeleVue.top() * transfo);
	Shader::majModele();
}

void Ecran::supprimerTransformation() throw(int) {
	// Si on supprime plus que le verrou en place nous autorise…
	if(Ecran::_attributs->_modeleVue.size() <= Ecran::_attributs->_verrousTransformation.top())
		throw 0;
	
	Ecran::_attributs->_modeleVue.pop();
	Shader::majModele();
}

glm::vec4 Ecran::transformerVecteur(glm::vec4 const &vec) {
	return Ecran::_attributs->_modeleVue.top() * vec;
}

void Ecran::ajouterVerrouTransformations() {
	Ecran::_attributs->_verrousTransformation.push(Ecran::_attributs->_modeleVue.size());
}

void Ecran::supprimerVerrouTransformations() throw(int) {
	if(Ecran::_attributs->_verrousTransformation.size() <= 1)
		throw 0;
	Ecran::_attributs->_verrousTransformation.pop();
}

void Ecran::retourVerrouTransformations() {
	size_t nb = Ecran::_attributs->_verrousTransformation.top();
	while(Ecran::_attributs->_modeleVue.size() > nb) {
		Ecran::_attributs->_modeleVue.pop();
	}
	Shader::majModele();
}

glm::mat4 const &Ecran::projection() {
	return Ecran::_attributs->_perspective;
}

glm::mat4 const &Ecran::modeleVue() {
	return Ecran::_attributs->_modeleVue.top();
}

void Ecran::perspective(float angle, float near, float far) {
	Ecran::perspective(angle, near, far, true);
}

void Ecran::perspective(float angle, float near, float far, bool majShader) {
	Ecran::_attributs->_perspective = ::projection(angle, static_cast<float>(Ecran::largeur()) / static_cast<float>(Ecran::hauteur()), near, far);
	
	if(majShader)
		Shader::majProjection();
}
