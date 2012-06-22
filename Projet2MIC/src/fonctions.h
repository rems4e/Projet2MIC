//
//  fonctions.h
//  Projet2MIC
//
//  Créé par Marc Promé et Rémi Saurel.
//  Ce fichier et son contenu sont librement distribuables, modifiables et utilisables pour toute œuvre non commerciale, à condition d'en citer les auteurs.
//


#ifndef EN_TETE_FONCTIONS
#define EN_TETE_FONCTIONS

#include <string>

template <typename T>
T signe(T const &t) {
	return (t > 0)  ? 1 : ((t < 0) ? -1 : 0);
}

// Nombre aléatoire entre 0 et (nombreMax - 1)
int nombreAleatoire(int nombreMax);

template <typename T>
bool comprisEntre(T valeur, T borneInf, T borneSup) {
	return valeur >= borneInf && valeur <= borneSup;
}

int caractereVersHexa(char c);
char hexaVersCaractere(int c);
int caractereVersBase64(char c);
char base64VersCaractere(int c);

double texteVersNombre(std::string const &texte);
std::string nombreVersTexte(double nombre, int decimales);
std::string nombreVersTexte(long nombre);

int nombreDecimales(double nombre);

#endif
