#version 110
#extension GL_ARB_texture_rectangle : enable

uniform sampler2DRect colorTex;
uniform sampler2DRect occlusionTex;
uniform int renderOcclusionMap;
varying float positionValid;
varying float projectiveZ;
varying float occlusion;

const float epsilon = 1e-6;

float LinearizeDepth(float zoverw){
	float n = 10.0; // camera z near
	float f = 100.0; // camera z far
	return (2.0 * n) / (f + n - zoverw * (f - n));
}

void main()
{
	if(positionValid < epsilon){
    	discard;
        return;
    }
	
	if(renderOcclusionMap == 1){
		vec4 texcol = texture2DRect(colorTex, gl_TexCoord[0].st);
		gl_FragColor.xyz = vec3( projectiveZ );// * texcol.rgb;
		gl_FragColor.a = 1.0;
	}
	else {
		gl_FragColor = texture2DRect(colorTex, gl_TexCoord[0].st) * gl_Color;
//		vec2 flipcd = gl_TexCoord[0].st;
//		flipcd.y = 1080. - flipcd.y;
//		gl_FragColor.xyz = texture2DRect(occlusionTex, flipcd).xyz;
//		gl_FragColor.a = 1.0;
	}


}
