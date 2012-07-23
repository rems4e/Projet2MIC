//
//  Inventaire.cpp
//  Projet2MIC
//
//  Créé par Marc Promé et Rémi Saurel.
//  Ce fichier et son contenu sont librement distribuables, modifiables et utilisables pour toute œuvre non commerciale, à condition d'en citer les auteurs.
//

#include "Inventaire.h"
#include <vector>
#include <list>
#include <limits>
#include "Session.h"
#include <algorithm>
#include "Joueur.h"
#include "Marchand.h"
#include "Partie.h"
#include "Niveau.h"
#include "tinyxml.h"
#include "Texte.h"

std::list<Texte *> infoObjet(ObjetInventaire *o, Personnage const &reference);
void afficherInfos(std::list<Texte *> const &infos, Coordonnees const &position, Rectangle const &cadre);

Inventaire::Inventaire(Personnage &perso) : _perso(perso), _monnaie(0) {
	
}

Inventaire::~Inventaire() {
	
}

Personnage const &Inventaire::personnage() const {
	return  _perso;
}

Personnage &Inventaire::personnage() {
	return  _perso;
}

ssize_t Inventaire::monnaie() const {
	return _monnaie;
}

void Inventaire::modifierMonnaie(ssize_t delta) {
	_monnaie = std::max<ssize_t>(0L, _monnaie + delta);
}

template <template <class e, class = std::allocator<e> > class Conteneur>
template <typename InputIterator>
InventaireC<Conteneur>::InventaireC(Personnage &perso, size_t capacite, InputIterator debut, InputIterator fin) : Inventaire(perso), _elements(debut, fin), _capacite(capacite) {
	if(capacite < std::distance(debut, fin)) {
		// FIXME: pourri
		std::cerr << "Initialisation erronée de l'inventaire : trop d'objets pour sa capacité. fuite de mémoire." << std::endl;
		iterator i = this->debut();
		std::advance(i, _capacite);
		for(; i != this->end();) {
			i = _elements.erase(i);
		}
	}
}

template <template <class e, class = std::allocator<e> > class Conteneur>
InventaireC<Conteneur>::InventaireC(Personnage &perso, size_t capacite) : Inventaire(perso), _elements(), _capacite(capacite) {
	
}

template <template <class e, class = std::allocator<e> > class Conteneur>
InventaireC<Conteneur>::~InventaireC() {
	for(iterator i = _elements.begin(); i != _elements.end(); ++i) {
		delete *i;
	}
}

template <template <class e, class = std::allocator<e> > class Conteneur>
size_t InventaireC<Conteneur>::nombreObjets() const {
	return _elements.size();
}

template <template <class e, class = std::allocator<e> > class Conteneur>
size_t InventaireC<Conteneur>::capacite() const {
	return _capacite;
}

template <template <class e, class = std::allocator<e> > class Conteneur>
void InventaireC<Conteneur>::definirCapacite(size_t c) {
	if(c < this->nombreObjets())
		return;
	
	_capacite = _capacite;
}

template <template <class e, class = std::allocator<e> > class Conteneur>
typename InventaireC<Conteneur>::const_iterator InventaireC<Conteneur>::debut() const {
	return _elements.begin();
}

template <template <class e, class = std::allocator<e> > class Conteneur>
typename InventaireC<Conteneur>::const_iterator InventaireC<Conteneur>::fin() const {
	return _elements.end();
}

template <template <class e, class = std::allocator<e> > class Conteneur>
typename InventaireC<Conteneur>::iterator InventaireC<Conteneur>::debut() {
	return _elements.begin();
}

template <template <class e, class = std::allocator<e> > class Conteneur>
typename InventaireC<Conteneur>::iterator InventaireC<Conteneur>::fin() {
	return _elements.end();
}

template <template <class e, class = std::allocator<e> > class Conteneur>
bool InventaireC<Conteneur>::ajouterObjet(ObjetInventaire *o) {
	if(this->nombreObjets() < this->capacite()) {
		_elements.push_back(o);
		return true;
	}
	return false;
}

template <template <class e, class = std::allocator<e> > class Conteneur>
void InventaireC<Conteneur>::supprimerObjet(ObjetInventaire *o) {
	for(iterator i = _elements.begin(); i != _elements.end(); ++i) {
		if(*i == o) {
			_elements.erase(i._base);
			break;
		}
	}
}

