#include "TableauDeBord.h"
#include "Geometrie.h"
#include "Ecran.h"
#include "Joueur.h"
#include "Texte.h"
#include "fonctions.h"

TableauDeBord::TableauDeBord(Joueur *j) : _joueur(j) {

}


TableauDeBord::~TableauDeBord() {

}
	
void TableauDeBord::afficher() {
	Rectangle rec(0,Ecran::hauteur()-100,Ecran::largeur(),100);
	Ecran::afficherRectangle(rec, Couleur::noir);
	
	Rectangle barreVieTotale(Ecran::largeur()-280,Ecran::hauteur()-80,200,30);
	Ecran::afficherRectangle(barreVieTotale, Couleur(255,0,0,150));
	
	Rectangle barreVieActuelle(Ecran::largeur()-280,Ecran::hauteur()-80,_joueur->vieActuelle()*200/_joueur->vieTotale(),30);
	Ecran::afficherRectangle(barreVieActuelle, Couleur(230,0,0,255));
	
	Texte chiffresVie(nombreVersTexte(_joueur->vieActuelle())+"/"+nombreVersTexte(_joueur->vieTotale()), POLICE_NORMALE, 16, Couleur::blanc);
	chiffresVie.afficher(barreVieTotale.origine()+(barreVieTotale.dimensions()-chiffresVie.dimensions())/2);
}
