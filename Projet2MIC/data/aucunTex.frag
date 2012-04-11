#version 120
uniform sampler2D tex;

void main(void) {
	vec4 couleur = texture2D(tex, gl_TexCoord[0].st);
	vec4 dif = vec4(1.0, 1.0, 1.0, 1.0);
	//couleur = vec4((dif - couleur).rgb, couleur.a);
	gl_FragColor = couleur;
}