template <template <class e, class = std::allocator<e> > class Conteneur>
void InventaireC<Conteneur>::vider() {
	_elements.clear();
}

template <template <class e, class = std::allocator<e> > class Conteneur>
void InventaireC<Conteneur>::definirNiveau(Niveau *n) {
	for(iterator i = debut(); i != fin(); ++i) {
		if(*i) {
			(*i)->definirNiveau(n);
		}
	}
}

template<typename InputIterator>
InventaireListe::InventaireListe(Personnage &perso, size_t capacite, InputIterator debut, InputIterator fin) : InventaireC<std::list>(perso, capacite, debut, fin) {
	
}

InventaireListe::InventaireListe(Personnage &perso, size_t capacite) : InventaireC<std::list>(perso, capacite)  {
	
}

InventaireListe::~InventaireListe() {
	
}

void InventaireListe::afficher() const {

}

bool InventaireListe::gestionEvenements() {
	return true;
}

void InventaireListe::masquer() {
	
}

void InventaireListe::preparationAffichage() {
	
}

InventaireListe::const_iterator InventaireListe::debut() const {
	return _elements.begin();
}

InventaireListe::const_iterator InventaireListe::fin() const {
	return _elements.end();
}

InventaireListe::iterator InventaireListe::debut() {
	return _elements.begin();
}

InventaireListe::iterator InventaireListe::fin() {
	return _elements.end();
}

template<typename InputIterator>
InventaireSol::InventaireSol(Personnage &perso, InputIterator debut, InputIterator fin) : InventaireListe(perso, std::numeric_limits<size_t>::max(), debut, fin) {
	
}

InventaireSol::InventaireSol(Personnage &perso) : InventaireListe(perso, std::numeric_limits<size_t>::max()) {
	
}

InventaireSol::~InventaireSol() {
	
}

void InventaireSol::definirCapacite(size_t c) {
	
}

void InventaireSol::afficher() const {
	
}

bool InventaireSol::gestionEvenements() {
	return true;
}

template<typename InputIterator>
InventaireTableau::InventaireTableau(Personnage &perso, size_t largeur, size_t hauteur, InputIterator debut, InputIterator fin) : InventaireC<std::vector>(perso, largeur * hauteur, debut, fin), _largeur(largeur), _hauteur(hauteur) {
	size_t nb = std::distance(debut, fin);
	size_t manquants = this->capacite() - nb;
	for(index_t i = 0; i < manquants; ++i) {
		this->ajouterObjet(0);
	}
}

InventaireTableau::InventaireTableau(Personnage &perso, size_t largeur, size_t hauteur) : InventaireC<std::vector>(perso, largeur * hauteur), _largeur(largeur), _hauteur(hauteur) {
	for(index_t i = 0; i < this->capacite(); ++i) {
		this->InventaireC<std::vector>::ajouterObjet(0);
	}
}

InventaireTableau::~InventaireTableau() {
	
}

void InventaireTableau::afficher() const {
	
}

size_t InventaireTableau::nombreObjets() const {
	size_t nb = 0;
	for(const_iterator i = debut(); i != fin(); ++i) {
		if(*i)
			++nb;
	}
	
	return nb;
}

bool InventaireTableau::ajouterObjet(ObjetInventaire *o) {
	if(!o)
		return false;
	for(index_t i = 0; i < this->capacite(); ++i) {
		if(this->ajouterObjetEnPosition(o, i))
			return true;
	}

	return false;
}

bool InventaireTableau::ajouterObjetEnPosition(ObjetInventaire *o, index_t position) {
	if(!o)
		return false;
	size_t dimX = o->dimensionsInventaire().x, dimY = o->dimensionsInventaire().y;
	index_t pX = position % _largeur, pY = position / _largeur;

	
	for(index_t x = 0; x < dimX; ++x) {
		for(index_t y = 0; y < dimY; ++y) {
			if(x + pX >= _largeur || y + pY >= _hauteur) {
				return false;
			}
			if(this->objetDansCase(x + pX, y + pY)) {
				return false;
			}
		}
	}
	
	(*(this->debut()._base + position)) = o;
	
	return true;
}

void InventaireTableau::supprimerObjet(ObjetInventaire *o) {
	if(!o)
		return;
	std::vector<ObjetInventaire *>::iterator pos = std::find(this->debut()._base, this->fin()._base, o);
	if(pos != this->fin()._base) {
		*pos = 0;
	}
}

