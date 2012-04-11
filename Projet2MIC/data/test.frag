#version 120
uniform sampler2D tex;
uniform vec2 dimTex;

void main(void) {
	vec4 couleur = gl_Color * gl_TexCoord[0].st;
	gl_FragColor = couleur;
}