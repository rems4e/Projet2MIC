#version 120

uniform sampler2D _tex;
uniform vec2 _dim;
uniform vec2 _ecran;
uniform float _temps;

varying vec2 _texCoord;
varying vec4 _color;

/*void main(void) {
	vec4 couleur = _color * texture2D(_tex, _texCoord);
	float moyenne = dot(couleur.rgb, vec3(1.0)) / 3.0;
	couleur.rgb = moyenne * vec3(1.0);
	couleur.b = pow(sin(_temps), 2);
	gl_FragColor = couleur;
}*/

float stime=sin(_temps);
float ctime=cos(_temps);

float inObj(in vec3 p){
	float oP=length(p);
	p.x=sin(p.x)+stime;
	p.z=sin(p.z)+ctime;
	return float(min(length(p)-1.5-sin(oP-_temps*4.0),p.y+3.0));
}

void main(void){
	vec2 vPos=-1.0+2.0*gl_FragCoord.xy/_ecran.xy;
	
	//Camera animation
	vec3 vuv=vec3(stime,1,0);//view up vector
	vec3 vrp=vec3(sin(_temps*0.7)*10.0,0,cos(_temps*0.9)*10.0); //view reference point
	vec3 prp=vec3(sin(_temps*0.7)*20.0+vrp.x+20.0,
		      stime*4.0+4.0+vrp.y+3.0,
		      cos(_temps*0.6)*20.0+vrp.z+14.0); //camera position
	
	//Camera setup
	vec3 vpn=normalize(vrp-prp);
	vec3 u=normalize(cross(vuv,vpn));
	vec3 v=cross(vpn,u);
	vec3 vcv=(prp+vpn);
	vec3 scrCoord=vcv+vPos.x*u*_ecran.x/_ecran.y+vPos.y*v;
	vec3 scp=normalize(scrCoord-prp);
	
	//Raymarching
	const vec3 e = vec3(0.1,0,0);
	const float maxd=200.0;
	
	float s=0.1;
	vec3 c,p,n;
	float a = 1.0;
	
	//speed optimization -advance ray (simple raytracing) until plane y=2.5
	float f=-(prp.y-2.5)/scp.y;
	if (f>0.0) p=prp+scp*f;
	else f=maxd;
	
	for(int i=0;i<256;i++){
		if (abs(s)<.01||f>maxd) break;
		f+=s;
		p=prp+scp*f;
		s=inObj(p);
	}
	
	if (f<maxd){
		if(p.y<-2.5){
			if (fract(p.x*.5)>.5) {
				if (fract(p.z*.5)>.5) {
					c=vec3(0,0,0);
				}
				else {
					c=vec3(1,1,1);
				}
			}
			else {
				if (fract(p.z*.5)>.5) {
					c = vec3(1,1,1);
				}
				else {
					c = vec3(0,0,0);
				}
			}
			n=vec3(0,1,0);
			a = 0;
		}
		else{
			float d=length(p);
			c=vec3((sin(d*.25-_temps*4.0)+1.0)/2.0,
			       (stime+1.0)/2.0,
			       (sin(d-_temps*4.0)+1.0)/2.0); //color
			n=normalize(
				    vec3(s-inObj(p-e.xyy),
					 s-inObj(p-e.yxy),
					 s-inObj(p-e.yyx)));
		}
		float b=dot(n,normalize(prp-p));
		gl_FragColor=vec4((b*c+pow(b,54.0))*(1.0-f*.005),a);
		if(a == 0)
			gl_FragColor = _color * texture2D(_tex, _texCoord);
	}
	else gl_FragColor = _color * texture2D(_tex, _texCoord);
//vec4(1.0,0,0,1);
}
