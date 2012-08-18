/*
 *  VueDefilante.h
 *  Jeu C++
 *
 *  Created by Rémi on 26/01/11.
 *  Copyright 2011 Rémi Saurel. All rights reserved.
 *
 */

#ifndef VUE_DEFILANTE_H
#define VUE_DEFILANTE_H

#include "VueInterface.h"

class VueDefilante : public VueInterface {
public:
	VueDefilante(Rectangle const &cadre, VueInterface *vue);
	virtual ~VueDefilante();
	
	inline bool barresMasqueesAutomatiquement() const { return _barresMasquees; }
	inline void masquerBarresAutomatiquement(bool masq) { _barresMasquees = masq; }
	
	void definirCadre(Rectangle const &c);

	inline glm::vec2 const &decalage() const { return _decalage; }
	void definirDecalage(glm::vec2 const &d);

	glm::vec2 const dimensionsTotales() const { return _vue->cadre().dimensions(); }
	void definirDimensionsTotales(glm::vec2 const &dim);

	inline void ajouterEnfant(VueInterface &v) { _vue->ajouterEnfant(v); }
	inline void supprimerEnfant(VueInterface &v) { _vue->supprimerEnfant(v); }

	inline VueInterface *sousVueAvecTag(long t) { return _vue->sousVueAvecTag(t); }
	inline VueInterface *vue() { return _vue; }

	//inline Rectangle cadre() const { return _cadre + _decalage

protected:
	bool _barresMasquees;
	glm::vec2 _decalage;
	VueInterface *_vue;
	
	bool _fH, _fB, _c, _depC;
	bool _clic2;
	
	bool _fAff, _cAff;
	
	double _hCurseur, _pCurseur;
	double _pClic;

	virtual void gestionClavier();
	virtual void gestionSouris(bool vueActuelle, glm::vec2 const &souris, bool gauche, bool droit);
	virtual void dessiner();
	
	VueDefilante(VueDefilante const &);
	VueDefilante &operator=(VueDefilante const &);
	
	static Image _curseurH, _curseurB, _curseurM;
	static Image _barreH, _barreB, _barreBB, _barreBH, _barreM;
	static Image _desact;
};

#endif