ObjetInventaire *InventaireTableau::objetDansCase(index_t position) {
	return this->objetDansCase(position % _largeur, position / _largeur);
}

ObjetInventaire *InventaireTableau::objetDansCase(index_t pX, index_t pY) {
	const_iterator deb = this->debut();
	for(index_t x = 0; x <= pX; ++x) {
		for(index_t y = 0; y <= pY; ++y) {
			index_t p = y * _largeur + x;
			if(!comprisEntre<index_t>(p, 0, this->capacite() - 1))
				return 0;
			ObjetInventaire *o = *(deb._base + p);
			if(o && x + o->dimensionsInventaire().x > pX && y + o->dimensionsInventaire().y > pY) {
				return o;
			}
		}
	}
	
	return 0;
}

bool InventaireTableau::peutPlacerObjetDansCase(index_t pX, index_t pY, ObjetInventaire *objet, ObjetInventaire *&remp) {
	remp = 0;
	if(pX + objet->dimensionsInventaire().x > this->largeur() || pY + objet->dimensionsInventaire().y > this->hauteur())
		return false;
	
	for(index_t x = 0; x < objet->dimensionsInventaire().x; ++x) {
		for(index_t y = 0; y < objet->dimensionsInventaire().y; ++y) {
			ObjetInventaire *obj = this->objetDansCase(pX + x, pY + y);
			if(remp == 0)
				remp = obj;
			else if(obj && remp != obj) {
				return false;
			}
		}
	}

	return true;
}

index_t InventaireTableau::largeur() const {
	return _largeur;
}

index_t InventaireTableau::hauteur() const {
	return _hauteur;
}

Coordonnees InventaireTableau::positionObjet(ObjetInventaire *o) const {
	for(const_iterator i = this->debut(); i != this->fin(); ++i) {
		if(*i == o) {
			index_t pos = std::distance(this->debut()._base, i._base);
			return Coordonnees(pos % _largeur, pos / _largeur);
		}
	}
	
	return Coordonnees::aucun;
}

void InventaireTableau::vider() {
	this->InventaireC<std::vector>::vider();
	for(int i = 0; i < this->capacite(); ++i) {
		this->InventaireC<std::vector>::ajouterObjet(0);
	}
}

InventaireTableau::const_iterator InventaireTableau::debut() const {
	return _elements.begin();
}

InventaireTableau::const_iterator InventaireTableau::fin() const {
	return _elements.end();
}

InventaireTableau::iterator InventaireTableau::debut() {
	return _elements.begin();
}

InventaireTableau::iterator InventaireTableau::fin() {
	return _elements.end();
}

InventaireMarchand::InventaireMarchand(Marchand &perso) : InventaireTableau(perso, LARGEUR_MARCHAND, HAUTEUR_MARCHAND), _fond(Session::cheminRessources() + "inventaireMarchand.png"), _infos(0), _gauche(Session::cheminRessources() + "igauche.png"), _droite(Session::cheminRessources() + "idroite.png") {

}

InventaireMarchand::~InventaireMarchand() {
	
}

