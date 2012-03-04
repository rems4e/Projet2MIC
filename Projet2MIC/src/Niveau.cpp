//
//  Niveau.cpp
//  Projet2MIC
//
//  Created by Rémi Saurel on 03/02/12.
//  Copyright (c) 2012 Rémi Saurel. All rights reserved.
//

#include "Niveau.h"
#include "ElementNiveau.h"
#include "EntiteMobile.h"
#include "EntiteStatique.h"
#include "Session.h"
#include "fonctions.h"
#include "Personnage.h"
#include "Joueur.h"
#include <cmath>
#include <cstring>
#include "tinyxml.h"

#define NB_VALEURS_PROBA_ENTITES 16

/*class GenerateurElementAleatoire {
public:
	class Exc_ProbaInvalide : public std::exception {
	public:
		Exc_ProbaInvalide() : std::exception() { }
		virtual ~Exc_ProbaInvalide() throw() { }
		char const *what() const throw() { return "Proba invalide"; }
	};
	
	GenerateurElementAleatoire(std::string const &s) throw(Exc_ProbaInvalide) : _probasCumulees() {
		if(s.size() != ElementNiveau::nbTypesElement)
			throw Exc_ProbaInvalide();
		int total = 0;
		for(ElementNiveau::elementNiveau_t e = ElementNiveau::premierTypeElement; e != ElementNiveau::nbTypesElement; ++e) {
			_probasCumulees[e] = total + caractereVersHexa(s[e]);
		}
	}
	
	ElementNiveau *operator()(Niveau *n) const {
		int nbAlea = nombreAleatoire(_probasCumulees[sizeof(_probasCumulees) - 1]);
		for(ElementNiveau::elementNiveau_t e = ElementNiveau::premierTypeElement; e != ElementNiveau::nbTypesElement; ++e) {
			if(nbAlea < _probasCumulees[e]) {
				ElementNiveau *retour = ElementNiveau::elementNiveau(n, nombreAleatoire(ElementNiveau::nombreEntites(e)), e);
				return retour;
			}
		}
		
		return 0;
	}
	
	virtual ~GenerateurElementAleatoire() { }
	
private:
	int _probasCumulees[ElementNiveau::nbTypesElement];
};
*/
Niveau::couche_t &operator++(Niveau::couche_t &c) { return c = static_cast<Niveau::couche_t>(static_cast<int>(c) + 1); }
Niveau::couche_t operator+(Niveau::couche_t c, int i) { return static_cast<Niveau::couche_t>(static_cast<int>(c) + 1); }

ElementNiveau * const Niveau::aucunElement = 0;

Niveau::Case::Case() : _entites(), _entiteExterieure() {
	std::memset(_entites, 0, nb_couches * sizeof(EntiteStatique *));
	std::memset(_entiteExterieure, 0, nb_couches * sizeof(bool));
}

