#version 120

uniform sampler2D _tex;
uniform vec2 _pos;
uniform vec2 _dim;
uniform vec2 _ecran;

void main(void) {
	vec4 couleur = gl_Color * texture2D(_tex, gl_TexCoord[0].st);
	gl_FragColor = vec4(1.0, couleur.bga);
}