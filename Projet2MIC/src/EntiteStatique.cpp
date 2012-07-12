//
//  EntiteStatique.cpp
//  Projet2MIC
//
//  Créé par Marc Promé et Rémi Saurel.
//  Ce fichier et son contenu sont librement distribuables, modifiables et utilisables pour toute œuvre non commerciale, à condition d'en citer les auteurs.
//

#include "EntiteStatique.h"
#include "Ecran.h"
#include "tinyxml.h"
#include "Niveau.h"
#include <cmath>

EntiteStatique::EntiteStatique(bool decoupagePerspective, Niveau *n, uindex_t index, ElementNiveau::elementNiveau_t cat) : ElementNiveau(decoupagePerspective, n, index, cat), _image(), _cadres(0), _nb(1) {
	TiXmlElement *e = ElementNiveau::description(index, cat);
	char const *img = e->Attribute("image");
	if(!img)
		throw ElementNiveau::Exc_DefinitionEntiteIncomplete();

	_image = Image(Session::cheminRessources() + img);
	
	Coordonnees origine;
	if(e->Attribute("x"))
		e->Attribute("x", &origine.x);
	if(e->Attribute("y"))
		e->Attribute("y", &origine.y);
	Coordonnees dimensions(_image.dimensions());
	if(e->Attribute("l"))
		e->Attribute("l", &dimensions.x);
	if(e->Attribute("h"))
		e->Attribute("h", &dimensions.y);

	if(this->decoupagePerspective()) {
		_nb = this->dimY();
		_cadres = new Rectangle[_nb];
		_cadres[0] = Rectangle(origine, Coordonnees(LARGEUR_CASE, dimensions.y));
		for(index_t i = 1; i < _nb; ++i) {
			_cadres[i].definirDimensions(_cadres[0].dimensions());
			_cadres[i].definirOrigine(Coordonnees((_cadres[i - 1].origine() + _cadres[i - 1].dimensions()).x, 0));
			if(_cadres[i].origine().x + _cadres[i].largeur > dimensions.x)
				_nb = i + 1;
		}
		_cadres[_nb - 1].largeur = dimensions.x - (_cadres[_nb - 1].gauche - origine.x);
	}
	else {
		_cadres = new Rectangle[1];
		_cadres[0].definirOrigine(origine);
		_cadres[0].definirDimensions(dimensions);
	}
}

EntiteStatique::~EntiteStatique() {
	delete[] _cadres;
}

void EntiteStatique::afficher(index_t deltaY, Coordonnees const &decalage) const {
	if(deltaY >= _nb)
		return;
	_image.afficher(this->positionAffichage() - decalage + Coordonnees(deltaY * 64, 0), this->decoupagePerspective() ? _cadres[deltaY] : _cadres[0]);
}

void EntiteStatique::animer() {
	
}

Image const &EntiteStatique::image() const {
	return _image;
}
