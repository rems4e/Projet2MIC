/*
 *  VueInterface.h
 *  Jeu C++
 *
 *  Created by Rémi on 02/01/11.
 *  Copyright 2011 Rémi Saurel. All rights reserved.
 *
 */

#ifndef VUE_INTERFACE_H
#define VUE_INTERFACE_H

#include <list>
#include "Geometrie.h"
#include "Ecran.h"
#include "Session.h"
#include "fonctions.h"
#include "Unichar.h"
#include "Constantes.h"

class ControleInterface;

class VueInterface {
	friend bool Session::boucle(horloge_t const freq, bool continuer);
public:
	typedef long tag_t;
	
	VueInterface(Rectangle const &cadre, Couleur const &couleurFond = Couleur(255, 0));
	virtual ~VueInterface();
	
	virtual void ajouterEnfant(VueInterface &v);
	virtual void supprimerEnfant(VueInterface &v);
	
	VueInterface *parent() { return _parent; }
	VueInterface const *parent() const { return _parent; }
	void definirParent(VueInterface *v);
	
	bool appartientDescendence(VueInterface *vue) const;
	
	inline virtual Rectangle const &cadre() const { return _cadre; }
	inline virtual void definirCadre(Rectangle const &c) { _cadre = c; }
	
	inline glm::vec2 position() const { return this->cadre().origine(); }
	inline void definirPosition(glm::vec2 const &p) { this->definirCadre(Rectangle(p.x, p.y, this->cadre().largeur, this->cadre().hauteur)); }
	
	inline virtual glm::vec2 dimensions() const { return glm::vec2(this->cadre().largeur, this->cadre().hauteur); }
	inline void definirDimensions(glm::vec2 const &d) { this->definirCadre(Rectangle(this->cadre().gauche, this->cadre().haut, d.x, d.y)); }

	glm::vec2 positionAbsolue() const;
	Rectangle cadreAbsolu() const;
	
	inline bool actif() const { return _actif; }
	inline void definirActif(bool actif) {
		if(actif)
			this->activer();
		else
			this->desactiver();
	}
	inline void activer() { _actif = true; }
	inline void desactiver() {
		if(_vueActuelle == this) {
			this->rendreLaMain(0);
			_vueActuelle = 0;
			_actif = false;
		}
		else {
			_actif = false;
		}
	}
	
	inline bool masque() const { return _masque; }
	virtual void definirMasque(bool masque);

	inline std::list<VueInterface *> &objets() { return _objets; }
	
	static inline VueInterface *vueActuelle() { return VueInterface::_vueActuelle; }
	static void definirVueActuelle(VueInterface *vue);
	
	inline bool clic() const { return _clic; }
	
	inline virtual Unichar const &description() const { return _description; }
	inline void definirDescription(Unichar const &description) { _description = description; }
	
	inline tag_t const &tag() const { return _tag; }
	inline void definirTag(tag_t const &t) { _tag = t; }
	
	virtual VueInterface *sousVueAvecTag(tag_t t);
	ControleInterface *controleAvecTag(tag_t t);
	
	inline int couche() const { return _couche; }
	void definirCouche(int couche);
	
	inline virtual bool focusClavier() const { return false; }
	inline virtual bool necessiteFocusClavier() const { return false; }
	
	VueInterface *premierParentFocus();
	
protected:
	VueInterface(VueInterface const &);
	VueInterface &operator=(VueInterface const &);
	
	VueInterface *_parent;
	
	Couleur _couleurFond;
	std::list<VueInterface *> _objets;
	
	Unichar _description;
	
	bool _masque;
	bool _actif;
	bool _clic;
	
	tag_t _tag;
	int _couche;
	
	static VueInterface *_vueActuelle;
	
	virtual void rendreLaMain(VueInterface *prochaine);
	virtual void prendreLaMain(VueInterface *ancienne);
	
	virtual void gestionClavier();
	virtual void gestionSouris(bool vueActuelle, glm::vec2 const &souris, bool gauche, bool droit);
	virtual void dessiner();
	virtual void preparationDessin();
	
	virtual VueInterface *gestionClic();
		
private:
	void afficher(glm::vec2 const &, int, int &);
	void gestionGestionClavier();
	VueInterface *gestionSurvol();
	static VueInterface *_dernierSurvole;
	static horloge_t *_tempsSurvol;

	Rectangle _cadre;
};

inline std::ostream &operator<<(std::ostream &s, VueInterface const &v) {
	return s << "{" << &v << " " << v.cadre() << " " << v.description() << "}";
}

#endif
