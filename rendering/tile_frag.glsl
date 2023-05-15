#version 330 core

uniform vec3 light0;

smooth in vec3 normal;
in vec3 pos;

out vec4 outcol;

void main( void ) {
  vec3 lightin = pos - light0;
  vec3 lightout = reflect( lightin, normalize(normal) );
  float diff = -dot(lightin,normal);
  float spec = clamp(dot(light0-pos,lightout),0,1);
  if( diff < 0 ){ spec = 0; }
  diff = clamp( diff,0,1 );
  vec3 col0;
  int ii = int(floor(pos.x/0.3)) + int(floor(pos.z/0.3));
  if( (ii&1) == 1 )
  {
    col0 = vec3(1.0);
  }else
  {
    col0 = vec3(0.2);
  }
  outcol = vec4( col0*(0.4 + diff*0.8) + vec3(1)*spec*0.3, 1 );
}