void InventaireMarchand::afficher() const {
	_sortie = Rectangle(15, 440, 46, 49).etirer(Ecran::echelle());
	_sortie.definirOrigine(_sortie.origine() + Coordonnees(Ecran::largeur() / 2, 0));
	
	_inventaire.definirOrigine(Coordonnees(Ecran::largeur() / 2, 0) + Coordonnees(17, 8).etirer(Ecran::echelle()));
	_inventaire.definirDimensions(Coordonnees(360, 360).etirer(Ecran::echelle()));

	_fond.redimensionner(Ecran::echelle());
	_fond.afficher(Coordonnees(Ecran::largeur() / 2, 0));
	
	_gauche.redimensionner(Ecran::echelle());
	_droite.redimensionner(Ecran::echelle());

	_cadreGauche.definirOrigine(Coordonnees(Ecran::largeur() / 2 + 200 * Ecran::echelle().x - _gauche.dimensions().x - 5 * Ecran::echelle().x, 440 * Ecran::echelle().y));
	_cadreGauche.definirDimensions(_gauche.dimensions());

	_cadreDroite.definirOrigine(Coordonnees(Ecran::largeur() / 2 + (200 + 5) * Ecran::echelle().x, 440 * Ecran::echelle().y));
	_cadreDroite.definirDimensions(_droite.dimensions());
	
	_gauche.afficher(_cadreGauche.origine());
	_droite.afficher(_cadreDroite.origine());
	
	Coordonnees pos(Ecran::largeur() / 2 + 28 * Ecran::echelle().x, 12 * Ecran::echelle().y);
	index_t i = 0;
	for(const_iterator j = this->debut(); j != this->fin(); ++j) {
		if(*j) {
			(*j)->image().redimensionner(Ecran::echelle());
			(*j)->image().afficher(pos);
		}
		pos.x += 36 * Ecran::echelle().x;
		++i;
		if((i % this->largeur()) == 0) {
			pos.x = Ecran::largeur() / 2 + 28 * Ecran::echelle().x;
			pos.y += 35.4 * Ecran::echelle().y;
		}
	}

	if(_surlignage != Rectangle::aucun) {
		Ecran::afficherRectangle(_surlignage, _couleurSurlignage);
	}

	if(_infos) {
		std::list<Texte *> infos = infoObjet(_infos, *Partie::partie()->joueur());
		Texte *t = new Texte(TRAD("inv %1 p. or", Partie::partie()->marchand()->prixVente(_infos)));
		if(Partie::partie()->marchand()->prixVente(_infos) > Partie::partie()->joueur()->inventaire()->monnaie())
			t->definir(Couleur::rouge);
		else
			t->definir(Couleur::jaune);
		
		infos.push_back(t);
		
		afficherInfos(infos, Session::souris(), Rectangle(Ecran::largeur() / 2, 0, Ecran::largeur() / 2, Partie::partie()->zoneJeu().hauteur));
		
		for(std::list<Texte *>::iterator i = infos.begin(); i != infos.end(); ++i) {
			delete *i;
		}
	}
}

bool InventaireMarchand::gestionEvenements() {
	bool sortie = false;
	_surlignage = Rectangle::aucun;
	_infos = 0;
	
	if(Session::souris() < _sortie && Session::evenement(Session::B_GAUCHE)) {
		sortie = true;
		Session::reinitialiser(Session::B_GAUCHE);
	}
	else if(Session::souris() < _inventaire) {
		Coordonnees pos = Session::souris() - _inventaire.origine();
		index_t pX = pos.x / (36 * Ecran::echelle().x), pY = pos.y / (36 * Ecran::echelle().y);
		if(comprisEntre<index_t>(pX, 0, this->largeur() - 1) && comprisEntre<index_t>(pY, 0, this->hauteur() - 1)) {
			if(Session::evenement(Session::B_GAUCHE)) {
				InventaireJoueur *iJ = static_cast<InventaireJoueur *>(Partie::partie()->joueur()->inventaire());
				if(iJ->objetTransfert() == 0) {
					ObjetInventaire *article = this->objetDansCase(pX, pY);
					ssize_t prix = Partie::partie()->marchand()->prixVente(article);
					
					if(prix <= iJ->monnaie()) {
						this->supprimerObjet(article);
						iJ->definirObjetTransfert(article);
						iJ->modifierMonnaie(-prix);
					}
				}
				Session::reinitialiser(Session::B_GAUCHE);
			}
			else {
				InventaireJoueur *iJ = static_cast<InventaireJoueur *>(Partie::partie()->joueur()->inventaire());
				if(iJ->objetTransfert() == 0) {
					ObjetInventaire *o = this->objetDansCase(pX, pY);
					if(o) {
						Coordonnees positionCase = this->positionObjet(o);
						_surlignage = Rectangle(_inventaire.origine() + Coordonnees(positionCase.x * 36 * Ecran::echelle().x, positionCase.y * 35.4 * Ecran::echelle().y), Coordonnees(o->dimensionsInventaire().x * 36 * Ecran::echelle().x, o->dimensionsInventaire().y * 35.4 * Ecran::echelle().y));
						if(Partie::partie()->marchand()->prixVente(o) <= iJ->monnaie())
							_couleurSurlignage = Couleur(Couleur::blanc, 66);
						else {
							_couleurSurlignage = Couleur(Couleur::rouge, 50);
						}
					}
					_infos = o;
				}
			}
		}
	}
	
	return !sortie;
}

Rectangle const &InventaireMarchand::zoneObjets() const {
	return _inventaire;
}

