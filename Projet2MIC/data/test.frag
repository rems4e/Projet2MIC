#version 120

uniform sampler2D _tex;
uniform vec2 _ecran;

uniform vec3 positionLumiere;
uniform vec3 lumiere;
uniform vec3 lumiereAmbiante;

varying vec2 _texCoord;
varying vec4 _color;
varying vec3 _normale;
varying vec3 _positionModele;

uniform mat4 invProjection;
uniform float lightAttenuation;

vec3 CalcCameraSpacePosition() {
	vec4 ndcPos;
	ndcPos.xy = ((gl_FragCoord.xy / _ecran.xy) * 2.0) - 1.0;
	ndcPos.z = (2.0 * gl_FragCoord.z - gl_DepthRange.near - gl_DepthRange.far) / (gl_DepthRange.far - gl_DepthRange.near);
	ndcPos.w = 1.0;
	
	vec4 clipPos = ndcPos / gl_FragCoord.w;
	
	return vec3(invProjection * clipPos);
}

vec3 ApplyLightIntensity(in vec3 cameraSpacePosition, out vec3 lightDirection) {
	vec3 lightDifference =  positionLumiere - cameraSpacePosition;
	float lightDistanceSqr = dot(lightDifference, lightDifference);
	lightDirection = lightDifference * inversesqrt(lightDistanceSqr);
	
	float distFactor = sqrt(lightDistanceSqr);
	
	return lumiere * (1 / ( 1.0 + lightAttenuation * distFactor));
}

void main(void) {
	/*//vec4 couleur = _color;// * texture2D(_tex, _texCoord);
	
	float cosAngIncidence = dot(normalize(_normale), normalize(positionLumiere - _positionModele));
	cosAngIncidence = clamp(cosAngIncidence, 0.0, 1.0);
	
	gl_FragColor = vec4(lumiere * _color.rgb * cosAngIncidence + lumiereAmbiante * _color.rgb, 1.0);*/
	
	vec3 cameraSpacePosition = CalcCameraSpacePosition();
	
	vec3 lightDir = vec3(0.0);
	vec3 attenIntensity = ApplyLightIntensity(cameraSpacePosition, lightDir);
	
	float cosAngIncidence = dot(normalize(_normale), normalize(lightDir));
	cosAngIncidence = clamp(cosAngIncidence, 0, 1);
	
	gl_FragColor = vec4((_color.rgb * attenIntensity * cosAngIncidence) + (_color.rgb * lumiereAmbiante), 1.0);
}