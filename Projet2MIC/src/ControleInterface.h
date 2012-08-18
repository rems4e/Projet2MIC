/*
 *  ControleInterface.h
 *  Jeu C++
 *
 *  Created by Rémi on 12/01/11.
 *  Copyright 2011 Rémi Saurel. All rights reserved.
 *
 */

#ifndef CONTROLE_INTERFACE_H
#define CONTROLE_INTERFACE_H

#include "VueInterface.h"

class ControleInterface : public VueInterface {
public:
	typedef void (*ActionParam)(ControleInterface *emetteur, void *parametreAction);
	typedef void (*Action)();

	ControleInterface(Rectangle const &cadre, Couleur const &couleurFond = Couleur(255, 0));
	virtual ~ControleInterface();
	
	inline virtual bool valeurIndeterminee() const { return _valeurIndeterminee; }
	inline virtual void definirValeurIndeterminee(bool i) { _valeurIndeterminee = i; }
	
	inline virtual bool valeurBool() const { return this->valeurInt(); }
	inline virtual int valeurInt() const { return 0; }
	inline virtual long valeurLong() const { return this->valeurInt(); }
	inline virtual double valeurDouble() const { return this->valeurInt(); }
	inline virtual Unichar valeurTexte() const { return nombreVersTexte(this->valeurInt()); }
	
	inline virtual void definirValeurBool(bool valeur) { this->definirValeurInt(valeur); }
	inline virtual void definirValeurInt(int /*valeur*/) {
		_valeurIndeterminee = false;
	}
	inline virtual void definirValeurLong(long valeur) { this->definirValeurInt(static_cast<int>(valeur)); }
	inline virtual void definirValeurDouble(double valeur) { this->definirValeurInt(static_cast<int>(valeur)); }
	inline virtual void definirValeurTexte(Unichar const &valeur) { this->definirValeurDouble(texteVersNombre(valeur)); }
	
	inline Action action() const { return _action; }
	inline ActionParam actionParametres() const { return _actionP; }
	inline void definirAction(Action const &a) { _action = a; _actionP = 0; }
	inline void definirAction(ActionParam const &a) { _actionP = a; _action = 0; }
	inline void *parametreAction() const { return _parametreAction; }
	inline void definirParametreAction(void *p) { _parametreAction = p; }
	
	Unichar &description() const;
	
	inline Session::evenement_t const &raccourci() const { return _raccourci; }
	inline void definirRaccourci(Session::evenement_t const &r) { _raccourci = r; }

	inline Session::modificateur_touche_t const &modificateurRaccourci() const { return _modificateurRaccourci; }
	inline void definirModificateurRaccourci(Session::modificateur_touche_t const &m) { _modificateurRaccourci = m; }

protected:
	bool _valeurIndeterminee;
	
	Action _action;
	ActionParam _actionP;
	void *_parametreAction;
	
	mutable Unichar _desc;
	
	Session::evenement_t _raccourci;
	Session::modificateur_touche_t _modificateurRaccourci;
	
	ControleInterface(ControleInterface const &);
	ControleInterface &operator=(ControleInterface const &);
	
	void executerAction();
	
	VueInterface *gestionClic();
	void gestionClavier();
};

#endif
