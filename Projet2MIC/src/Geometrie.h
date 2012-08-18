//
//  Geometrie.h
//  Projet2MIC
//
//  Créé par Marc Promé et Rémi Saurel.
//  Ce fichier et son contenu sont librement distribuables, modifiables et utilisables pour toute œuvre non commerciale, à condition d'en citer les auteurs.
//


#ifndef EN_TETE_Geometrie
#define EN_TETE_Geometrie

#include <iostream>

#include "Coordonnees.h"
#include "Rectangle.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

inline glm::vec3 spheriquesVersCartesiennes(glm::vec3 const &coordonnees) {
	coordonnee_t rho = coordonnees.x;
	coordonnee_t theta = (coordonnees.y + 90) / 180 * M_PI;
	coordonnee_t phi = coordonnees.z / 180 * M_PI;
	
	coordonnee_t sinTheta = std::sin(theta);
	coordonnee_t cosTheta = std::cos(theta);
	coordonnee_t cosPhi = std::cos(phi);
	coordonnee_t sinPhi = std::sin(phi);
	
	return glm::vec3(sinTheta * cosPhi, cosTheta, sinTheta * sinPhi) * rho;
}

inline glm::mat4 projection(float angle, float ratio, float near, float far) {
	return glm::perspective(angle, ratio, near, far);
}

inline glm::mat4 translation(glm::vec3 const &vec) {
	return glm::translate(glm::mat4(), vec);
}

inline glm::mat4 homothetie(glm::vec3 const &vec) {
	return glm::scale(glm::mat4(), vec);
}

inline glm::mat4 rotation(float angle, glm::vec3 vec) {
	return glm::rotate(glm::mat4(), angle, vec);
}

inline glm::mat4 camera(glm::vec3 const &position, glm::vec3 const &cible, glm::vec3 const &haut) {
	return glm::lookAt(position, cible, haut);
}

inline glm::mat4 cameraSphere(glm::vec3 const &posRelativeSphere, glm::vec3 const &cible, glm::vec3 const &haut) {
	glm::vec3 directionCartesienne = spheriquesVersCartesiennes(posRelativeSphere);
	return camera(directionCartesienne + cible, cible, haut);
}

inline std::ostream &operator<<(std::ostream &s, glm::vec3 const &c) {
	return s << '(' << c.x << ", " << c.y << ", " << c.z << ')';
}

inline std::ostream &operator<<(std::ostream &s, glm::vec2 const &c) {
	return s << '(' << c.x << ", " << c.y << ')';
}
	
inline std::ostream &operator<<(std::ostream &s, glm::ivec2 const &c) {
	return s << '(' << c.x << ", " << c.y << ')';
}

inline std::ostream &operator<<(std::ostream &s, Rectangle const &r) {
	return s << "{(" << r.gauche << ", " << r.haut << "), (" << r.largeur << ", " << r.hauteur << ")}";
}
	
	
#endif
