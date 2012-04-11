#version 120
uniform sampler2D tex;
uniform vec2 dimTex;

uniform float rayon = 2;

void main(void) {
	vec4 couleur = vec4(0.0);
	vec4 dif = vec4(1.0, 1.0, 1.0, 1.0);
	float i, j, nb = 0;
	for(i = -rayon; i <= rayon; ++i) {
		for(j = -rayon; j <= rayon; ++j, ++nb) {
			vec2 pTex = gl_TexCoord[0].st + vec2(i / dimTex.x, j / dimTex.y);
			pTex.x = clamp(pTex.x, 0.0, 1.0);
			pTex.y = clamp(pTex.y, 0.0, 1.0);
			couleur += texture2D(tex, pTex);
		}
	}
	couleur /= nb;
	couleur *= gl_Color;
	
	gl_FragColor = couleur;
}