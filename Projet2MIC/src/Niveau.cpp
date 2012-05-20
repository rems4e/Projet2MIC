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
			total += val;
			_probasCumulees[e] = total;
			_nbNonNuls += val != 0;
		}
	}
	
	GenerateurElementAleatoire(ElementNiveau::elementNiveau_t cat, float proba) : _probasCumulees(), _nbNonNuls(0) {
		int total = 0;
		for(ElementNiveau::elementNiveau_t e = ElementNiveau::premierTypeElement; e != ElementNiveau::nbTypesElement; ++e) {
			int val = e == cat ? proba * NB_VALEURS_PROBA_ENTITES : 0;
			total += val;
			_probasCumulees[e] = total;
			_nbNonNuls += val != 0;
		}
	}
	
	ElementNiveau *operator()(Niveau *n) const {
		if(!_nbNonNuls)
			return 0;

		int nbAlea = nombreAleatoire(_nbNonNuls * NB_VALEURS_PROBA_ENTITES);
		for(ElementNiveau::elementNiveau_t e = ElementNiveau::premierTypeElement; e != ElementNiveau::nbTypesElement; ++e) {
			if(nbAlea < _probasCumulees[e]) {
				ElementNiveau *retour = ElementNiveau::elementNiveau(true, n, nombreAleatoire(static_cast<int>(ElementNiveau::nombreEntites(e))), e);
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

Niveau::Case::Case() : _entites(), _monnaie(0), _transitions() {

}

Niveau::Niveau(Joueur *j, std::string const &nomFichier) : _elements(0), _dimX(0), _dimY(0), _entitesMobiles(), _perso(), _bordures(), _persoInit(), _pluie(Session::cheminRessources() + "aucun.vert", Session::cheminRessources() + "pluie.frag"), _transitionSol(Session::cheminRessources() + "aucun.vert", Session::cheminRessources() + "transitionSol.frag"), _tonnerre(0), _transitionsBordures(), _musique(0) {	
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
	
	for(couche_t couche = premiereCouche; couche < nbCouches; ++couche) {
		TiXmlElement *cc = n->FirstChildElement(Niveau::nomCouche(couche));
		if(!cc)
			continue;

		char const *texte = cc->GetText();
		size_t tailleAttendue = _dimY * (_dimX * 4 + 1) - 1; // chaque ligne contient 4 chiffres hexa par colonne et un saut de ligne, sauf pour la dernière ligne.
		if(!texte || std::strlen(texte) != tailleAttendue) {
			std::string err((std::string() + "Erreur : dimensions de la couche " + nombreVersTexte(couche) + " invalides (" + nombreVersTexte(std::strlen(texte)) + " au lieu de " + nombreVersTexte(tailleAttendue) +  " attendus) !"));
			std::cerr << err << std::endl;
			throw Exc_CreationNiveau(err);
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
					if(couche == cn_sol && categorie == ElementNiveau::teleporteur && index == 0) {
						_persoInit = Coordonnees(x + 1.5, y + 1.5) * LARGEUR_CASE;
					}
					try {
						e = ElementNiveau::elementNiveau(true, this, index, categorie);
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
					e->definirPosition(Coordonnees(x, y) * LARGEUR_CASE);
					this->definirContenuCase(e->pX(), e->pY(), couche, e);
				}
			}
			
			// Saut de ligne.
			pos += 1;
		}
	}
	
	for(index_t i = 0; i != nbGenerateurs; ++i)
		delete generateurs[i];
	delete[] generateurs;
	
	this->remplissageTransitionsSol();
	
	_objet = Image(Session::cheminRessources() + "objet.png");
	_objets = Image(Session::cheminRessources() + "objets.png");
	_monnaie = Image(Session::cheminRessources() + "monnaie.png");
	
	char const *mus = n->Attribute("musique");
	if(!mus) {
		mus = "silence.wav";
	}
	_musique = Audio::chargerMusique(Session::cheminRessources() + mus);
	_tonnerre = Audio::chargerSon(Session::cheminRessources() + "tonnerre.wav");

	j->definirPosition(_persoInit);
	this->definirJoueur(j);
}

void Niveau::allocationCases() {
	// Allocation des cases du niveau
	_elements = new Case*[_dimY];
	for(size_t y = 0; y < _dimY; ++y) {
		_elements[y] = new Case[_dimX];
	}
	
	for(int i = 0; i < 4; ++i) {
		ssize_t dim = this->longueurBordure(i);
		_bordures[i] = new ElementNiveau *[dim];
	}
	
	_transitionsBordures[TRANSITION_GAUCHE] = new TransitionSol[_dimY];
	_transitionsBordures[TRANSITION_BAS] = new TransitionSol[_dimX];
	
	_anglesTransitionsBordures[TRANSITION_GAUCHE] = new TransitionSol[_dimY];
	_anglesTransitionsBordures[TRANSITION_BAS] = new TransitionSol[_dimX + 1];
}

void Niveau::remplissageBordures() {
	_solBordures = ElementNiveau::elementNiveau<EntiteStatique>(false, this, 2, ElementNiveau::sol);

	GenerateurElementAleatoire geneArbres(ElementNiveau::arbre, 1.0f);
	for(int i = 0; i < 4; ++i) {
		ssize_t dim = this->longueurBordure(i);
		for(int x = 0; x < dim; ++x) {
			ElementNiveau *e = geneArbres(this);
			_bordures[i][x] = e;
			if(i == GAUCHE) {
				e->definirPosition(Coordonnees(-1, x) * LARGEUR_CASE);
			}
			else if(i == DROITE) {
				e->definirPosition(Coordonnees(_dimX, x) * LARGEUR_CASE);
			}
			else if(i == HAUT) {
				e->definirPosition(Coordonnees(dim - x - 1, -1) * LARGEUR_CASE);
			}
			else if(i == BAS) {
				e->definirPosition(Coordonnees(dim - x - 1, _dimY) * LARGEUR_CASE);
			}
		}
	}
}

ssize_t Niveau::epaisseurBordure() {
	return 32;
}

ssize_t Niveau::longueurBordure(int cote) {
	switch(cote) {
		case GAUCHE:
		case DROITE:
			return _dimY;
		case HAUT:
		case BAS:
			return _dimX;
		default:
			return 0;
	}
}

void Niveau::transitionSol(index_t x, index_t y, int position) {
	if(x >= _dimX || y >= _dimY)
		return;
	
	TransitionSol &caseTransition = _elements[y][x]._transitions[position];
	if(caseTransition._element)
		return;
	ElementNiveau *transition = 0;
	index_t tX = x, tY = y;
	size_t dim = 0;
	if(position == TRANSITION_GAUCHE) {
		--tX;
	}
	else if(position == TRANSITION_BAS) {
		--tY;
	}
	bool versX = false, versY = false;

	if(tX < 0 || tY < 0) {
		transition = _solBordures;
	}
	else
		transition = _elements[tY][tX]._entites[cn_sol].begin()->entite;
	
	if(position == TRANSITION_GAUCHE) {
		dim = transition->dimY();
		versY = true;
	}
	else if(position == TRANSITION_BAS) {
		dim = transition->dimX();
		versX = true;
	}
	
	transition = ElementNiveau::elementNiveau(false, this, transition->index(), transition->categorie());
	for(index_t yy = y; yy < std::min(versY ? dim + y : y + 1, _dimY); ++yy) {
		for(index_t xx = x; xx < std::min(versX ? dim + x : x + 1, _dimX); ++xx) {
			_elements[yy][xx]._transitions[position]._element = transition;
			_elements[yy][xx]._transitions[position]._dim = dim;
			_elements[yy][xx]._transitions[position]._exterieure = (yy != y) || (xx != x);
		}
	}
	if(position == TRANSITION_BAS && dim + x < _dimX) {
		_elements[y][dim + x]._transitions[TRANSITION_ANGLE]._element = ElementNiveau::elementNiveau(false, this, transition->index(), transition->categorie());
		_elements[y][dim + x]._transitions[TRANSITION_ANGLE]._dim = 1;
		_elements[y][dim + x]._transitions[TRANSITION_ANGLE]._exterieure = false;
		_elements[y][dim + x]._transitions[TRANSITION_ANGLE]._element->definirPosition(Coordonnees(x + dim, y) * LARGEUR_CASE);
	}

	caseTransition._element->definirPosition(Coordonnees(x, y) * LARGEUR_CASE);
}

void Niveau::remplissageTransitionsSol() {
	// Transitions entre les diverses entités au sol
	for(size_t y = 0; y < _dimY; ++y) {
		for(ssize_t x = 0; x < _dimX; ++x) {
			ElementNiveau *el = _elements[y][x]._entites[cn_sol].begin()->entite;
			if(_elements[y][x]._entites[cn_sol].begin()->exterieure)
				continue;
			for(int yy = 0; yy < el->dimY(); ++yy) {
				this->transitionSol(x, y + yy, TRANSITION_GAUCHE);
			}
			for(int xx = 0; xx < el->dimX(); ++xx) {
				this->transitionSol(x + xx, y, TRANSITION_BAS);
			}
		}
	}
	
	for(size_t y = 0; y < _dimY; ++y) {
		ElementNiveau *el = _elements[y][_dimX - 1]._entites[cn_sol].begin()->entite;
		if(!_transitionsBordures[TRANSITION_GAUCHE][y]._element) {
			ElementNiveau *transition = ElementNiveau::elementNiveau(false, this, el->index(), el->categorie());
			_transitionsBordures[TRANSITION_GAUCHE][y]._exterieure = false;
			transition->definirPosition(Coordonnees(_dimX, y) * LARGEUR_CASE);
			for(int yy = 0; yy < el->dimY(); ++yy) {
				_transitionsBordures[TRANSITION_GAUCHE][y + yy]._element = transition;
				_transitionsBordures[TRANSITION_GAUCHE][y + yy]._dim = el->dimY();
			}
			if(y + el->dimY() < _dimY) {
				transition = ElementNiveau::elementNiveau(false, this, el->index(), el->categorie());
				_anglesTransitionsBordures[TRANSITION_GAUCHE][y + el->dimY()]._element = transition;
				_anglesTransitionsBordures[TRANSITION_GAUCHE][y + el->dimY()]._exterieure = false;
				_anglesTransitionsBordures[TRANSITION_GAUCHE][y + el->dimY()]._dim = 1;
				_anglesTransitionsBordures[TRANSITION_GAUCHE][y + el->dimY()]._element->definirPosition(Coordonnees(_dimX, y + el->dimY()) * LARGEUR_CASE);
			}
		}
	}

	for(size_t x = 0; x < _dimX; ++x) {
		ElementNiveau *el = _elements[_dimY - 1][x]._entites[cn_sol].begin()->entite;
		if(!_transitionsBordures[TRANSITION_BAS][x]._element) {
			ElementNiveau *transition = ElementNiveau::elementNiveau(false, this, el->index(), el->categorie());
			_transitionsBordures[TRANSITION_BAS][x]._exterieure = false;
			transition->definirPosition(Coordonnees(x, _dimY) * LARGEUR_CASE);
			for(int xx = 0; xx < el->dimX(); ++xx) {
				_transitionsBordures[TRANSITION_BAS][x + xx]._element = transition;
				_transitionsBordures[TRANSITION_BAS][x + xx]._dim = el->dimX();
			}
			transition = ElementNiveau::elementNiveau(false, this, el->index(), el->categorie());
			_anglesTransitionsBordures[TRANSITION_BAS][x + el->dimX()]._element = transition;
			_anglesTransitionsBordures[TRANSITION_BAS][x + el->dimX()]._exterieure = false;
			_anglesTransitionsBordures[TRANSITION_BAS][x + el->dimX()]._dim = 1;
			_anglesTransitionsBordures[TRANSITION_BAS][x + el->dimX()]._element->definirPosition(Coordonnees(x + el->dimX(), _dimY) * LARGEUR_CASE);
		}
	}
}

Niveau::~Niveau() {
	for(uindex_t y = 0; y != this->dimY(); ++y) {
		for(uindex_t x = 0; x != this->dimX(); ++x) {
			for(Niveau::couche_t c = premiereCouche; c != nbCouches; ++c) {
				for(elements_t::iterator i = _elements[y][x]._entites[c].begin(); i != _elements[y][x]._entites[c].end(); ++i) {
					if(!i->exterieure) {
						Entite *e = &(*i);
						if(!e->entite->mobile())
							delete i->entite;
					}
				}
			}
			for(int i = 0; i < 3; ++i) {
				if(!_elements[y][x]._transitions[i]._exterieure)
					delete _elements[y][x]._transitions[i]._element;
			}
		}
		delete[] _elements[y];
		
		if(!_transitionsBordures[TRANSITION_GAUCHE][y]._exterieure)
			delete _transitionsBordures[TRANSITION_GAUCHE][y]._element;
		if(!_anglesTransitionsBordures[TRANSITION_GAUCHE][y]._exterieure)
			delete _anglesTransitionsBordures[TRANSITION_GAUCHE][y]._element;
	}
	delete[] _elements;
	
	for(index_t x = 0; x < this->dimX(); ++x) {
		if(!_transitionsBordures[TRANSITION_BAS][x]._exterieure)
			delete _transitionsBordures[TRANSITION_BAS][x]._element;
		if(!_anglesTransitionsBordures[TRANSITION_BAS][x]._exterieure)
			delete _anglesTransitionsBordures[TRANSITION_BAS][x]._element;
	}
	if(!_anglesTransitionsBordures[TRANSITION_BAS][_dimX]._exterieure)
		delete _anglesTransitionsBordures[TRANSITION_BAS][_dimX]._element;
	
	delete[] _transitionsBordures[TRANSITION_GAUCHE];
	delete[] _transitionsBordures[TRANSITION_BAS];
	delete[] _anglesTransitionsBordures[TRANSITION_GAUCHE];
	delete[] _anglesTransitionsBordures[TRANSITION_BAS];
	
	if(_entitesMobiles.size()) {
		for(std::list<CaseMobile>::iterator i = _entitesMobiles.begin(); i != _entitesMobiles.end(); ++i) {
			if(i->_e != _perso)
				delete i->_e;
		}
	}
	
	for(int cote = 0; cote < 4; ++cote) {
		for(index_t i = 0; i < this->longueurBordure(cote); ++i) {
			delete _bordures[cote][i];
		}
		delete[] _bordures[cote];
	}
	
	for(std::list<ElementNiveau *>::iterator i = _aEffacer.begin(); i != _aEffacer.end(); ++i) {
		delete *i;
	}
	
	Audio::libererSon(_tonnerre);
	Audio::libererSon(_musique);
}

Joueur *Niveau::joueur() {
	return _perso;
}

Audio::audio_t Niveau::musique() {
	return _musique;
}

Niveau::listeElements_t Niveau::elements(index_t x, index_t y, Niveau::couche_t couche) {
	if(x < 0 || y < 0 || x >= this->dimX() || y >= this->dimY()) {
		return std::make_pair(_elements[0][0]._entites[premiereCouche].end(), _elements[0][0]._entites[premiereCouche].end());
	}
	
	if(_elements[y][x]._entites[couche].empty())
		return std::make_pair(_elements[0][0]._entites[premiereCouche].end(), _elements[0][0]._entites[premiereCouche].end());
	
	return std::make_pair(_elements[y][x]._entites[couche].begin(), _elements[y][x]._entites[couche].end());
}

Niveau::const_listeElements_t Niveau::elements(index_t x, index_t y, Niveau::couche_t couche) const {
	if(x < 0 || y < 0 || x >= this->dimX() || y >= this->dimY()) {
		return std::make_pair(_elements[0][0]._entites[premiereCouche].end(), _elements[0][0]._entites[premiereCouche].end());
	}
	
	if(_elements[y][x]._entites[couche].empty())
		return std::make_pair(_elements[0][0]._entites[premiereCouche].end(), _elements[0][0]._entites[premiereCouche].end());
	
	return std::make_pair(_elements[y][x]._entites[couche].begin(), _elements[y][x]._entites[couche].end());
}

ssize_t Niveau::monnaie(index_t x, index_t y) const {
	return _elements[y][x]._monnaie;
}

void Niveau::modifierMonnaie(index_t x, index_t y, ssize_t delta) {
	_elements[y][x]._monnaie = std::max<ssize_t>(0, _elements[y][x]._monnaie + delta);
}

void Niveau::ajouterElement(index_t x, index_t y, couche_t couche, ElementNiveau *elem) {
	this->definirContenuCase(x, y, couche, elem);
}

void Niveau::supprimerElement(ElementNiveau *e, couche_t couche, bool deleteElement) {
	if(deleteElement) {
		_aEffacer.push_back(e);
	}
	
	if(e->mobile()) {
		_entitesMobiles.remove(CaseMobile(static_cast<EntiteMobile *>(e)));
	}

	elements_t::iterator i = std::find(_elements[e->pY()][e->pX()]._entites[couche].begin(), _elements[e->pY()][e->pX()]._entites[couche].end(), e);
	_elements[e->pY()][e->pX()]._entites[couche].erase(i);
}

Niveau::elements_t::iterator Niveau::supprimerElement(elements_t::iterator i, couche_t couche, bool deleteElement) {
	ElementNiveau *e = i->entite;
	if(deleteElement) {
		_aEffacer.push_back(e);
	}
	
	if(e->mobile()) {
		_entitesMobiles.remove(CaseMobile(static_cast<EntiteMobile *>(e)));
	}
	
	return _elements[i->entite->pY()][i->entite->pX()]._entites[couche].erase(i);
}

void Niveau::notifierDeplacement(EntiteMobile *e, index_t ancienX, index_t ancienY, couche_t ancienneCouche) {
	index_t x = e->pX(), y = e->pY();
	elements_t &liste = _elements[ancienY][ancienX]._entites[ancienneCouche];
	elements_t::iterator elem = std::find(liste.begin(), liste.end(), e);

	liste.erase(elem);
	
	Entite paire(e, false);
	
	std::list<CaseMobile>::iterator i = _entitesMobiles.begin();
	while(i != _entitesMobiles.end()) {
		if(i->_e == e)
			break;
		++i;
	}
	i->_pos = &(_elements[y][x]);
	i->_pos->_entites[i->_e->couche()].push_back(Entite(e, false));
}

bool Niveau::collision(index_t x, index_t y, couche_t couche, ElementNiveau *el) const {
	if(x < 0 || y < 0 || x >= _dimX || y >= _dimY)
		return true;

	const_listeElements_t liste = this->elements(x, y, couche);
	for(elements_t::const_iterator e = liste.first; e != liste.second; ++e) {
		if(e->entite != el && e->entite->collision(x - e->entite->pX(), y - e->entite->pY()))
			return true;
	}
	
	return false;
}

bool Niveau::collision(couche_t couche) {
	switch(couche) {
		case cn_sol:
			return false;
		case cn_sol2:
			return false;
		case cn_objet:
			return true;
		case cn_objetsInventaire:
			return false;
		case nbCouches:
			return false;
	}
}

size_t Niveau::dimX() const {
	return _dimX;
}

size_t Niveau::dimY() const {
	return _dimY;
}

void Niveau::animer() {
	for(std::list<ElementNiveau *>::iterator i = _aEffacer.begin(); i != _aEffacer.end(); ++i) {
		delete *i;
	}
	_aEffacer.clear();

	for(couche_t c = premiereCouche; c != nbCouches; ++c) {
		for(size_t y = 0; y < _dimY; ++y) {
			for(ssize_t x = _dimX - 1; x >= 0; --x) {
				elements_t::iterator const fin = _elements[y][x]._entites[c].end();
				for(elements_t::iterator i = _elements[y][x]._entites[c].begin(); i != fin; ++i) {
					if(!i->exterieure && !i->entite->mobile()) {
						i->entite->animer();
					}
				}
			}
		}
	}
	
	std::list<CaseMobile>::iterator const fin = _entitesMobiles.end();
	for(std::list<CaseMobile>::iterator i = _entitesMobiles.begin(); i != fin; ++i) {
		if(!i->_e->mort()) {
			i->_e->animer();
		}
	}
}

void Niveau::afficher() {
	Coordonnees cam = _perso->positionAffichage();
	if(_perso->inventaireAffiche())
		cam = _perso->positionAffichage() - Coordonnees(Ecran::largeur() / 4, 0);
	cam -= (Ecran::dimensions() - _perso->dimensions() * LARGEUR_CASE) / 2 - Coordonnees(0, 80);
	cam.x = std::floor(cam.x);
	cam.y = std::floor(cam.y);
	

	this->afficherCouche(cn_sol, cam);
	this->afficherTransitionsSol(cam);
	
	this->afficherBordure(DROITE, cam);
	this->afficherBordure(HAUT, cam);
	
	// Sert à afficher la grille de case, inutile sauf en cas de test
	/*for(index_t y = 0; y <= _dimY; ++y) {
		Ecran::afficherLigne(referentielNiveauVersEcran(Coordonnees(0, y) * LARGEUR_CASE) - cam, referentielNiveauVersEcran(Coordonnees(_dimX, y) * LARGEUR_CASE) - cam, Couleur::rouge, 1.0);
	}
	for(index_t x = 0; x <= _dimX; ++x) {
		Ecran::afficherLigne(referentielNiveauVersEcran(Coordonnees(x, 0) * LARGEUR_CASE) - cam, referentielNiveauVersEcran(Coordonnees(x, _dimY) * LARGEUR_CASE) - cam, Couleur::rouge, 1.0);
	}*/
	
	for(Niveau::couche_t c = premiereCouche + 1; c != nbCouches; ++c) {
		if(c == cn_objetsInventaire)
			this->afficherObjetsInventaire(cam);
		else
			this->afficherCouche(c, cam);
	}
	
	this->afficherBordure(BAS, cam);
	this->afficherBordure(GAUCHE, cam);

	static float tonnerre = 0;
	if(horloge() - tonnerre > nombreAleatoire(5) + 1) {
		if(tonnerre > 0)
			Audio::jouerSon(_tonnerre);
		tonnerre = horloge() + 15 + nombreAleatoire(60);
	}
	
	_pluie.activer();
	_pluie.definirParametre("temps", horloge());
	_pluie.definirParametre("cam", cam.x, cam.y);
	_pluie.definirParametre("tonnerre", tonnerre);
	
	_pluie.activer();
	Ecran::afficherRectangle(Ecran::ecran(), Couleur::blanc);
	Shader::desactiver();
}

void Niveau::afficherObjetsInventaire(Coordonnees const &cam) {
	for(size_t y = 0; y < _dimY; ++y) {
		for(ssize_t x = _dimX - 1; x >= 0; --x) {
			size_t nb = _elements[y][x]._entites[cn_objetsInventaire].size();
			Image *img = 0;
			if(nb == 1)
				img = &_objet;
			else if(nb > 1)
				img = &_objets;
			if(img) {
				Coordonnees pos = referentielNiveauVersEcran(Coordonnees(x, y) * LARGEUR_CASE) - Coordonnees(11, 18) + Coordonnees(LARGEUR_CASE, 0) / 2;
				img->afficher(pos - cam);
			}
			
			if(_elements[y][x]._monnaie) {
				Coordonnees pos = referentielNiveauVersEcran(Coordonnees(x, y) * LARGEUR_CASE) - Coordonnees(20, 15) * 0.7 + Coordonnees(LARGEUR_CASE, 0) / 2;
				_monnaie.redimensionner(0.7);
				_monnaie.afficher(pos - cam);
			}
		}
	}
}

void Niveau::afficherCouche(couche_t c, Coordonnees const &cam) {
	for(size_t y = 0; y < _dimY; ++y) {
		for(ssize_t x = _dimX - 1; x >= 0; --x) {
			elements_t::iterator const fin = _elements[y][x]._entites[c].end();
			for(elements_t::iterator i = _elements[y][x]._entites[c].begin(); i != fin; ++i) {
				if(x == i->entite->pX() && (i->entite->decoupagePerspective() || y == i->entite->pY())) {
					if(c == cn_objet && i->entite->mobile() && static_cast<EntiteMobile *>(i->entite)->mort()) {
						
					}
					else {
						i->entite->afficher(y - i->entite->pY(), cam);
					}
				}
			}
			if(c == cn_sol2) {
				elements_t::iterator const fin = _elements[y][x]._entites[cn_objet].end();
				for(elements_t::iterator i = _elements[y][x]._entites[cn_objet].begin(); i != fin; ++i) {
					if(i->entite->mobile() && static_cast<EntiteMobile *>(i->entite)->mort()) {
						i->entite->afficher(y - i->entite->pY(), cam);
					}
				}
			}
		}
	}
}

void Niveau::afficherTransitionsSol(Coordonnees const &cam) {
	_transitionSol.activer();
	
	for(int i = 0; i < 3; ++i) {
		_transitionSol.definirParametre("pos", i);
		for(size_t y = 0; y < _dimY; ++y) {
			for(ssize_t x = _dimX - 1; x >= 0; --x) {
				TransitionSol *t = _elements[y][x]._transitions;
				if(!t[i]._exterieure) {
					_transitionSol.definirParametre("dimTrans", t[i]._dim);
					t[i]._element->afficher(0, cam);
				}
			}
		}
	}
	
	Shader::desactiver();
}

void Niveau::afficherBordure(int cote, Coordonnees const &cam) {	
	ssize_t dim = this->longueurBordure(cote);

	size_t dimI = _solBordures->dimY(), dimJ = _solBordures->dimX();
	index_t premier = 0;
	if(cote == GAUCHE || cote == DROITE) {
		dim += Niveau::epaisseurBordure();
		premier = -Niveau::epaisseurBordure();
		dimI = _solBordures->dimX(), dimJ = _solBordures->dimY();
	}
	
	
	for(int i = 0; i < Niveau::epaisseurBordure(); i += dimI) {
		for(int j = premier; j < dim; j += dimJ) {
			if(cote == GAUCHE)
				_solBordures->definirPosition(Coordonnees(-Niveau::epaisseurBordure() + i, j) * LARGEUR_CASE);
			else if(cote == DROITE)
				_solBordures->definirPosition(Coordonnees(_dimX + i, j) * LARGEUR_CASE);
			else if(cote == HAUT)
				_solBordures->definirPosition(Coordonnees(j, -Niveau::epaisseurBordure() + i) * LARGEUR_CASE);
			else if(cote == BAS)
				_solBordures->definirPosition(Coordonnees(j, _dimY + i) * LARGEUR_CASE);
			
			_solBordures->afficher(0, cam);
		}
	}
	
	if(cote == DROITE) {
		_transitionSol.activer();

		TransitionSol *t = _transitionsBordures[TRANSITION_GAUCHE];
		TransitionSol *a = _anglesTransitionsBordures[TRANSITION_GAUCHE];
		for(size_t y = 0; y < _dimY; ++y) {
			if(!t[y]._exterieure) {
				_transitionSol.definirParametre("pos", TRANSITION_GAUCHE);
				_transitionSol.definirParametre("dimTrans", t[y]._dim);
				t[y]._element->afficher(0, cam);
			}
			if(!a[y]._exterieure) {
				_transitionSol.definirParametre("pos", TRANSITION_ANGLE);
				_transitionSol.definirParametre("dimTrans", a[y]._dim);
				a[y]._element->afficher(0, cam);
			}
		}

		Shader::desactiver();
	}
	else if(cote == BAS) {
		_transitionSol.activer();

		TransitionSol *t = _transitionsBordures[TRANSITION_BAS];
		TransitionSol *a = _anglesTransitionsBordures[TRANSITION_BAS];
		for(size_t x = 0; x < _dimX; ++x) {
			if(!t[x]._exterieure) {
				_transitionSol.definirParametre("pos", TRANSITION_BAS);
				_transitionSol.definirParametre("dimTrans", t[x]._dim);
				t[x]._element->afficher(0, cam);
			}
			if(!a[x]._exterieure) {
				_transitionSol.definirParametre("pos", TRANSITION_ANGLE);
				_transitionSol.definirParametre("dimTrans", a[x]._dim);
				a[x]._element->afficher(0, cam);
			}
		}
		if(!a[_dimX]._exterieure) {
			_transitionSol.definirParametre("pos", TRANSITION_ANGLE);
			_transitionSol.definirParametre("dimTrans", a[_dimX]._dim);
			a[_dimX]._element->afficher(0, cam);
		}

		Shader::desactiver();
	}
	
	dim = this->longueurBordure(cote);
	for(int j = 0; j < dim; ++j) {
		_bordures[cote][j]->afficher(0, cam);
	}
}

void Niveau::definirContenuCase(index_t x, index_t y, couche_t couche, ElementNiveau *e) {
	if(e->mobile()) {
		EntiteMobile *eM = static_cast<EntiteMobile *>(e);
		CaseMobile c(eM);
		c._pos = &(_elements[y][x]);
		_entitesMobiles.push_back(c);
		c._pos->_entites[couche].push_back(Entite(eM, false));
	}
	else {
		size_t dimX = e->dimX(), dimY = e->dimY();
		for(index_t yy = y; yy < std::min(dimY + y, _dimY); ++yy) {
			for(index_t xx = x; xx < std::min(dimX + x, _dimX); ++xx) {
				_elements[yy][xx]._entites[couche].push_back(Entite(e, (yy != y) || (xx != x)));
			}
		}
	}
}

void Niveau::definirJoueur(Joueur *j) {
	_perso = j;
	this->definirContenuCase(j->pX(), j->pY(), _perso->couche(), _perso);
}

char const *Niveau::nomCouche(couche_t couche) {
	switch(couche) {
		case cn_objet:
			return "coucheObjet";
		case cn_sol:
			return "coucheSol";
		case cn_sol2:
			return "coucheSol2";
		case cn_objetsInventaire:
			return "coucheObjetsInventaire";
		case nbCouches:
			return 0;
	}
}
