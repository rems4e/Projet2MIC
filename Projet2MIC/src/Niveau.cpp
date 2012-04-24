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

Niveau::Case::Case() : _entites(), _monnaie(0) {

}

Niveau::Niveau(Joueur *j, std::string const &nomFichier) : _elements(0), _dimX(0), _dimY(0), _zoom(1.0), _entitesMobiles(), _perso(), _bordures(), _persoInit() {
	/*horloge_t temps = horloge();
	Shader barreDefilement(Session::cheminRessources() + "aucun.vert", Session::cheminRessources() + "barre.frag");

	while(Session::boucle(100, (horloge() - temps < 20) && !Session::evenement(Session::QUITTER))) {
		Ecran::definirPointeurAffiche(true);
		Ecran::definirPointeur(0);
		Ecran::effacer();

		barreDefilement.definirParametre(Shader::avancement, (horloge() - temps) / 20);
		barreDefilement.definirParametre(Shader::temps, (horloge() - temps));
		
		Rectangle barre;
		barre.definirDimensions(Coordonnees(400, 30));
		barre.definirOrigine((Ecran::dimensions() - barre.dimensions()) / 2);
		Ecran::afficherRectangle(barre, Couleur::rouge, barreDefilement);
		
		
		Ecran::maj();
	}
	exit(0);*/
	
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
	
	j->definirPosition(_persoInit);
	this->definirJoueur(j);
}

Niveau::Niveau(Joueur *j) : _elements(0), _dimX(0), _dimY(0), _zoom(1.0), _entitesMobiles(), _perso(j), _bordures() {
	_dimX = 160;
	_dimY = 160;
	
	this->allocationCases();
	this->remplissageBordures();

	// Remplissage du terrain avec un sol uni et des arbres disposés aléatoirement
	for(size_t y = 0; y < _dimY; ++y) {
		for(size_t x = 0; x < _dimX; ++x) {
			Case &c = _elements[y][x];
			if(c._entites[cn_sol].size() == 0) {
				ElementNiveau *e = ElementNiveau::elementNiveau(true, this, 2, ElementNiveau::entiteStatique);
				this->definirContenuCase(x, y, cn_sol, e);
				e->definirPosition(Coordonnees(x, y) * LARGEUR_CASE);
			}
			if(c._entites[cn_objet].size() == 0) {
				int nb = nombreAleatoire(100);
				if(nb < 10) {
					EntiteStatique *e = ElementNiveau::elementNiveau<EntiteStatique>(true, this, nb, ElementNiveau::arbre);
					this->definirContenuCase(x, y, cn_objet, e);
					e->definirPosition(Coordonnees(x, y) * LARGEUR_CASE);
				}
			}
		}
	}

	this->definirJoueur(j);
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
	
	_objet = Image(Session::cheminRessources() + "objet.png");
	_objets = Image(Session::cheminRessources() + "objets.png");
	_monnaie = Image(Session::cheminRessources() + "monnaie.png");
}

