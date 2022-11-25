#version 120

uniform mat4 P;
uniform mat4 MV;
uniform mat4 iMV;

attribute vec3 aPos;
attribute vec3 aNor;

attribute vec4 position;

varying vec3 normal;
varying vec3 pos;


void main(){
	
    pos = (MV  * vec4(position.w * aPos + position.xyz, 1.0)).xyz;

    gl_Position = P *  vec4(pos, 1.0);	
    
	normal = normalize((iMV * vec4(aNor,0.0)).xyz);
}