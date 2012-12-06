#version 110
#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect colorTex;
varying float VZPositionValid0;

void main()
{
	if(VZPositionValid0 < 0.999){ 
    	discard;
    }
     
	vec4 col = texture2DRect(colorTex, gl_TexCoord[0].st);
	gl_FragColor = col * gl_Color;
    //gl_FragColor = vec4(VZPositionValid0, 0.0, 0.0, 1.0);
    //gl_FragColor = vec4(depthSample, depthSample, depthSample,1.0);
}