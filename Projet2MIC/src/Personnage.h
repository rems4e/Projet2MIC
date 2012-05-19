//
//  Personnage.h
//  Projet2MIC
//
//  Created by Rémi Saurel on 07/02/12.
//  Copyright (c) 2012 Rémi Saurel. All rights reserved.
//

#ifndef Projet2MIC_Personnage_h
#define Projet2MIC_Personnage_h

#include "EntiteMobile.h"

class Inventaire;
class ObjetInventaire;
class TiXmlElement;

class Personnage : public EntiteMobile {
public:
	enum competences_t {premiereCompetence, force = premiereCompetence, agilite, endurance, nbCompetences};
	class Competences {
	public:
		int operator[](competences_t c) const;
		int &operator[](competences_t c);
		
		bool operator==(Competences const &c) const;
		bool operator<(Competences const &c) const;
		bool operator<=(Competences const &c) const;
		bool operator>(Competences const &c) const;
		bool operator>=(Competences const &c) const;
		
		TiXmlElement *sauvegarde() const;
		void restaurer(TiXmlElement *);
		
	private:
		int _valeurs[nbCompetences];
	};
	
	enum positionTenue_t {premierePositionTenue, brasG = premierePositionTenue, brasD, casque, armure, gants, bottes, nbPositionsTenue};
	virtual ~Personnage();
	
	virtual void animer();
	virtual bool interagir(Personnage *p, bool test) = 0;
	virtual void attaquer(Personnage *p);
	
	virtual Coordonnees origine() const;
	virtual bool centrage() const;
	
	// Le coefficient multiplicateur de la vitesse de déplacement de l'entité
	virtual double vitesse() const;
	
	unsigned int vieActuelle() const;
	void modifierVieActuelle(int delta);
	
	virtual unsigned int vieTotale() const;
	void definirVieTotale(int vie);
	
	virtual bool definirAction(action_t a);
	virtual Niveau::couche_t couche() const;
	
	ObjetInventaire const *tenue(positionTenue_t p) const;
	ObjetInventaire *tenue(positionTenue_t p);
	void definirTenue(positionTenue_t p, ObjetInventaire *o);
	Inventaire *inventaire();
	Inventaire const *inventaire() const;
	
	Competences const &competences() const;
	void definirCompetences(Competences const &c);
	
	virtual bool peutEquiperObjet(ObjetInventaire *objet);
	virtual bool peutEquiperObjet(ObjetInventaire *objet, positionTenue_t pos);
	

protected:
	Personnage(bool decoupagePerspective, Niveau *n, uindex_t index, ElementNiveau::elementNiveau_t, Inventaire *inventaire);
	Personnage(Personnage const &);
	Personnage &operator=(Personnage const &);
	
	Personnage *interagir(bool test);
	virtual void mourir();
	void renaitre();
	
	virtual void jeterObjets() = 0;

private:
	int _vieActuelle;
	int _vieTotale;
	double _vitesse;
	struct DelaisAction {
		horloge_t _cooldown;
		horloge_t _cdAbsolu;
	};
	DelaisAction _delaisAction[nbActions];
	
	Inventaire *_inventaire;
	ObjetInventaire *_tenue[nbPositionsTenue];
	Competences _competences;
	
	Personnage *_cibleAttaque;
};

Personnage::positionTenue_t &operator++(Personnage::positionTenue_t &p);
Personnage::competences_t &operator++(Personnage::competences_t &p);

#endif
