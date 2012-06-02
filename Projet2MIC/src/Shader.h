//
//  Shader.h
//  Projet2MIC
//
//  Créé par Marc Promé et Rémi Saurel.
//  Ce fichier et son contenu sont librement distribuables, modifiables et utilisables pour toute œuvre non commerciale, à condition d'en citer les auteurs.
//

#ifndef Projet2MIC_Shader_h
#define Projet2MIC_Shader_h

#include "Constantes.h"
#include <exception>
#include <map>
#include <string>
#include "Geometrie.h"
#include "Ecran.h"

#ifdef __MACOSX__
#include <OpenGL/gl.h>
#else
#define GL_GLEXT_LEGACY
#include <GL/gl.h>
#define GL_GLEXT_PROTOTYPES
#include <GL/glext.h>
#endif

namespace Session {
	void nettoyer();
	void initialiser();
}

class Shader {
	friend void Session::nettoyer();
	friend void Session::initialiser();
	friend void Ecran::modifierResolution(unsigned int largeur, unsigned int hauteur, bool pleinEcran) throw(Ecran::Exc_InitialisationImpossible);

public:
	static char const * const rayonFlou;
	static char const * const dim;
	static char const * const pos;
	static char const * const temps;
	static char const * const tempsAbsolu;
		
	class Exc_CreationImpossible : public std::exception {
	public:
		Exc_CreationImpossible() throw() : std::exception() {
			std::cerr << this->what() << std::endl;
		}
		virtual ~Exc_CreationImpossible() throw() { }
		virtual const char* what() const throw() { return "Création shader impossible"; }
	};

	Shader(std::string const &vert, std::string const &frag) throw(Exc_CreationImpossible);
	~Shader();
		
	static Shader const &aucun();
	
	static Shader const &flou(float rayon);
	
	void definirParametre(char const *param, float v) const;
	void definirParametre(char const *param, float v1, float v2) const;
	void definirParametre(char const *param, float v1, float v2, float v3) const;
	void definirParametre(char const *param, float v1, float v2, float v3, float v4) const;

	void activer() const;
	
	GLint vertCoord() const { return _prog->_vertCoord; }
	GLint texCoord() const { return _prog->_texCoord; }
	GLint coul() const { return _prog->_coul; }
	
	static void desactiver();
	
	static Shader const &shaderActuel();
	static Coordonnees versShader(Coordonnees const &);
	static Rectangle versShader(Rectangle const &);
		
protected:
	static void compiler(GLuint shader) throw(Exc_CreationImpossible);
	static void lier(GLint prog) throw(Exc_CreationImpossible);
	
	static GLint chargerShader(std::string const &chemin, GLint type) throw(Exc_CreationImpossible);
	
	GLint locParam(char const *param) const;

private:
	struct SousProgramme;
	struct Programme {
		GLint _id;
		SousProgramme *_vert, *_frag;

		GLint _vertCoord, _texCoord, _coul;
		
		size_t _ref;
	};
	
	struct SousProgramme {
		GLint _id;
		size_t _ref;
		
		std::string _fichier;
	};

	Programme *_prog;
				
	mutable std::map<char const *, GLint> _parametres;

	static std::map<std::pair<std::string, std::string>, Programme> *_programmes;
	static std::map<std::string, SousProgramme> *_vertexShaders;
	static std::map<std::string, SousProgramme> *_fragmentShaders;
	static Shader *_flou;
	static Shader *_aucun;
	
	static void preReinitialiser();
	static void initialiser();
	static void reinitialiser() throw(Exc_CreationImpossible);
	static void nettoyer();
	
	static bool init();

	static Shader const *_shaderActuel;
	
	Shader &operator=(Shader const &);
	Shader(Shader const &);
};

#endif