Niveau::Niveau(Joueur *j, std::string const &nomFichier) : _elements(0), _dimX(0), _dimY(0), _zoom(1.0), _entitesMobiles(), _perso(j), _bordures() {	
	TiXmlDocument niveau(Session::cheminRessources() + nomFichier);
	if(!niveau.LoadFile())
		std::cout << "Erreur de l'ouverture du fichier de niveau (" << (Session::cheminRessources() + nomFichier) << "." << std::endl;

	TiXmlElement *n = niveau.FirstChildElement("Niveau");
	n->Attribute("dimX", &_dimX);
	n->Attribute("dimY", &_dimY);
	
	this->init();
	this->remplissageBordures();
	
	TiXmlElement *probas = n->FirstChildElement("proba");
	
	for(couche_t couche = premierCouche; couche < nb_couches; ++couche) {
		TiXmlElement *cc = n->FirstChildElement(std::string("couche") + nombreVersTexte(couche));
		if(!cc)
			continue;
		char const *texte = cc->GetText();
		size_t tailleAttendue = _dimY * (_dimX * 4 + 1) - 1; // chaque ligne contient 4 chiffres hexa par colonne et un saut de ligne, sauf pour la dernière ligne.
		if(!texte || std::strlen(texte) != tailleAttendue) {
			throw Exc_CreationNiveau(std::string() + "Erreur : dimensions de la couche " + nombreVersTexte(couche) + " invalides (" + nombreVersTexte(std::strlen(texte)) + " au lieu de " + nombreVersTexte(tailleAttendue) +  " attendus) !");
		}

		index_t pos = 0;
		for(index_t y = 0; y < _dimY; ++y) {
			for(index_t x = 0; x < _dimX; ++x, pos += 4) {
				uint16_t valeur = (caractereVersHexa(texte[pos]) << 12) + (caractereVersHexa(texte[pos + 1]) << 8) + (caractereVersHexa(texte[pos + 2]) << 4) + caractereVersHexa(texte[pos + 3]);
				
				ElementNiveau *e = 0;
				if(valeur & (1 << 15)) { // Bit de poids fort à 1 -> valeur déterminée de la case
					ElementNiveau::elementNiveau_t categorie = ElementNiveau::nbTypesElement;
					index_t index = 0;
					categorie = static_cast<ElementNiveau::elementNiveau_t>((valeur & 0x7E00) >> 9);
					index = valeur & 0x1FF;
					try {
						e = ElementNiveau::elementNiveau(this, index, categorie);
					}
					catch(ElementNiveau::Exc_EntiteIndefinie &e) {
						std::cerr << "L'entité (cat : " << categorie << " ; index : " << index << ") appelée dans le fichier de niveau \"" << nomFichier << "\" (x : " << x << " ; y : " << y << ") n'est pas définie !" << std::endl;
						throw;
					}
				}
				else { // Bit de poids fort à 0 -> valeur probabiliste de la case
					if(valeur == 0) { // Aucun entité présente dans la case
						
					}
					else { // TODO: proba !
						
					}
					/*index_t probaCumulee = 0;
					index_t nbAlea = nombreAleatoire(NB_VALEURS_PROBA_ENTITES * ElementNiveau::nbTypesElement);
					for(ElementNiveau::elementNiveau_t i = ElementNiveau::premierTypeElement; i != ElementNiveau::nbTypesElement; ++i) {
						index_t proba = (v / (NB_VALEURS_PROBA_ENTITES * i)) % NB_VALEURS_PROBA_ENTITES;
						probaCumulee += proba;
						if(probaCumulee >= nbAlea) {
							
							break;
						}
					}*/
				}

				if(e) {
					this->definirContenuCase(x, y, couche, e);
					_elements[y][x]._entites[couche]->definirPosition(Coordonnees(x, y));
				}
			}
			
			// Saut de ligne.
			pos += 1;
		}
	}
}

Niveau::Niveau(Joueur *j) : _elements(0), _dimX(0), _dimY(0), _zoom(1.0), _entitesMobiles(), _perso(j), _bordures() {	
	j->definirPosition(Coordonnees(200, 100));
	
	_dimX = 160;
	_dimY = 160;
	
	this->init();
	this->remplissageBordures();

	// Remplissage du terrain avec un sol uni et des arbres disposés aléatoirement
	for(size_t y = 0; y < _dimY; ++y) {
		for(size_t x = 0; x < _dimX; ++x) {
			Case &c = _elements[y][x];
			if(c._entites[cn_sol] == 0) {
				ElementNiveau *e = ElementNiveau::elementNiveau(this, 22, ElementNiveau::entiteStatique);
				this->definirContenuCase(x, y, cn_sol, e);
			}
			if(c._entites[cn_objet] == 0) {
				int nb = nombreAleatoire(100);
				if(nb < 10) {
					EntiteStatique *e = ElementNiveau::elementNiveau<EntiteStatique>(this, nb);
					this->definirContenuCase(x, y, cn_objet, e);
				}
			}
			for(couche_t couche = premierCouche; couche != nb_couches; ++couche) {
				if(c._entites[couche] && !c._entiteExterieure[couche]) {
					c._entites[couche]->definirPosition(Coordonnees(x, y));
				}
			}
		}
	}
}

void Niveau::init() {
	// Allocation des cases du niveau
	_elements = new Case*[_dimY];
	for(size_t y = 0; y < _dimY; ++y) {
		_elements[y] = new Case[_dimX];
	}

	// On dimensionne les 4 tableaux servant à afficher la bordure, en fonction de la taille du niveau
	for(int cote = 0; cote < 4; ++cote) {
		_bordures[cote] = new Case *[Niveau::epaisseurBordure()];
		ssize_t dim = ((cote == GAUCHE || cote == DROITE) ? _dimX + 2 * Niveau::epaisseurBordure() : _dimY);
		for(size_t i = 0; i < Niveau::epaisseurBordure(); ++i) {
			_bordures[cote][i] = new Case[dim];
		}
	}
}