void InventaireMarchand::masquer() {
	for(iterator i = this->debut(); i != this->fin(); ++i) {
		if(*i) {
			delete *i;
		}
	}
	this->vider();
	
	_infos = 0;
}

void InventaireMarchand::preparationAffichage() {
	for(int i = 0; i < 20; ++i) {
		ObjetInventaire *e = ElementNiveau::elementNiveau<ObjetInventaire>(false, this->personnage().niveau(), nombreAleatoire(6));
		this->ajouterObjet(e);
	}
}

void InventaireMarchand::definirCapacite(size_t c) {
	
}

InventaireJoueur::InventaireJoueur(Joueur &perso) : InventaireTableau(perso, LARGEUR_JOUEUR, HAUTEUR_JOUEUR), _fond(Session::cheminRessources() + "inventaire.png"), _objetTransfert(0), _or("", POLICE_DECO, 20, Couleur::blanc), _infos(0), _inventaire() {
}

InventaireJoueur::~InventaireJoueur() {
	delete _objetTransfert;
}

void InventaireJoueur::afficher() const {
	_inventaire = Rectangle(21, 282, 365, 150).etirer(Ecran::echelle());
	
	_tenue[Personnage::brasG] = Rectangle(22, 51, 76, 136).etirer(Ecran::echelle());
	_tenue[Personnage::brasD] = Rectangle(198, 51, 76, 136).etirer(Ecran::echelle());
	_tenue[Personnage::casque] = Rectangle(110, 2, 80, 72).etirer(Ecran::echelle());
	_tenue[Personnage::armure] = Rectangle(110, 83, 80, 105).etirer(Ecran::echelle());
	_tenue[Personnage::gants] = Rectangle(19, 200, 80, 75).etirer(Ecran::echelle());
	_tenue[Personnage::bottes] = Rectangle(198, 200, 80, 75).etirer(Ecran::echelle());
	
	_sortie = Rectangle(341, 442, 46, 49).etirer(Ecran::echelle());
		
	_fond.redimensionner(Ecran::echelle());
	_fond.afficher(Coordonnees());
	
	_or.definir(20 * Ecran::echelleMin());
	_or.definir(nombreVersTexte(this->monnaie()));
	_or.afficher(Coordonnees(205, 450).etirer(Ecran::echelle()) + Coordonnees(6, -8).etirer(Ecran::echelle()));

	Coordonnees pos = Coordonnees(32, 288).etirer(Ecran::echelle());
	index_t i = 0;
	for(const_iterator j = this->debut(); j != this->fin(); ++j) {
		if(*j) {
			(*j)->image().redimensionner(Ecran::echelle());
			(*j)->image().afficher(pos);
		}
		pos.x += 36 * Ecran::echelle().x;
		++i;
		if((i % this->largeur()) == 0) {
			pos.x = 32 * Ecran::echelle().x;
			pos.y += 37 * Ecran::echelle().y;
		}
	}
	for(Personnage::positionTenue_t i = Personnage::premierePositionTenue; i != Personnage::nbPositionsTenue; ++i) {
		if(this->personnage().tenue(i)) {
			Image const &img = this->personnage().tenue(i)->image();
			img.redimensionner(Ecran::echelle());
			img.afficher(_tenue[i].origine() + (_tenue[i].dimensions() - img.dimensions()) / 2);
		}
	}
	
	if(_surlignage != Rectangle::aucun) {
		Ecran::afficherRectangle(_surlignage, _couleurSurlignage);
	}
	if(_surlignageTransfert != Rectangle::aucun) {
		Ecran::afficherRectangle(_surlignageTransfert.intersection(_inventaire), _couleurSurlignageTransfert);
	}
	
	pos = Coordonnees(284, 20).etirer(Ecran::echelle());
	std::vector<std::pair<Texte, Coordonnees> > infosJoueur;
	Texte t;
	t.definir(Couleur::blanc);
	t.definir(POLICE_DECO, 14 * Ecran::echelleMin());
	for(Personnage::competences_t c = Personnage::premiereCompetence; c != Personnage::nbCompetences; ++c) {
		// FIXME: traduction " : "
		t.definir(Personnage::nomCompetence(c) + std::string(" :"));
		infosJoueur.push_back(std::make_pair(t, pos));
		pos.y += 5 * Ecran::echelle().y + t.dimensions().y;
		
		t.definir(nombreVersTexte(this->personnage().competences()[c]));
		infosJoueur.push_back(std::make_pair(t, pos + Coordonnees(100 * Ecran::echelle().x - t.dimensions().x, 0)));
		pos.y += 5 * Ecran::echelle().y + t.dimensions().y;		
	}
	
	for(std::vector<std::pair<Texte, Coordonnees> >::const_iterator i = infosJoueur.begin(); i != infosJoueur.end(); ++i) {
		i->first.afficher(i->second);
	}

	if(_infos) {
		std::list<Texte *> infos = infoObjet(_infos, this->personnage());
		if(Partie::partie()->marchand() && _infos->categorieObjet() != ObjetInventaire::cle) {
			Texte *t = new Texte(TRAD("inv %1 p. or", Partie::partie()->marchand()->prixAchat(_infos)));
			t->definir(Couleur::jaune);
			infos.push_back(t);
		}

		afficherInfos(infos, Session::souris(), Rectangle(0, 0, Ecran::largeur() / 2, Partie::partie()->zoneJeu().hauteur));
		
		for(std::list<Texte *>::iterator i = infos.begin(); i != infos.end(); ++i) {
			delete *i;
		}
	}
}

