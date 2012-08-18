/*
 *  ChampDeTexte.h
 *  Jeu C++
 *
 *  Created by Rémi on 12/01/11.
 *  Copyright 2011 Rémi Saurel. All rights reserved.
 *
 */

#ifndef CHAMP_DE_TEXTE_H
#define CHAMP_DE_TEXTE_H

#include "ControleInterface.h"
#include <string>
#include "Unichar.h"

class ChampDeTexte : public ControleInterface {	
public:
	ChampDeTexte(Rectangle const &cadre, police_t police = POLICE_NORMALE, taillePolice_t taillePolice = 12, Couleur const &couleurTexte = Couleur::noir, Couleur const &coulFond = Couleur::blanc);
	virtual ~ChampDeTexte();
	
	inline Unichar valeurTexte() const { return _texte; }
	virtual void definirValeurTexte(Unichar const &txt);

	inline police_t police() const { return _police; }
	inline taillePolice_t taillePolice() const { return _taillePolice; }
	inline Couleur couleur() const { return _couleur; }
	inline Couleur couleurFond() const { return _couleurFond; }
	
	inline void definirValeurIndeterminee(bool v) {
		if(v && !_valeurIndeterminee) {
			this->definirValeurTexte("");
		}
		this->ControleInterface::definirValeurIndeterminee(v);
	}
	inline bool focusClavier() const { return true; }
	inline bool necessiteFocusClavier() const { return true; }
	
protected:
	ChampDeTexte(ChampDeTexte const &);
	ChampDeTexte &operator=(ChampDeTexte const &);
	Unichar _texte;
	police_t _police;
	taillePolice_t _taillePolice;
	Couleur _couleur;
	long _positionCurseur;
	long _tailleSelection;
	double _x;
	Unichar _ancienTexte;
	bool _ancienIndetermine;
	bool _restaurerEtat;
	
	static horloge_t _horlogeCurseur;

	virtual void dessiner();
	void gestionClavier();
	void gestionSouris(bool vueActuelle, glm::vec2 const &souris, bool gauche, bool droit);
	
	VueInterface *gestionClic() {
		return VueInterface::gestionClic();
	}

	void prendreLaMain(VueInterface *ancienne);
	void rendreLaMain(VueInterface *prochaine);
	
	void definirPositionCurseur(long p);
};

class TexteEtiquette : public ChampDeTexte {
public:
	TexteEtiquette(std::string const &texte, glm::vec2 const &position, police_t police = POLICE_NORMALE, taillePolice_t taillePolice = 12, Couleur const &couleurTexte = Couleur::noir);
	virtual ~TexteEtiquette();
	
	virtual void definirValeurTexte(Unichar const &txt);

protected:
	TexteEtiquette(TexteEtiquette const &);
	TexteEtiquette &operator=(TexteEtiquette const &);

	virtual void dessiner();
	inline void gestionClavier() { }
	inline void gestionSouris(bool, glm::vec2 const &, bool, bool) { }
};

class ChampDeNombre : public ChampDeTexte {
public:
	ChampDeNombre(Rectangle const &cadre, police_t police = POLICE_NORMALE, taillePolice_t taillePolice = 12, Couleur const &coul = Couleur::noir, Couleur const &coulFond = Couleur::blanc);
	virtual ~ChampDeNombre();
	
	void rendreLaMain(VueInterface *prochaine);

	inline void definirValeurTexte(Unichar const &v) { this->definirValeurDouble(texteVersNombre(v)); }
	
	double valeurDouble() const;
	void definirValeurDouble(double nombre);
	
	int valeurInt() const;
	void definirValeurInt(int nombre);
	
	long valeurLong() const;
	void definirValeurLong(long nombre);

	double min() const { return _min; }
	void definirMin(double min);
	
	double max() const { return _max; }
	void definirMax(double max);
	
	bool nombresDecimaux() const;
	void definirNombresDecimaux(bool d);
	
	inline void definirValeurIndeterminee(bool v) {
		if(v && !_valeurIndeterminee) {
			this->definirValeurDouble(0.0);
		}
		this->ChampDeTexte::definirValeurIndeterminee(v);
	}
	
protected:
	bool _nombresDecimaux;
	double _min, _max, _nombre;

	void gestionClavier();	
};

#endif
