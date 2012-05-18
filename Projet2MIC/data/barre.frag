#version 120

uniform sampler2D _tex;
uniform vec2 _dim;
uniform vec2 _ecran;

uniform float avancement;
uniform float _temps;

void main(void) {
	vec4 couleur = gl_Color;
	vec2 pos = (gl_FragCoord.xy - _pos) / _dim;
	
	if(pos.x > avancement) {
		float nuance = pow(abs(pos.y - 0.5) * 2, 0.7);
		nuance *= 0.4;
		couleur = vec4(nuance, nuance, nuance, 1.0);
	}
	else {
		couleur.rgb = vec3(0.2);
		couleur.r += (1.0 + sin((pos.x - 0.1 * _temps) / _dim.x * 200 * 128 + pos.y * 4)) * 0.5;
		couleur.rgb /= 1.0 / pow(min(abs(1.0 - pos.y), pos.y), 1.0/3.0);
	}
	
	gl_FragColor = couleur;
}