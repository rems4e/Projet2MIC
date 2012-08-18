/*
 *  ChampDeTexte.cpp
 *  Jeu C++
 *
 *  Created by Rémi on 05/05/08.
 *  Copyright 2008 Saurel Rémi. All rights reserved.
 *
 */

#include "ObjetInterface.h"
#include "Affichage.h"
#include "Session.h"

horloge_t ChampDeTexte::_horlogeCurseur = 0.0f;

ChampDeTexte::ChampDeTexte(Rectangle const &cadre, police_t police, taillePolice_t taillePolice, Couleur const &couleurTexte, Couleur const &couleurFond) : ControleInterface(cadre, couleurFond), _texte(), _police(police), _taillePolice(taillePolice), _couleur(couleurTexte), _positionCurseur(0L), _tailleSelection(0L), _x(0), _ancienTexte(), _ancienIndetermine(), _restaurerEtat() {
}

ChampDeTexte::~ChampDeTexte() {}

void ChampDeTexte::definirValeurTexte(Unichar const &txt) {
	_texte = txt;
	this->definirPositionCurseur(_texte.size());
	_valeurIndeterminee = false;
}

void ChampDeTexte::dessiner() {
	this->ControleInterface::dessiner();
	
	Affichage::afficherRectangle(Rectangle(0, 0, this->cadre().largeur, 1), Couleur::noir);
	Affichage::afficherRectangle(Rectangle(0, this->cadre().hauteur - 1, this->cadre().largeur, 1), Couleur::noir);
	Affichage::afficherRectangle(Rectangle(0, 0, 1, this->cadre().hauteur), Couleur::noir);
	Affichage::afficherRectangle(Rectangle(this->cadre().largeur - 1, 0, 1, this->cadre().hauteur), Couleur::noir);
	
	Couleur const &couleur = _actif ? _couleur : Couleur::gris;
	
	Texte txt(_texte, _police, _taillePolice, couleur);
	if(VueInterface::vueActuelle() == this) {
		if(horloge() - _horlogeCurseur >= 1.2f) {
			_horlogeCurseur = horloge();
		}
		if(horloge() - _horlogeCurseur < 0.6f) {
			Affichage::afficherRectangle(Rectangle(Texte::dimensions(_texte.substr(0, _positionCurseur), _police, _taillePolice).x - _x, 2, 1, this->dimensions().y - 4), Couleur::noir);
		}
	}
	else if(this->valeurIndeterminee()) {
		txt.definir(Couleur::gris);
		txt.definir(TRAD("ctxt <valeurs multiples>"));
	}
	txt.afficher(glm::vec2(-_x, 2));
}

void ChampDeTexte::prendreLaMain(VueInterface */*ancienne*/) {
	_horlogeCurseur = horloge();
	_ancienTexte = _texte;
	_ancienIndetermine = this->valeurIndeterminee();
	_restaurerEtat = false;
	this->definirPositionCurseur(_texte.size());
}

void ChampDeTexte::rendreLaMain(VueInterface */*prochaine*/) {
	if(_restaurerEtat) {
		_texte = _ancienTexte;
		this->definirValeurIndeterminee(_ancienIndetermine);
	}
	else {
		this->executerAction();
		this->definirValeurIndeterminee(false);
	}
}

