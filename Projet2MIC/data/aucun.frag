#version 120
uniform sampler2D tex;
uniform vec2 dimText = vec2(0.0, 0.0);

void main(void) {
	vec4 couleur = gl_Color * texture2D(tex, gl_TexCoord[0].st);
	gl_FragColor = couleur;
}