void Niveau::remplissageBordures() {
	index_t idSol = 22;
	EntiteStatique *solBordure = ElementNiveau::elementNiveau<EntiteStatique>(this, idSol);
	size_t dimSol = solBordure->dimensions().x / LARGEUR_CASE;
	delete solBordure;
	for(int cote = 0; cote < 4; ++cote) {
		ssize_t dim = ((cote == GAUCHE || cote == DROITE) ? _dimX + 2 * Niveau::epaisseurBordure() : _dimY);
		for(index_t i = 0; i < Niveau::epaisseurBordure(); ++i) {
			for(index_t j = 0; j < dim; ++j) {
				int nb = nombreAleatoire(10);
				EntiteStatique *e = ElementNiveau::elementNiveau<EntiteStatique>(this, nb);
				_bordures[cote][i][j]._entites[cn_objet] = e;
				if(((cote == GAUCHE || cote == DROITE) && (i % dimSol == 0 && (Niveau::epaisseurBordure() - j - 1) % dimSol == 0)) || ((cote == HAUT || cote == BAS) && ((Niveau::epaisseurBordure() - i - 1) % dimSol == 0 && j % dimSol == 0))) {
					e = ElementNiveau::elementNiveau<EntiteStatique>(this, idSol);
					_bordures[cote][i][j]._entites[cn_sol] = e;
				}
				
				for(couche_t c = premierCouche; c != nb_couches; ++c) {
					ElementNiveau *ee = _bordures[cote][i][j]._entites[c];
					if(!ee)
						continue;
					switch(cote) {
						case GAUCHE:
							ee->definirPosition(Coordonnees(dim - j - Niveau::epaisseurBordure() - 1, -(Niveau::epaisseurBordure() - i)));
							break;
						case DROITE:
							ee->definirPosition(Coordonnees(dim - j - Niveau::epaisseurBordure() - 1, _dimY + i));
							break;
						case HAUT:
							ee->definirPosition(Coordonnees(_dimX + Niveau::epaisseurBordure() - i - 1, j));
							break;
						case BAS:
							ee->definirPosition(Coordonnees(-i - 1, j));
							break;
					}
				}
			}
		}
	}
}

ssize_t Niveau::epaisseurBordure() {
	return 16;
}

Niveau::~Niveau() {
	for(uindex_t y = 0; y != this->dimY(); ++y) {
		for(uindex_t x = 0; x != this->dimX(); ++x) {
			for(Niveau::couche_t c = premierCouche; c != nb_couches; ++c) {
				if(!_elements[y][x]._entiteExterieure[c])
					delete _elements[y][x]._entites[c];
			}
		}
		delete _elements[y];
	}
	delete _elements;
	
	for(int cote = 0; cote < 4; ++cote) {
		for(size_t i = 0; i < Niveau::epaisseurBordure(); ++i) {
			ssize_t dim = ((cote == GAUCHE || cote == DROITE) ? _dimX + 2 * Niveau::epaisseurBordure() : _dimY);
			for(uindex_t j = 0; j != dim; ++j) {
				for(Niveau::couche_t c = premierCouche; c != nb_couches; ++c) {
					delete _bordures[cote][i][j]._entites[c];
				}
			}
			delete _bordures[cote][i];
		}
		delete _bordures[cote];
	}
}

ElementNiveau const *Niveau::element(index_t x, index_t y, Niveau::couche_t couche) const {
	if(x < 0 || y < 0 || x >= this->dimX() || y >= this->dimY()) {
		return Niveau::aucunElement;
	}
	
	return _elements[y][x]._entites[couche];
}

/*ElementNiveau *Niveau::element(index_t x, index_t y, Niveau::couche_t couche) {
	return (static_cast<Niveau const *>(this))->element(x, y, couche);
}*/

bool Niveau::collision(index_t x, index_t y) const {
	for(couche_t c = premierCouche; c < nb_couches; ++c) {
		ElementNiveau const *e = this->element(x, y, c);
		if(e && e->collision())
			return true;
	}
	
	return false;
}

