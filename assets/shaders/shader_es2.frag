#version 150

in highp vec3 vColor;
in highp float pointSize;

out vec4 outColor;

void main()
{
    
    highp vec3 d = vec3(gl_PointCoord.xy,0) - vec3(0.5,0.5,0);
    
    highp float calcLength = length(d);
	
  //  float c =  1 - smoothstep( pointSize *0.0001, pointSize * .002 , l);
    highp  float c =  1.0 - smoothstep(0.1, 0.5 , calcLength );

    outColor = vec4(vec3(vColor*c) , c);
    
}
