#version 120
uniform sampler2D _tex;
uniform vec2 _pos;
uniform vec2 _dim;
uniform vec2 _ecran;

uniform float rayon;

void main(void) {
	vec4 couleur = vec4(0.0);
	vec4 dif = vec4(1.0, 1.0, 1.0, 1.0);
	float i, j, nb = 0;
	for(i = -rayon; i <= rayon; i += 1.0) {
		for(j = -rayon; j <= rayon; j += 1.0, ++nb) {
			vec2 pTex = gl_TexCoord[0].st + vec2(i / _dim.x, j / _dim.y);
			pTex.x = clamp(pTex.x, 0.0, 1.0);
			pTex.y = clamp(pTex.y, 0.0, 1.0);
			couleur += texture2D(_tex, pTex);
		}
	}
	couleur /= nb;
	couleur *= gl_Color;
	
	gl_FragColor = couleur;
}