void ChampDeTexte::gestionClavier() {
	if(Session::evenement(Session::T_ESC)) {
		_restaurerEtat = true;
		VueInterface::definirVueActuelle(0);
		return;
	}
	if(Session::evenement(Session::T_ENTREE)) {
		VueInterface::definirVueActuelle(0);
		this->definirValeurIndeterminee(false);
		return;
	}
	if(Session::evenementDiscret(Session::T_EFFACER) && _positionCurseur > 0) {
		_texte.replace(_texte.begin() + _positionCurseur - 1, _texte.begin() + _positionCurseur, std::string());
		this->definirPositionCurseur(_positionCurseur - 1);
	}
	else if(Session::evenementDiscret(Session::T_GAUCHE)) {
		this->definirPositionCurseur(_positionCurseur - 1);
	}
	else if(Session::evenementDiscret(Session::T_DROITE)) {
		this->definirPositionCurseur(_positionCurseur + 1);
	}
	else if(Session::evenementDiscret(Session::T_HAUT)) {
		this->definirPositionCurseur(0);
	}
	else if(Session::evenementDiscret(Session::T_BAS)) {
		this->definirPositionCurseur(_texte.size());
	}
	else {
		char c = 0;
		for(Session::evenement_t i = Session::T_a; i <= Session::T_z; ++i) {
			if(Session::evenementDiscret(i)) {
				c = i + (Session::modificateurTouches(Session::M_MAJ) ? 'A' : 'a') - Session::T_a;
			}
		}
		for(Session::evenement_t i = Session::T_0; i <= Session::T_9; ++i) {
			if(Session::evenementDiscret(i)) {
				c = i + '0' - Session::T_0;
			}
		}
		if(Session::evenementDiscret(Session::T_ESPACE))
			c = ' ';
		else if(Session::evenementDiscret(Session::T_MOINS))
			c = '-';
		else if(Session::evenementDiscret(Session::T_PLUS))
			c = '+';
		else if(Session::evenementDiscret(Session::T_SLASH))
			c = '/';
		else if(Session::evenementDiscret(Session::T_ASTERISQUE))
			c = '*';
		else if(Session::evenementDiscret(Session::T_EGAL))
			c = '=';
		else if(Session::evenementDiscret(Session::T_INFERIEUR))
			c = '<';
		else if(Session::evenementDiscret(Session::T_SUPERIEUR))
			c = '>';
		else if(Session::evenementDiscret(Session::T_TAB))
			c = '\t';
		else if(Session::evenementDiscret(Session::T_TIRET_BAS))
			c = '_';
		else if(Session::evenementDiscret(Session::T_POINT))
			c = '.';
		else if(Session::evenementDiscret(Session::T_VIRGULE))
			c = ',';
		else if(Session::evenementDiscret(Session::T_DEUX_POINTS))
			c = ':';
		else if(Session::evenementDiscret(Session::T_POINT_VIRGULE))
			c = ';';
		else if(Session::evenementDiscret(Session::T_POINT_EXCLAMATION))
			c = '!';
		else if(Session::evenementDiscret(Session::T_POINT_INTERROGATION))
			c = '?';
		else if(Session::evenementDiscret(Session::T_ESPERLUETTE))
			c = '&';
		else if(Session::evenementDiscret(Session::T_GUILLEMETS))
			c = '"';
		else if(Session::evenementDiscret(Session::T_APOSTROPHE))
			c = '\'';
		else if(Session::evenementDiscret(Session::T_DOLLAR))
			c = '$';
		else if(Session::evenementDiscret(Session::T_HASH))
			c = '#';
		else if(Session::evenementDiscret(Session::T_BACKSLASH))
			c = '\\';
		else if(Session::evenementDiscret(Session::T_PARENTHESE_G))
			c = '(';
		else if(Session::evenementDiscret(Session::T_PARENTHESE_D))
			c = ')';
		else if(Session::evenementDiscret(Session::T_CROCHET_G))
			c = '[';
		else if(Session::evenementDiscret(Session::T_CROCHET_D))
			c = ']';

		if(c) {
			char ajout[2] = {c, 0};
			_texte.replace(_texte.begin() + _positionCurseur, _texte.begin() + _positionCurseur, ajout);
			this->definirPositionCurseur(_positionCurseur + 1);
			this->definirValeurIndeterminee(false);
		}
	}
}

void ChampDeTexte::gestionSouris(bool/*vueActuelle*/, glm::vec2 const &/*souris*/, bool gauche, bool /*droit*/) {
	if(gauche) {
		//souris;
	}
}

void ChampDeTexte::definirPositionCurseur(long p) {
	p = std::max(0L, std::min<long>(p, this->valeurTexte().size()));
	
	double dd = Texte::dimensions(_texte.substr(0, p), _police, _taillePolice).x;
	if(dd - _x < 0 ||  dd - _x > std::max<coordonnee_t>(this->dimensions().x - 8, 0.0)) {
		double l = 0;
		long i;
		for(i = p - 1; i >= 0; --i) {
			l += Texte::dimensions(_texte.substr(i, 1), _police, _taillePolice).x;
			if(l >= this->dimensions().x - 8) {
				break;
			}
		}
		
		_x = Texte::dimensions(_texte.substr(0, i + 1), _police, _taillePolice).x - 4;
	}
	_positionCurseur = p;
	
	_horlogeCurseur = horloge();
}

#pragma mark -
#pragma mark Texte étiquette

TexteEtiquette::TexteEtiquette(std::string const &texte, glm::vec2 const &position, police_t police, taillePolice_t taillePolice, Couleur const &couleurTexte) : ChampDeTexte(Rectangle(position, glm::vec2(std::numeric_limits<double>::max() / 2, 10)), police, taillePolice, couleurTexte, Couleur::transparent) {
	this->definirValeurTexte(texte);
}

TexteEtiquette::~TexteEtiquette() {
	
}

void TexteEtiquette::definirValeurTexte(Unichar const &txt) {
	Unichar t(txt);
	
	double largeur = 0, dim = 0;
	for(index_t i = 0, dernierEspace = 0; i < t.size(); ++i) {
		if(t[i] == ' ')
			dernierEspace = i;
		double dl = Texte::dimensions(t.substr(i, 1), _police, _taillePolice).x;

		if(largeur + dl > this->cadre().largeur) {
			t.replace(dernierEspace, 1, '\n');
			dim = std::max(dim, largeur);
			largeur = 0;
			++i;
		}
		else {
			largeur += dl;
			dim = std::max(dim, largeur);
		}
	}
	
	this->definirDimensions(glm::vec2(dim, Texte::dimensions(t, this->police(), this->taillePolice()).y));
	this->ChampDeTexte::definirValeurTexte(t);
}

void TexteEtiquette::dessiner() {
	Texte::afficher(this->valeurTexte(), _police, _taillePolice, this->actif() ? this->couleur() : Couleur::gris, glm::vec2(0));
}

