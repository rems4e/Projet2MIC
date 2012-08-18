
#ifndef Projet2MIC_TableauBord_h
#define Projet2MIC_TableauBord_h

#include "Image.h"
#include "Texte.h"
#include "VueInterface.h"

class Joueur;

class TableauDeBord : public VueInterface {
public:
	TableauDeBord(Joueur *j);
	virtual ~TableauDeBord();
		
	dimension_t hauteur() const;
	
protected: 
	TableauDeBord(TableauDeBord const &);
	TableauDeBord &operator=(TableauDeBord const &);
	
	void dessiner() override;
	void gestionClavier() override;
	void preparationDessin() override;
private:
	Joueur *_joueur;
	Image _fond;
	Texte _interactionJoueur;
};

#endif
