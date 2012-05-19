#version 120

uniform sampler2D _tex;
uniform vec2 _dim;
uniform vec2 _ecran;

varying vec2 _texCoord;
varying vec4 _color;

uniform float temps;
uniform float tonnerre;
uniform vec2 cam;

bool comprisEntre(float v, float min, float max) {
	if(v >= min && v <= max)
		return true;
	return false;
}

float random(vec2 a) {
	a = floor(a);
	float f = cos(dot(sin(a), vec2(12.9898, 78.233))) * 43758.5453;
	return fract(f);
}

float freqMin = 0.2;
float freqMax = 0.2015 + 0.001 * sin(temps / 8);

bool freq(float f) {
	return comprisEntre(f, freqMin, freqMax);
}

void main(void) {
	vec4 couleur = _color * texture2D(_tex, _texCoord);
	
	couleur.a = 0.0;
	if(abs(tonnerre - temps) < 0.2) {
		if(pow(sin((tonnerre - temps) * 20), 2) > 0.15) {
			couleur.rgba = vec4(1.0, 1.0, 1.0, 0.8);
		}
	}
	
	vec2 pCam = vec2(cam.x, -cam.y);
	
	vec2 dir = vec2(sin(temps / 10.0) / 2, 1.0);
	vec2 vitesse = vec2(0, 100);
	vec2 dep = vitesse * temps;
	dep.x += sin(temps / 20.0) * 40.0;
	dep.y += 50 * cos(temps / 10.0);
	
	float nb[6];
	for(int i = 0; i < 6; ++i) {
		nb[i] = random(gl_FragCoord.xy + pCam - i * dir + dep);
	}
	
	int indice = -1;
	for(int i = 0; i < 6; ++i) {
		if(freq(nb[i])) {
			indice = i;
			break;
		}
	}
	
	if(indice != -1) {
		couleur.a = 0.5;
		couleur.rgb *= vec3(0.8);
		
		if(freq(nb[indice]) && ((nb[indice] - freqMin) / (freqMax - freqMin) * _ecran.y >= gl_FragCoord.y))
			discard;
	}
	
	gl_FragColor = couleur;
}
