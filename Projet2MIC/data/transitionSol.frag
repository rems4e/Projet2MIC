#version 120

#define T_GAUCHE 0
#define T_BAS 1
#define T_ANGLE 2

#define LARGEUR_CASE 64

uniform sampler2D _tex;
uniform vec2 _dim;
uniform vec2 _pos;
uniform vec2 _ecran;

varying vec2 _texCoord;
varying vec4 _color;

uniform float pos;
uniform float dimTrans;

vec2 referentielNiveauVersEcran(vec2 pos) {
	return vec2(pos.x + pos.y, (pos.y - pos.x) / 2) / 2;
}

vec2 referentielEcranVersNiveau(vec2 pos) {
	return vec2(pos.x - 2 * pos.y, pos.x + 2 * pos.y);
}

float dX = LARGEUR_CASE / 2;

void main(void) {	
	vec4 couleur = _color * texture2D(_tex, _texCoord);
	
	float dim = dimTrans * LARGEUR_CASE;
	
	vec2 posDansImage = vec2(gl_FragCoord.x, _ecran.y - gl_FragCoord.y) - _pos - vec2(0, _dim.y) / 2;
	vec2 posRectifiee = referentielEcranVersNiveau(posDansImage);
	
	if(pos == T_ANGLE) {
		if(posRectifiee.y < dX && posRectifiee.y >= 0 && posRectifiee.x >= 0 && posRectifiee.x <= dX) {
			couleur.a *= 1.0 - posRectifiee.x / dX;
			couleur.a *= 1.0 - posRectifiee.y / dX;
		}
		else
			discard;
	}
	else {
		float distance;
		
		if(pos == T_GAUCHE) {
			if(posRectifiee.x < dX && posRectifiee.x >= 0 && posRectifiee.y >= 0 && posRectifiee.y <= dim) {
				distance = 1.0 - posRectifiee.x / dX;
			}
			else
				discard;
		}
		else if(pos == T_BAS) {
			if(posRectifiee.y < dX && posRectifiee.y >= 0 && posRectifiee.x >= 0 && posRectifiee.x <= dim) {
				distance = 1.0 - posRectifiee.y / dX;
			}
			else
				discard;
		}
		
		couleur.a *= distance;
	}
	
	gl_FragColor = couleur;
}