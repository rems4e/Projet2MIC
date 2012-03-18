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
#include "UtilitaireNiveau.h"

#define NB_VALEURS_PROBA_ENTITES 64 * 64
#define BASE_VALEURS_PROBA_ENTITES 64
#define CHIFFRES_VALEURS_PROBA_ENTITES 2

class GenerateurElementAleatoire {
public:
	class Exc_ProbaInvalide : public std::exception {
	public:
		Exc_ProbaInvalide() : std::exception() { }
		virtual ~Exc_ProbaInvalide() throw() { }
		char const *what() const throw() { return "Proba invalide"; }
	};
	
	GenerateurElementAleatoire(std::string const &s) throw(Exc_ProbaInvalide) : _probasCumulees(), _nbNonNuls(0) {
		if(s.size() != ElementNiveau::nbTypesElement * CHIFFRES_VALEURS_PROBA_ENTITES)
			throw Exc_ProbaInvalide();
		int total = 0;
		for(ElementNiveau::elementNiveau_t e = ElementNiveau::premierTypeElement; e != ElementNiveau::nbTypesElement; ++e) {
			int val = caractereVersBase64(s[e * CHIFFRES_VALEURS_PROBA_ENTITES]) * BASE_VALEURS_PROBA_ENTITES + caractereVersBase64(s[e * CHIFFRES_VALEURS_PROBA_ENTITES + 1]);
			_probasCumulees[e] = total + val;
			_nbNonNuls += val != 0;
		}
	}
	
	ElementNiveau *operator()(Niveau *n) const {
		if(!_nbNonNuls)
			return 0;

		int nbAlea = nombreAleatoire(_nbNonNuls * NB_VALEURS_PROBA_ENTITES);
		for(ElementNiveau::elementNiveau_t e = ElementNiveau::premierTypeElement; e != ElementNiveau::nbTypesElement; ++e) {
			if(nbAlea < _probasCumulees[e]) {
				ElementNiveau *retour = ElementNiveau::elementNiveau(n, nombreAleatoire(static_cast<int>(ElementNiveau::nombreEntites(e))), e);
				return retour;
			}
		}

		return 0;
	}
	
	virtual ~GenerateurElementAleatoire() { }
	
private:
	int _probasCumulees[ElementNiveau::nbTypesElement];
	int _nbNonNuls;
};

Niveau::couche_t &operator++(Niveau::couche_t &c) { return c = static_cast<Niveau::couche_t>(static_cast<int>(c) + 1); }
Niveau::couche_t operator+(Niveau::couche_t c, int i) { return static_cast<Niveau::couche_t>(static_cast<int>(c) + 1); }

ElementNiveau * const Niveau::aucunElement = 0;

Niveau::Case::Case() : _entites(), _entiteExterieure() {
	std::memset(_entites, 0, nb_couches * sizeof(EntiteStatique *));
	std::memset(_entiteExterieure, 0, nb_couches * sizeof(bool));
}

