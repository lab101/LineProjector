#version 150

in vec4 ciPosition;
in vec3 ciColor;

uniform mat4 ciModelViewProjection;

out highp vec3 vColor;
out highp float pointSize;

void main()
{
    
	pointSize= max(ciPosition.z,2.0);
    gl_PointSize = pointSize;
    vec4 pos = ciPosition;
    pos.z=0;
    
    gl_Position = ciModelViewProjection * pos;
    vColor = ciColor;
    

}