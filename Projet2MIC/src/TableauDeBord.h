
#ifndef Projet2MIC_TableauBord_h
#define Projet2MIC_TableauBord_h

class Joueur;

class TableauDeBord {
public:
	TableauDeBord(Joueur *j) ;
	virtual ~TableauDeBord() ;
	
	void afficher() ;

protected: 
	TableauDeBord(TableauDeBord const &);
	TableauDeBord &operator=(TableauDeBord const &);
private:
	Joueur *_joueur;
};

#endif