Niveau::Niveau(Joueur *j, std::string const &nomFichier) : _elements(0), _dimX(0), _dimY(0), _zoom(1.0), _entitesMobiles(), _perso(), _bordures() {
	this->definirJoueur(j);
	
	TiXmlDocument niveau(Session::cheminRessources() + nomFichier);
	if(!niveau.LoadFile()) {
		throw Exc_CreationNiveau(std::string() + "Erreur de l'ouverture du fichier de niveau (" + (Session::cheminRessources() + nomFichier) + ".");
	}

	TiXmlElement *n = niveau.FirstChildElement("Niveau");
	n->Attribute("dimX", &_dimX);
	n->Attribute("dimY", &_dimY);
	
	if(_dimX <= 0 || _dimY <= 0) {
		throw Exc_CreationNiveau("Dimensions du niveau invalides !");
	}
	
	this->allocationCases();
	this->remplissageBordures();
	
	GenerateurElementAleatoire **generateurs = 0;
	size_t nbGenerateurs = 0;
	{
		TiXmlElement *probas = n->FirstChildElement("proba");
		for(TiXmlNode *proba = probas->FirstChild(); proba; proba = proba->NextSibling(), ++nbGenerateurs);
		generateurs = new GenerateurElementAleatoire *[nbGenerateurs];
		TiXmlElement *proba = probas->FirstChildElement();
		for(index_t i = 0; i != nbGenerateurs; ++i, proba = proba->NextSiblingElement()) {
			if(std::string(proba->Value()) != "proba") {
				throw  Exc_CreationNiveau(std::string() + "Erreur : élément de proba " + nombreVersTexte(i) + " invalide.");
			}
			
			std::string valeur = proba->Attribute("valeur");
			generateurs[i] = new GenerateurElementAleatoire(valeur);
		}
	}
	
	for(couche_t couche = premierCouche; couche < nb_couches; ++couche) {
		TiXmlElement *cc = n->FirstChildElement(Niveau::nomCouche(couche));
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
				ElementNiveau::elementNiveau_t categorie;
				index_t index;
				bool proba;
				index_t indexProba;
				obtenirInfosEntites(valeur, proba, indexProba, categorie, index);
				
				ElementNiveau *e = 0;
				if(!proba) { // Valeur déterminée de la case
					try {
						e = ElementNiveau::elementNiveau(this, index, categorie);
					}
					catch(ElementNiveau::Exc_EntiteIndefinie &e) {
						std::cerr << "L'entité (cat : " << categorie << " ; index : " << index << ") appelée dans le fichier de niveau \"" << nomFichier << "\" (couche : " << Niveau::nomCouche(couche) << " ; x : " << x << " ; y : " << y << ") n'est pas définie !" << std::endl;
						throw;
					}
				}
				else { // Bit de poids fort à 0 -> valeur probabiliste de la case
					if(indexProba == 0) { // Aucun entité présente dans la case
						
					}
					else { // choix d'une entité au hasard en fonction de la loi de probabilité indiquée
						--indexProba;
						if(indexProba >= nbGenerateurs) {
							throw Exc_CreationNiveau("L'index de la loi de probabilité demandé (" + nombreVersTexte(indexProba) + ") pour la case (x : " + nombreVersTexte(x) + " ; y : " + nombreVersTexte(y) + ") est invalide.");
						}
						e = generateurs[indexProba]->operator()(this);
					}
				}

				if(e) {
					this->definirContenuCase(x, y, couche, e);
					_elements[y][x]._entites[couche]->definirPosition(Coordonnees(x, y) * LARGEUR_CASE);
				}
			}
			
			// Saut de ligne.
			pos += 1;
		}
	}
	
	for(index_t i = 0; i != nbGenerateurs; ++i)
		delete generateurs[i];
	delete[] generateurs;
}

Niveau::Niveau(Joueur *j) : _elements(0), _dimX(0), _dimY(0), _zoom(1.0), _entitesMobiles(), _perso(j), _bordures() {	
	this->definirJoueur(j);
	
	_dimX = 160;
	_dimY = 160;
	
	this->allocationCases();
	this->remplissageBordures();

	// Remplissage du terrain avec un sol uni et des arbres disposés aléatoirement
	for(size_t y = 0; y < _dimY; ++y) {
		for(size_t x = 0; x < _dimX; ++x) {
			Case &c = _elements[y][x];
			if(c._entites[cn_sol] == 0) {
				ElementNiveau *e = ElementNiveau::elementNiveau(this, 2, ElementNiveau::entiteStatique);
				this->definirContenuCase(x, y, cn_sol, e);
			}
			if(c._entites[cn_objet] == 0) {
				int nb = nombreAleatoire(100);
				if(nb < 10) {
					EntiteStatique *e = ElementNiveau::elementNiveau<EntiteStatique>(this, nb, ElementNiveau::arbre);
					this->definirContenuCase(x, y, cn_objet, e);
				}
			}
			for(couche_t couche = premierCouche; couche != nb_couches; ++couche) {
				if(c._entites[couche] && !c._entiteExterieure[couche]) {
					c._entites[couche]->definirPosition(Coordonnees(x, y) * LARGEUR_CASE);
				}
			}
		}
	}
}

void Niveau::allocationCases() {
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
	
	_b1 = Image(Session::cheminRessources() + "bordure1.png");
	_b2 = Image(Session::cheminRessources() + "bordure2.png");
}