bool InventaireJoueur::gestionEvenements() {
	bool sortie = false;
	
	_surlignage = Rectangle::aucun;
	_surlignageTransfert = Rectangle::aucun;
	_infos = 0;

	if(Session::souris() < _sortie && Session::evenement(Session::B_GAUCHE)) {
		sortie = true;
		Session::reinitialiser(Session::B_GAUCHE);
	}
	else 	if(Partie::partie()->marchand() && Session::souris() < static_cast<InventaireMarchand*>(Partie::partie()->marchand()->inventaire())->zoneObjets()) {
		if(_objetTransfert && Session::evenement(Session::B_GAUCHE)  && _objetTransfert->categorieObjet() != ObjetInventaire::cle) {
			this->modifierMonnaie(Partie::partie()->marchand()->prixAchat(_objetTransfert));
			if(!Partie::partie()->marchand()->inventaire()->ajouterObjet(_objetTransfert)) {
				delete _objetTransfert;
			}
			_objetTransfert = 0;
			Session::reinitialiser(Session::B_GAUCHE);
		}
	}
	else if(Session::evenement(Session::B_GAUCHE) && Session::souris() < Partie::partie()->zoneJeu()) {
		if(_objetTransfert) {
			this->personnage().niveau()->ajouterElement(this->personnage().pX(), this->personnage().pY(), Niveau::cn_objetsInventaire, _objetTransfert);
			_objetTransfert = 0;
		}
		Session::reinitialiser(Session::B_GAUCHE);
	}
	else if(Session::souris() < _inventaire) {
		Coordonnees pos = Session::souris() - _inventaire.origine();
		index_t pX = pos.x / (36 * Ecran::echelle().x), pY = pos.y / (37 * Ecran::echelle().y);
		if(comprisEntre<index_t>(pX, 0, this->largeur() - 1) && comprisEntre<index_t>(pY, 0, this->hauteur() - 1)) {
			if(Session::evenement(Session::B_GAUCHE)) {
				if(_objetTransfert) {
					ObjetInventaire *remp;
					bool ok = this->peutPlacerObjetDansCase(pX, pY, _objetTransfert, remp);

					if(ok) {
						this->supprimerObjet(remp);
						this->ajouterObjetEnPosition(_objetTransfert, pY * this->largeur() + pX);
						_objetTransfert = remp;
					}
				}
				else {
					ObjetInventaire *o = this->objetDansCase(pX, pY);
					_objetTransfert = o;
					this->supprimerObjet(o);
				}
				Session::reinitialiser(Session::B_GAUCHE);
			}
			else {
				ObjetInventaire *o = this->objetDansCase(pX, pY);
				if(o) {
					Coordonnees positionCase = this->positionObjet(o);
					_surlignage = Rectangle(_inventaire.origine() + Coordonnees(positionCase.x * 36.5 * Ecran::echelle().x, positionCase.y * 37 * Ecran::echelle().y), Coordonnees(o->dimensionsInventaire().x * 36.5 * Ecran::echelle().x, o->dimensionsInventaire().y * 37 * Ecran::echelle().y));
					_couleurSurlignage = Couleur(Couleur::blanc, 66);
					if(!_objetTransfert)
						_infos = o;
				}
				if(_objetTransfert) {
					_surlignageTransfert = Rectangle(_inventaire.origine() + Coordonnees(pX * 36.5 * Ecran::echelle().x, pY * 37 * Ecran::echelle().y), Coordonnees(_objetTransfert->dimensionsInventaire().x * 36.5 * Ecran::echelle().x, _objetTransfert->dimensionsInventaire().y * 37 * Ecran::echelle().y));
					ObjetInventaire *remp;
					bool ok = this->peutPlacerObjetDansCase(pX, pY, _objetTransfert, remp);
					if(ok) {
						if(remp) {
							Coordonnees positionCase = this->positionObjet(remp);
							_surlignage = Rectangle(_inventaire.origine() + Coordonnees(positionCase.x * 36.5 * Ecran::echelle().x, positionCase.y * 37 * Ecran::echelle().y), Coordonnees(remp->dimensionsInventaire().x * 36.5 * Ecran::echelle().y, remp->dimensionsInventaire().y * 37 * Ecran::echelle().y));
							_couleurSurlignage = Couleur(Couleur::blanc, 33);
							_couleurSurlignageTransfert = Couleur(Couleur::blanc, 66);
						}
						else {
							_couleurSurlignageTransfert = Couleur(Couleur::blanc, 66);
						}
					}
					else {
						_couleurSurlignageTransfert = Couleur(Couleur::rouge, 50);
						_surlignage = Rectangle::aucun;
					}
				}
			}
		}
	}
	else {
		for(Personnage::positionTenue_t i = Personnage::premierePositionTenue; i != Personnage::nbPositionsTenue; ++i) {
			if(Session::souris() < _tenue[i]) {
				if(Session::evenement(Session::B_GAUCHE)) {
					if(_objetTransfert) {
						if(this->personnage().peutEquiperObjet(_objetTransfert, i)) {
							ObjetInventaire *o = _objetTransfert;
							_objetTransfert = this->personnage().tenue(i);
							this->personnage().definirTenue(i, o);
						}
					}
					else {
						_objetTransfert = this->personnage().tenue(i);
						this->personnage().definirTenue(i, 0);
					}
					Session::reinitialiser(Session::B_GAUCHE);
				}
				else {
					if(_objetTransfert) {
						if(this->personnage().peutEquiperObjet(_objetTransfert, i)) {
							_couleurSurlignage = Couleur(Couleur::blanc, 66);
						}
						else {
							_couleurSurlignage = Couleur(Couleur::rouge, 50);
						}
						_surlignage = _tenue[i];
					}
					else if(this->personnage().tenue(i)) {
						_couleurSurlignage = Couleur(Couleur::blanc, 66);
						_surlignage = _tenue[i];
						_infos = this->personnage().tenue(i);
					}
				}
				break;
			}
		}
	}
	this->definirObjetTransfert(_objetTransfert);
	
	return !sortie;
}

