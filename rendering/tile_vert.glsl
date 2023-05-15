#version 330 core

layout( location=0 ) in vec3 invec;
layout( location=1 ) in vec3 innormal;

uniform mat4 u_eye;
uniform mat4 u_projection;

smooth out vec3 normal;
out vec3 pos;

void main(){
  normal = innormal;
  pos = invec;
  gl_Position = u_projection*u_eye*vec4(invec,1.0);
}