void Niveau::remplissageBordures() {
	index_t idSol = 2;
	EntiteStatique *solBordure = ElementNiveau::elementNiveau<EntiteStatique>(false, this, idSol);
	size_t dimSol = solBordure->dimX();
	delete solBordure;
	GenerateurElementAleatoire geneArbres(ElementNiveau::arbre, 1.0f);
	for(int cote = 0; cote < 4; ++cote) {
		ssize_t dim = ((cote == GAUCHE || cote == DROITE) ? _dimX + 2 * Niveau::epaisseurBordure() : _dimY);
		for(index_t i = 0; i < Niveau::epaisseurBordure(); ++i) {
			for(index_t j = 0; j < dim; ++j) {
				if((((cote == BAS || cote == DROITE) && i == 0) || ((cote == HAUT || cote == GAUCHE) && i == Niveau::epaisseurBordure() - 1)) && (cote == HAUT || cote == BAS || ((j >= Niveau::epaisseurBordure()) && j < _dimX + Niveau::epaisseurBordure()))) {
					ElementNiveau *e = geneArbres(this);
					_bordures[cote][i][j]._entites[cn_objet].push_back(Entite(e, false));
				}
				if(((cote == GAUCHE || cote == DROITE) && (i % dimSol == 0 && (Niveau::epaisseurBordure() - j - 1) % dimSol == 0)) || ((cote == HAUT || cote == BAS) && ((Niveau::epaisseurBordure() - i - 1) % dimSol == 0 && j % dimSol == 0))) {
					ElementNiveau *e = ElementNiveau::elementNiveau<EntiteStatique>(false, this, idSol);
					_bordures[cote][i][j]._entites[cn_sol].push_back(Entite(e, false));
				}
				
				for(couche_t c = premiereCouche; c != nbCouches; ++c) {
					if(_bordures[cote][i][j]._entites[c].empty())
						continue;
					ElementNiveau *ee = _bordures[cote][i][j]._entites[c].front().entite;
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
			for(Niveau::couche_t c = premiereCouche; c != nbCouches; ++c) {
				for(elements_t::iterator i = _elements[y][x]._entites[c].begin(); i != _elements[y][x]._entites[c].end(); ++i) {
					if(!i->exterieure) {
						Entite *e = &(*i);
						if(!e->entite->mobile())
							delete i->entite;
					}
				}
			}
		}
		delete[] _elements[y];
	}
	delete[] _elements;
	
	if(_entitesMobiles.size()) {
		for(std::list<CaseMobile>::iterator i = _entitesMobiles.begin(); i != _entitesMobiles.end(); ++i) {
			if(i->_e != _perso)
				delete i->_e;
		}
	}
	
	for(int cote = 0; cote < 4; ++cote) {
		for(size_t i = 0; i < Niveau::epaisseurBordure(); ++i) {
			ssize_t dim = ((cote == GAUCHE || cote == DROITE) ? _dimX + 2 * Niveau::epaisseurBordure() : _dimY);
			for(uindex_t j = 0; j != dim; ++j) {
				for(Niveau::couche_t c = premiereCouche; c != nbCouches; ++c) {
					for(elements_t::iterator k = _bordures[cote][i][j]._entites[c].begin(); k != _bordures[cote][i][j]._entites[c].end(); ++k) {
						delete k->entite;
					}
				}
			}
			delete[] _bordures[cote][i];
		}
		delete[] _bordures[cote];
	}
	
	for(std::list<ElementNiveau *>::iterator i = _aEffacer.begin(); i != _aEffacer.end(); ++i) {
		delete *i;
	}
}

Joueur *Niveau::joueur() {
	return _perso;
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
	if(elem == liste.end())
		std::cout << "hjoijoi" << std::endl;
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
			return true;
		case cn_transitionSol:
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

double Niveau::zoom() const {
	return _zoom;
}

void Niveau::definirZoom(double z) {
	_zoom = z;
	if(_zoom == 0.0) {
		_zoom = 1.0;
	}
}

void Niveau::animer() {
	for(std::list<ElementNiveau *>::iterator i = _aEffacer.begin(); i != _aEffacer.end(); ++i) {
		delete *i;
	}
	_aEffacer.clear();

	for(int cote = 0; cote < 4; ++cote) {
		ssize_t dim = ((cote == GAUCHE || cote == DROITE) ? _dimX + 2 * Niveau::epaisseurBordure() : _dimY);
		for(Niveau::couche_t c = premiereCouche; c != nbCouches; ++c) {
			for(uindex_t i = 0; i < Niveau::epaisseurBordure(); ++i) {
				for(uindex_t j = 0; j < dim; ++j) {
					elements_t::iterator const fin = _bordures[cote][i][j]._entites[c].end();
					for(elements_t::iterator k = _bordures[cote][i][j]._entites[c].begin(); k != fin; ++k) {
						if(!k->exterieure) {
							k->entite->animer();
						}
					}
				}
			}
		}
	}

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

	this->afficherBordure(GAUCHE, cam);
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
	this->afficherBordure(DROITE, cam);
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
				img->redimensionner(this->zoom());
				Coordonnees pos = referentielNiveauVersEcran(Coordonnees(x, y) * LARGEUR_CASE) - Coordonnees(11, 18) + Coordonnees(LARGEUR_CASE, 0) / 2;
				img->afficher(pos * this->zoom() - cam);
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
						i->entite->afficher(y - i->entite->pY(), cam * this->zoom(), this->zoom());
					}
				}
			}
			if(c == cn_sol2) {
				elements_t::iterator const fin = _elements[y][x]._entites[cn_objet].end();
				for(elements_t::iterator i = _elements[y][x]._entites[cn_objet].begin(); i != fin; ++i) {
					if(i->entite->mobile() && static_cast<EntiteMobile *>(i->entite)->mort()) {
						i->entite->afficher(y - i->entite->pY(), cam * this->zoom(), this->zoom());
					}
				}
			}
		}
	}
}

void Niveau::afficherBordure(int cote, Coordonnees const &cam) {
	ssize_t dim = ((cote == GAUCHE || cote == DROITE) ? _dimX + 2 * Niveau::epaisseurBordure() : _dimY);
	for(Niveau::couche_t c = premiereCouche; c != nbCouches; ++c) {
		for(uindex_t i = 0; i < Niveau::epaisseurBordure(); ++i) {
			for(uindex_t j = 0; j < dim; ++j) {
				for(elements_t::iterator k = _bordures[cote][i][j]._entites[c].begin(); k != _bordures[cote][i][j]._entites[c].end(); ++k) {
					if(!k->exterieure) {
						k->entite->afficher(0, cam * this->zoom(), this->zoom());
					}
				}
			}
		}
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
		case cn_transitionSol:
			return "coucheTransitionSol";
		case cn_objetsInventaire:
			return "coucheObjetsInventaire";
		case nbCouches:
			return 0;
	}
}
