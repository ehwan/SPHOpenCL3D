#version 330 core

uniform vec3 light0;
smooth in vec3 normal;
in vec3 pos;

out vec4 outcol;

void main( void ) {
  //outcol = vec4(1);
  //return;
  vec3 normal_ = normalize(normal);
  vec3 lightin = pos - light0;
  vec3 lightout = reflect( lightin, normal_ );
  float diff = -dot(lightin,normal_);
  float spec = clamp(dot(light0-pos,lightout),0,1);
  if( diff < 0 ){ spec = 0; }
  diff = clamp( diff,0,1 );
  vec3 col0;
  col0 = vec3( 0.6, 0.8, 1.0 );
  outcol = vec4( col0*(0.4 + diff*0.8) + vec3(1)*spec*0.3, 1 );
  //outcol.w = 0.6;

  /*
  vec3 floor_color;

  vec3 normal_dir = dot(lightin,normal_)*normal_;
  vec3 tangent_dir = lightin - normal_dir;
  float alpha = 0.5;
  vec3 refrag = normal_dir + tangent_dir*alpha;
  if( abs(refrag.y) < 1e-6 ){ floor_color = vec3(0.0); }
  else{
    float t = -pos.y/refrag.y;
    vec3 p = pos + t*refrag;

    int ii = int(floor(p.x/0.3)) + int(floor(p.z/0.3));
    if( (ii&1) == 1 )
    {
      floor_color = vec3(1.0);
    }else
    {
      floor_color = vec3(0.2);
    }
  }
  outcol = vec4(floor_color*(1.0-outcol.w) + outcol.w*outcol.xyz,1.0);
  */
}
