//
//  EntiteMobile.cpp
//  Projet2MIC
//
//  Créé par Marc Promé et Rémi Saurel.
//  Ce fichier et son contenu sont librement distribuables, modifiables et utilisables pour toute œuvre non commerciale, à condition d'en citer les auteurs.
//

#include "EntiteMobile.h"
#include <string>
#include <cstring>
#include "tinyxml.h"
#include "Niveau.h"
#include <cmath>
#include "nombre.h"
#include "Session.h"

EntiteMobile::action_t &operator++(EntiteMobile::action_t &c) { return c = static_cast<EntiteMobile::action_t>(static_cast<int>(c + 1)); }

EntiteMobile::EntiteMobile(bool decoupagePerspective, Niveau *n, uindex_t index, ElementNiveau::elementNiveau_t cat) : ElementNiveau(decoupagePerspective, n, index, cat), _tempsPrecedent(0), _images(), _cadres(), _tempsAffichage(), _nbImages(), _imageActuelle(0), _action(a_immobile), _direction(gauche), _mort(false), _mortTerminee(false) {
	std::memset(_nbImages, 0, nbActions * sizeof(size_t));
	for(action_t a = premiereAction; a != nbActions; ++a) {
		std::memset(_cadres[a], 0, 8 * sizeof(Rectangle *));
	}
	
	TiXmlElement *e = ElementNiveau::description(index, cat);
	char const *img = e->Attribute("image");
	if(img) {
		for(action_t a = premiereAction; a != nbActions; ++a) {
			_images[a] = Image(Session::cheminRessources() + img);
		}
	}
	
	size_t dimX = 128, dimY = 128;
	if(e->Attribute("blocX"))
		e->Attribute("blocX", &dimX);
	if(e->Attribute("blocY"))
		e->Attribute("blocY", &dimY);
	
	size_t premiereImage[nbActions] = {0};
	
	for(action_t a = premiereAction; a != nbActions; ++a) {
		TiXmlElement *action = e->FirstChildElement(EntiteMobile::transcriptionAction(a));
		if(action) {
			action->Attribute("nbPoses", &_nbImages[a]);
			if(action->Attribute("image")) {
				_images[a] = Image(Session::cheminRessources() + action->Attribute("image"));
			}
			
			if(action->Attribute("premiere"))
				action->Attribute("premiere", &premiereImage[a]);
			else if(a != premiereAction) {
				premiereImage[a] = premiereImage[a - 1] + _nbImages[a - 1];
			}
			int temps;
			action->Attribute("tempsAttente", &temps);
			_tempsAffichage[a] = temps / 1000.0f;
			
			if(a == a_attaquer) {
				if(action->Attribute("attaque")) {
					action->Attribute("attaque", &_imageAttaque);
				}
				else {
					_imageAttaque = _nbImages[a_attaquer] - 1;
				}
			}
			
			if(!_images[a].valide()) {
				throw ElementNiveau::Exc_DefinitionEntiteIncomplete();
			}
		}
	}
	
	for(int direction = 0; direction < 8; ++direction) {
		for(action_t a = premiereAction; a != nbActions; ++a) {
			index_t x = premiereImage[a];
			_cadres[a][direction] = new Rectangle[_nbImages[a]];
			for(int p = 0; p < _nbImages[a]; ++p) {
				_cadres[a][direction][p] = Rectangle(x * dimX, direction * dimY, dimX, dimY);
				++x;
			}
		}
	}
	
	this->definirAction(EntiteMobile::a_immobile);
	_direction = direction_t(nombreAleatoire(8));
}

EntiteMobile::~EntiteMobile() {
	for(action_t a = premiereAction; a != nbActions; ++a) {
		for(int i = 0; i < 8; ++i) {
			delete[] _cadres[a][i];
		}
	}
}

void EntiteMobile::afficher(index_t deltaY, Coordonnees const &decalage) const {
	Rectangle const &cadre = this->cadre();
	_images[_action].afficher(this->positionAffichage() - decalage, cadre);
}

bool EntiteMobile::collision(index_t x, index_t y) const {
	return false;
}

index_t EntiteMobile::nPX(coordonnee_t pX) const {
	return std::floor((pX + 0*this->origine().x) / LARGEUR_CASE);

}

index_t EntiteMobile::nPY(coordonnee_t pY) const {
	return std::floor(pY / LARGEUR_CASE);
}

bool EntiteMobile::mort() const {
	return _mort;
}

void EntiteMobile::preparerMort() {
	_action = nbActions;
	this->definirAction(a_mourir);
	this->mourir();
}

void EntiteMobile::animer() {
	if(horloge() - _tempsPrecedent >= _tempsAffichage[_action]) {
		_tempsPrecedent = horloge();
		_imageActuelle = (_imageActuelle + 1) % _nbImages[_action];
		
		if(_action == a_mourir && _imageActuelle == _nbImages[a_mourir] - 1) {
			_mortTerminee = true;
		}
		if(_imageActuelle == 0) {
			_action = nbActions;
			this->definirAction(a_immobile);
		}
	}
}

