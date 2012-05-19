
#ifndef Projet2MIC_TableauBord_h
#define Projet2MIC_TableauBord_h

#include "Image.h"
#include "Texte.h"

class Joueur;

class TableauDeBord {
public:
	TableauDeBord(Joueur *j);
	virtual ~TableauDeBord();
	
	void afficher();
	
	dimension_t hauteur() const;
	
protected: 
	TableauDeBord(TableauDeBord const &);
	TableauDeBord &operator=(TableauDeBord const &);
private:
	Joueur *_joueur;
	Image _fond;
	Texte _interactionJoueur;
};

#endif