#pragma mark -
#pragma mark Champ de nombre

ChampDeNombre::ChampDeNombre(Rectangle const &cadre, police_t police, taillePolice_t taillePolice, Couleur const &coul, Couleur const &coulFond) : ChampDeTexte(cadre, police, taillePolice, coul, coulFond), _nombre(0.0), _nombresDecimaux(true), _min(-1.0E100), _max(1.0E100) {
	this->definirValeurDouble(0.0);
}

ChampDeNombre::~ChampDeNombre() { }

void ChampDeNombre::rendreLaMain(VueInterface *prochaine) {
	double nombre = 0.0;

	if(_texte.size()) {
		Unichar::iterator i = _texte.begin();
		bool neg = *i == '-';
		if(neg || *i == '+')
			++i;
		unsigned long nb = 0;
		while(i != _texte.end() && *i >= '0' && *i <= '9')
			nb = nb * 10 + *i++ - '0';
		nombre = nb;
		if(i != _texte.end()) {
			if(*i == ',' || *i == '.') {
				++i;
				int nbC = 0;
				nb = 0;
				while(i != _texte.end() && *i >= '0' && *i <= '9' && ++nbC)
					nb = nb * 10 + *i++ - '0';
				nombre += double(nb) * std::pow(10.0, -nbC);
			}
		}
		if(neg)
			nombre *= -1;
	}
	
	this->definirValeurDouble(nombre);
	this->ChampDeTexte::rendreLaMain(prochaine);
}

double ChampDeNombre::valeurDouble() const {
	if(!_texte.size())
		return 0.0;
	return _nombre;
}

int ChampDeNombre::valeurInt() const {
	return static_cast<int>(this->valeurDouble());
}

long ChampDeNombre::valeurLong() const {
	return static_cast<long>(this->valeurDouble());
}
void ChampDeNombre::definirValeurLong(long nombre) {
	this->definirValeurDouble(nombre);
}

void ChampDeNombre::definirValeurDouble(double nombre) {
	_nombre = std::min(std::max(_min, nombre), _max);
	if(!_nombresDecimaux)
		_nombre = trunc(_nombre);
	this->ChampDeTexte::definirValeurTexte(nombreVersTexte(_nombre, -1));
}

void ChampDeNombre::definirValeurInt(int nombre) {
	this->definirValeurDouble(nombre);
}

void ChampDeNombre::definirMin(double nb) {
	if(nb <= _max) {
		_min = nb;
		this->definirValeurDouble(std::max(this->valeurDouble(), this->min()));
	}
}

void ChampDeNombre::definirMax(double nb) {
	if(nb >= _min) {
		_max = nb;
		this->definirValeurDouble(std::min(this->valeurDouble(), this->max()));
	}
}

bool ChampDeNombre::nombresDecimaux() const {
	return _nombresDecimaux;
}

void ChampDeNombre::definirNombresDecimaux(bool d) {
	_nombresDecimaux = d;
}

void ChampDeNombre::gestionClavier() {	
	if(Session::evenement(Session::T_ESC)) {
		_restaurerEtat = true;
		VueInterface::definirVueActuelle(0);
		return;
	}
	if(Session::evenement(Session::T_ENTREE)) {
		VueInterface::definirVueActuelle(0);
		this->definirValeurIndeterminee(false);
		return;
	}
	
	if(Session::evenementDiscret(Session::T_EFFACER) && _positionCurseur > 0) {
		_texte.replace(_texte.begin() + _positionCurseur - 1, _texte.begin() + _positionCurseur, std::string());
		this->definirPositionCurseur(_positionCurseur - 1);
	}
	else if(Session::evenementDiscret(Session::T_GAUCHE)) {
		this->definirPositionCurseur(_positionCurseur - 1);
	}
	else if(Session::evenementDiscret(Session::T_DROITE)) {
		this->definirPositionCurseur(_positionCurseur + 1);
	}
	else if(Session::evenementDiscret(Session::T_HAUT)) {
		this->definirPositionCurseur(0);
	}
	else if(Session::evenementDiscret(Session::T_BAS)) {
		this->definirPositionCurseur(_texte.size());
	}
	else {
		std::string texte(_texte);
		char c = 0;
		for(Session::evenement_t i = Session::T_0; i <= Session::T_9; ++i) {
			if(Session::evenementDiscret(i)) {
				c = i + '0' - Session::T_0;
			}
		}
		if(Session::evenementDiscret(Session::T_MOINS) && _positionCurseur == 0 && (_texte.empty() || _texte.substr(0, 1) != "-") && this->min() < 0)
			c = '-';
		else if((Session::evenementDiscret(Session::T_POINT) || Session::evenementDiscret(Session::T_VIRGULE)) && _texte.utf8().find('.') == std::string::npos)
			c = ',';
		
		if(c) {
			char ajout[2] = {c, 0};
			texte.replace(texte.begin() + _positionCurseur, texte.begin() + _positionCurseur, ajout);
			_texte = texte;
			this->definirPositionCurseur(_positionCurseur + 1);
			this->definirValeurIndeterminee(false);
		}
	}	
}