void EntiteMobile::mourir() {
	_mort = true;
}

void EntiteMobile::renaitre() {
	_mort = false;
	_mortTerminee = false;
}

bool EntiteMobile::mortTerminee() const {
	return _mortTerminee;
}

Coordonnees EntiteMobile::dimensions() const {
	return Coordonnees(1, 1);
}

Rectangle const &EntiteMobile::cadre() const {
	return _cadres[_action][_direction][_imageActuelle];
}

bool EntiteMobile::actionDisponible(action_t a) const {
	return _nbImages[a];
}
					
char const *EntiteMobile::transcriptionAction(action_t a) {
	switch(a) {
		case a_immobile:
			return "Immobile";
		case a_deplacer:
			return "Deplacer";
		case a_attaquer:
			return "Attaque";
		case a_mourir:
			return "Mourir";
		case nbActions:
			return "nbActions";
	}
}

bool EntiteMobile::actionInterruptible(action_t a) {
	switch(a) {
		case a_immobile:
			return true;
		case a_deplacer:
			return true;
		case a_attaquer:
			return false;
		case a_mourir:
			return false;
		case nbActions:
			return true;
	}
}


bool EntiteMobile::actionInterruptible() const {
	return EntiteMobile::actionInterruptible(_action);
}

void EntiteMobile::definirDirection(direction_t d) {
	++_compteurDirection;
	if(_compteurDirection > 3) {
		_direction = d;
		_compteurDirection = 0;
	}
}

bool EntiteMobile::definirAction(action_t a) {
	if(this->actionDisponible(a)) {
		if(this->actionInterruptible()) {
			if(_action != a) {
				_action = a;
				_imageActuelle = 0;
				_compteurDirection = 0;
			}
			
			return true;
		}
	}
	
	return false;
}

bool EntiteMobile::deplacerPosition(Coordonnees const &delta) {
	if(delta.vecteurNul())
		return false;
	
	Coordonnees anciennePosition = this->position();
	index_t x = this->pX(), y = this->pY();
	
	direction_t dir = EntiteMobile::gauche;
	if(delta.x < 0) {
		if(delta.y < 0)
			dir = EntiteMobile::gauche;
		else if(delta.y == 0)
			dir = EntiteMobile::gaucheBas;
		else if(delta.y > 0)
			dir = EntiteMobile::bas;
	}
	else if(delta.x == 0) {
		if(delta.y < 0)
			dir = EntiteMobile::gaucheHaut;
		else if(delta.y > 0)
			dir = EntiteMobile::droiteBas;
	}
	else if(delta.x > 0) {
		if(delta.y < 0)
			dir = EntiteMobile::haut;
		else if(delta.y == 0)
			dir = EntiteMobile::droiteHaut;
		else if(delta.y > 0)
			dir = EntiteMobile::droite;
	}
	
	this->definirDirection(dir);
	
	size_t const n = std::floor(delta.norme());
	if(n == 0) {
		if(testerDeplacement(delta))
			this->definirPosition(this->position() + delta);
	}
	else {
		Coordonnees const dep = delta / n;
		for(index_t i = 0; i < n; ++i) {
			if(testerDeplacement(dep)) {
				this->definirPosition(this->position() + dep);
			}
			else
				break;
		}
		if(testerDeplacement(delta - n * dep))
			this->definirPosition(this->position() + delta - n * dep);
	}
	
	bool deplace = this->position() != anciennePosition;
	if(deplace) {
		this->niveau()->notifierDeplacement(this, x, y, this->couche());
		
		return true;
	}
	
	return false;
}

bool EntiteMobile::testerDeplacement(Coordonnees const &dep) {
	if(dep.vecteurNul())
		return true;
	
	index_t x1 = this->nPX(this->position().x + dep.x), y1 = this->nPY(this->position().y + dep.y);
	index_t x2 = this->nPX(this->position().x + dep.x + LARGEUR_CASE - 1), y2 = this->nPY(this->position().y + dep.y + LARGEUR_CASE - 1);
	
	if(dep.x < 0)
		x2 = x1;
	else if(dep.x > 0)
		x1 = x2;
	if(dep.y < 0)
		y2 = y1;
	else if(dep.y > 0)
		y1 = y2;
	
	
	for(index_t x = x1; x <= x2; ++x) {
		for(index_t y = y1; y <= y2; ++y) {
			if(this->niveau()->collision(x, y, this->couche(), this)) {
				return false;
			}
		}
	}
		
	return true;
}

EntiteMobile::action_t EntiteMobile::action() const {
	return _action;
}

EntiteMobile::direction_t EntiteMobile::direction() const {
	return _direction;
}

bool EntiteMobile::mobile() const {
	return true;
}

bool EntiteMobile::personnage() const {
	switch(this->categorieMobile()) {
		case em_ennemi:
		case em_joueur:
		case em_marchand:
			return true;
	}
}