void InventaireJoueur::masquer() {
	if(_objetTransfert) {
		this->personnage().niveau()->ajouterElement(this->personnage().pX(), this->personnage().pY(), Niveau::cn_objetsInventaire, _objetTransfert);
		_objetTransfert = 0;
	}
}

void InventaireJoueur::preparationAffichage() {

}

void InventaireJoueur::definirCapacite(size_t c) {
	
}

ObjetInventaire *InventaireJoueur::objetTransfert() {
	return _objetTransfert;
}

void InventaireJoueur::definirObjetTransfert(ObjetInventaire *o) {
	_objetTransfert = o;
	if(_objetTransfert) {
		Ecran::definirPointeur(&_objetTransfert->image());
	}
}

TiXmlElement *InventaireJoueur::sauvegarde() const {
	TiXmlElement *e = new TiXmlElement("Inventaire");
	
	e->SetAttribute("monnaie", this->monnaie());
	for(const_iterator i = this->debut(); i != this->fin(); ++i) {
		TiXmlElement *objet = new TiXmlElement("Objet");
		if(*i) {
			TiXmlElement *comp = (*i)->competencesRequises().sauvegarde();
			objet->InsertEndChild(*comp);
			delete comp;
			objet->SetAttribute("index", (*i)->index());
			
			objet->SetAttribute("attaque", (*i)->attaque());
			objet->SetAttribute("defense", (*i)->defense());
			objet->SetAttribute("vie", (*i)->vie());
		}
		e->InsertEndChild(*objet);
		delete objet;
	}
	
	return e;
}