size_t Niveau::dimX() const {
	return _dimX;
}

size_t Niveau::dimY() const {
	return _dimY;
}

double Niveau::zoom() const {
	return _zoom;
}

void Niveau::definirZoom(double z) {
	_zoom = z;
	if(_zoom == 0.0) {
		_zoom = 1.0;
	}
}

void Niveau::animer(horloge_t tempsEcoule) {
	/*if(Session::evenement(Session::T_a)) {
		this->definirZoom(this->zoom() * 1.2);
	}
	if(Session::evenement(Session::T_z)) {
		this->definirZoom(this->zoom() / 1.2);
	}*/
	
	for(couche_t c = premierCouche; c != nb_couches; ++c) {
		for(size_t y = 0; y < _dimY; ++y) {
			for(size_t x = 0; x < _dimX; ++x) {			
				ElementNiveau *entite = _elements[y][_dimX - x - 1]._entites[c];
				if(entite != 0 && !_elements[y][_dimX - x - 1]._entiteExterieure[c]) {
					entite->animer(tempsEcoule);
				}
			}
		}
	}

	_perso->animer(tempsEcoule);
}

void Niveau::afficher() {
	index_t persoX = std::floor((_perso->position().x - _perso->origine().x) / LARGEUR_CASE), persoY = std::floor((_perso->position().y -  _perso->origine().y) / LARGEUR_CASE);
	Coordonnees cam = _perso->positionAffichage() - (Ecran::dimensions() - _perso->dimensions()) / 2;
	cam.x = std::floor(cam.x);
	cam.y = std::floor(cam.y);

	this->afficherCouche(cn_sol, cam, persoX, persoY);

	this->afficherBordure(GAUCHE, cam);
	this->afficherBordure(HAUT, cam);
	
	for(Niveau::couche_t c = premierCouche + 1; c != nb_couches; ++c) {
		this->afficherCouche(c, cam, persoX, persoY);
	}
	
	this->afficherBordure(BAS, cam);
	this->afficherBordure(DROITE, cam);
}

void Niveau::afficherCouche(couche_t c, Coordonnees const &cam, index_t persoX, index_t persoY) {
	for(size_t y = 0; y < _dimY; ++y) {
		for(size_t x = 0; x < _dimX; ++x) {			
			if(y == persoY && (_dimX - x - 1) == persoX && c == nb_couches - 1) {
				_perso->afficher(cam * this->zoom(), this->zoom());
			}
			ElementNiveau *entite = _elements[y][_dimX - x - 1]._entites[c];
			if(entite != 0 && !_elements[y][_dimX - x - 1]._entiteExterieure[c]) {
				entite->afficher(cam * this->zoom(), this->zoom());
			}
		}
	}
}

void Niveau::afficherBordure(int cote, Coordonnees const &cam) {
	ssize_t dim = ((cote == GAUCHE || cote == DROITE) ? _dimX + 2 * Niveau::epaisseurBordure() : _dimY);
	for(Niveau::couche_t c = premierCouche; c != nb_couches; ++c) {
		for(uindex_t i = 0; i < Niveau::epaisseurBordure(); ++i) {
			for(uindex_t j = 0; j < dim; ++j) {
				if(_bordures[cote][i][j]._entites[c] && !_bordures[cote][i][j]._entiteExterieure[c]) {
					_bordures[cote][i][j]._entites[c]->afficher(cam * this->zoom(), this->zoom());
				}
			}
		}
	}
}


void Niveau::definirContenuCase(index_t x, index_t y, couche_t couche, ElementNiveau *e) {
	if(e->multi()) {
		size_t dimX = e->dimensions().x / LARGEUR_CASE, dimY = e->dimensions().y / HAUTEUR_CASE;
		for(index_t yy = y; yy < std::min(dimY + y, _dimY); ++yy) {
			for(index_t xx = x; xx < std::min(dimX + x, _dimX); ++xx) {
				_elements[yy][xx]._entites[couche] = e;
				_elements[yy][xx]._entiteExterieure[couche] = (yy != y) || (xx != x);
			}
		}
	}
	else {
		_elements[y][x]._entites[couche] = e;
	}
}

void Niveau::definirJoueur(Joueur *j) {
	if(_perso) {
		j->definirPosition(_perso->position());
		j->definirDirection(_perso->direction());
	}
	_perso = j;
}


