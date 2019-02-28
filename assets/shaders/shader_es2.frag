#version 150


//uniform float uRadius;

in vec3 vColor;
in float pointSize;

out vec4 outColor;

void main()
{
    
    vec3 d = vec3(gl_PointCoord.xy,0) - vec3(0.5,0.5,0);
    
    float l = length(d);
	
  //  float c =  1 - smoothstep( pointSize *0.0001, pointSize * .002 , l);
    float c =  1 - smoothstep(0.1, .5 , l);

    outColor = vec4(vec3(1.0,1.0,0.0),c);
    
}