void InventaireJoueur::restaurer(TiXmlElement *sauvegarde) {
	TiXmlElement *e = sauvegarde->FirstChildElement("Inventaire");
	
	ssize_t monnaie;
	e->Attribute("monnaie", &monnaie);
	this->modifierMonnaie(monnaie - this->monnaie());
	
	index_t pos = 0;
	for(TiXmlElement *o = e->FirstChildElement(); o; o = o->NextSiblingElement(), ++pos) {
		if(o->Attribute("index")) {
			index_t i;
			o->Attribute("index", &i);
			ObjetInventaire *obj = ElementNiveau::elementNiveau<ObjetInventaire>(false, this->personnage().niveau(), i);
			Personnage::Competences c;
			c.restaurer(o);

			if(o->Attribute("attaque")) {
				o->Attribute("attaque", &i);
				obj->definirAttaque(i);
			}
			if(o->Attribute("defense")) {
				o->Attribute("defense", &i);
				obj->definirDefense(i);
			}
			if(o->Attribute("vie")) {
				o->Attribute("vie", &i);
				obj->definirVie(i);
			}
			
			obj->definirCompetencesRequises(c);
			this->ajouterObjetEnPosition(obj, pos);
		}
	}

}

std::list<Texte *> infoObjet(ObjetInventaire *o, Personnage const &reference) {
	std::list<Texte *> retour;
	
	switch(o->categorieObjet()) {
		case ObjetInventaire::bouclier:
		case ObjetInventaire::armure:
		case ObjetInventaire::casque:
		case ObjetInventaire::gants:
		case ObjetInventaire::bottes: {
			Texte *t = new Texte(TRAD("inv Défense : %1", o->defense()));
			t->definir(Couleur::bleu);
			retour.push_back(t);
		}
		case ObjetInventaire::arme: {
			for(Personnage::competences_t c = Personnage::premiereCompetence; c != Personnage::nbCompetences; ++c) {
				Texte *t = new Texte("");
				t->definir(Personnage::nomCompetence(c) + std::string(" : ") + nombreVersTexte(o->competencesRequises()[c]));
				if(o->competencesRequises()[c] > reference.competences()[c])
					t->definir(Couleur::rouge);
				else
					t->definir(Couleur::blanc);
				
				retour.push_back(t);
			}
			break;
		}
		case ObjetInventaire::potion: {
			Texte *t = new Texte(TRAD("inv Points de vie : %1", o->vie()));
			t->definir(Couleur::bleu);
			retour.push_back(t);
			break;
		}
		case ObjetInventaire::cle: {
			break;
		}
	}
	
	if(o->categorieObjet() == ObjetInventaire::arme) {
		Texte *t = new Texte(TRAD("inv Attaque : %1", o->attaque()));
		t->definir(Couleur::bleu);
		retour.push_front(t);
	}
	
	Texte *t = new Texte(o->nomCategorieObjet());
	t->definir(Couleur::blanc);
	retour.push_front(t);
	
	return retour;
}

void afficherInfos(std::list<Texte *> const &infos, Coordonnees const &position, Rectangle const &cadre) {
	Coordonnees dim;
	Coordonnees const ecart = Coordonnees(5, 5).etirer(Ecran::echelle());
	for(std::list<Texte *>::const_iterator i = infos.begin(); i != infos.end(); ++i) {
		(*i)->definir(POLICE_DECO, 12 * Ecran::echelleMin());
		dim.x = std::max(dim.x, (*i)->dimensions().x);
		dim.y += (*i)->dimensions().y + ecart.y;
	}
	dim.x += 2 * ecart.x;
	dim.y += ecart.y;

	Coordonnees pos = position + Ecran::pointeur()->dimensions();

	if(pos.x + dim.x > cadre.gauche + cadre.largeur)
		pos.x = position.x - dim.x;
	if(pos.y + dim.y > cadre.haut + cadre.hauteur)
		pos.y = position.y - dim.y;

	Ecran::afficherRectangle(Rectangle(pos, dim), Couleur(0, 0, 0, 200));
	pos += ecart;
			
	for(std::list<Texte *>::const_iterator i = infos.begin(); i != infos.end(); ++i) {
		(*i)->afficher(pos);
		pos.y += (*i)->dimensions().y + ecart.y;
	}
}