void Niveau::remplissageBordures() {
	index_t idSol = 2;
	EntiteStatique *solBordure = ElementNiveau::elementNiveau<EntiteStatique>(this, idSol);
	size_t dimSol = solBordure->dimensions().x / LARGEUR_CASE;
	delete solBordure;
	for(int cote = 0; cote < 4; ++cote) {
		ssize_t dim = ((cote == GAUCHE || cote == DROITE) ? _dimX + 2 * Niveau::epaisseurBordure() : _dimY);
		for(index_t i = 0; i < Niveau::epaisseurBordure(); ++i) {
			for(index_t j = 0; j < dim; ++j) {
				int nb = nombreAleatoire(10);
				if((((cote == BAS || cote == DROITE) && i == 0) || ((cote == HAUT || cote == GAUCHE) && i == Niveau::epaisseurBordure() - 1)) && (cote == HAUT || cote == BAS || ((j >= Niveau::epaisseurBordure()) && j < _dimX + Niveau::epaisseurBordure()))) {
					EntiteStatique *e = ElementNiveau::elementNiveau<EntiteStatique>(this, nb, ElementNiveau::arbre);
					_bordures[cote][i][j]._entites[cn_objet] = e;
				}
				if(((cote == GAUCHE || cote == DROITE) && (i % dimSol == 0 && (Niveau::epaisseurBordure() - j - 1) % dimSol == 0)) || ((cote == HAUT || cote == BAS) && ((Niveau::epaisseurBordure() - i - 1) % dimSol == 0 && j % dimSol == 0))) {
					EntiteStatique *e = ElementNiveau::elementNiveau<EntiteStatique>(this, idSol);
					_bordures[cote][i][j]._entites[cn_sol] = e;
				}
				
				for(couche_t c = premierCouche; c != nb_couches; ++c) {
					ElementNiveau *ee = _bordures[cote][i][j]._entites[c];
					if(!ee)
						continue;
					Coordonnees pos;
					switch(cote) {
						case GAUCHE:
							pos = Coordonnees(dim - j - Niveau::epaisseurBordure() - 1, -(Niveau::epaisseurBordure() - i));
							break;
						case DROITE:
							pos = Coordonnees(dim - j - Niveau::epaisseurBordure() - 1, _dimY + i);
							break;
						case HAUT:
							pos = Coordonnees(_dimX + Niveau::epaisseurBordure() - i - 1, j);
							break;
						case BAS:
							pos = Coordonnees(-i - 1, j);
							break;
					}
					ee->definirPosition(pos * LARGEUR_CASE);
				}
			}
		}
	}
}

ssize_t Niveau::epaisseurBordure() {
	return 24;
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

bool Niveau::collision(index_t x, index_t y) const {
	if(x < 0 || y < 0 || x >= _dimX || y >= _dimY)
		return true;
	for(couche_t c = premierCouche; c < nb_couches; ++c) {
		if(!Niveau::collision(c))
			continue;
		ElementNiveau const *e = this->element(x, y, c);
		if(e && e->collision())
			return true;
	}
	
	return false;
}

bool Niveau::collision(couche_t couche) {
	switch(couche) {
		case cn_sol:
			return false;			
		case cn_sol2:
			return true;
		case cn_transitionSol:
			return false;
		case cn_objet:
			return true;
		case nb_couches:
			return false;
	}
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
	if(Session::evenement(Session::T_a)) {
		this->definirZoom(this->zoom() * 1.2);
	}
	if(Session::evenement(Session::T_z)) {
		this->definirZoom(this->zoom() / 1.2);
	}
	
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
	index_t persoX = std::floor(_perso->position().x / LARGEUR_CASE), persoY = std::floor(_perso->position().y / LARGEUR_CASE);
	Coordonnees cam = _perso->positionAffichage() - (Ecran::dimensions() - _perso->dimensions()) / 2;
	cam.x = std::floor(cam.x);
	cam.y = std::floor(cam.y);
	

	this->afficherCouche(cn_sol, cam, persoX, persoY);

	this->afficherBordure(GAUCHE, cam);
	this->afficherBordure(HAUT, cam);
	
	
	// Sert à afficher la grille de case, inutile sauf en cas de test
	/*Coordonnees origine = cam;
	for(ssize_t y = 0; y <= _dimY; ++y) {
		for(ssize_t x = 0; x <= _dimX; ++x) {
			_b1.afficher(Coordonnees((x + y) * LARGEUR_CASE, (y - x) * LARGEUR_CASE / 2) / 2 - cam - Coordonnees(0, LARGEUR_CASE / 4));
			_b2.afficher(Coordonnees((x + y) * LARGEUR_CASE, (y - x) * LARGEUR_CASE / 2) / 2 - cam);
		}
	}*/
	
	
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

char const *Niveau::nomCouche(couche_t couche) {
	switch(couche) {
		case cn_objet:
			return "coucheObjet";			
		case cn_sol:
			return "coucheSol";			
		case cn_sol2:
			return "coucheSol2";			
		case cn_transitionSol:
			return "coucheTransitionSol";			
		case nb_couches:
			return 0;			
	}
}
