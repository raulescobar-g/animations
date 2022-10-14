#version 120

attribute vec4 aPos;
attribute vec3 aNor;
attribute vec2 aTex;
attribute vec3 dp1;
attribute vec3 dn1;
attribute vec3 dp2;
attribute vec3 dn2;

uniform mat4 P;
uniform mat4 MV;

uniform float t;

varying vec3 vPos;
varying vec3 vNor;
varying vec2 vTex;

void main()
{
	float a = 0.5 + cos(t * 1.0) / 2.0;
	float b = 0.5 + sin(t * 2.0) / 2.0;

	vec3 pos = aPos.xyz + dp1 * a + dp2 * b;
	vec3 n = aNor + dn1 * a + dn2 * b;

	vec4 posCam = MV * vec4(pos, 1.0);
	vec3 norCam = (MV * vec4(n, 0.0)).xyz;
	gl_Position = P * posCam;
	vPos = posCam.xyz;
	vNor = norCam;
	vTex = aTex;
}
