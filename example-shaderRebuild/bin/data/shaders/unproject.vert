#version 110

uniform vec2 dim;
uniform vec2 shift;
uniform vec2 scale;

uniform sampler2DRect depthTex;
uniform vec2 principalPoint;
uniform vec2 fov;
uniform float farClip;
uniform float edgeClip;

varying float VZPositionValid0;
void main(void)
{

    float depth = texture2DRect(depthTex, gl_Vertex.xy).r * 65535.;
    float side = texture2DRect(depthTex, gl_Vertex.xy + vec2(1.0,0)).r * 65535.;
    float down = texture2DRect(depthTex, gl_Vertex.xy + vec2(0,1.0)).r * 65535.;
	//VZPositionValid0 = (depth < farClip && depth > 400.) ? 1.0 : 0.0;

    VZPositionValid0 = (abs(side - depth) < edgeClip &&
                        abs(down - depth) < edgeClip &&
                        depth < farClip && depth > 200. ) ? 1.0 : 0.0;
    
	vec4 pos = vec4((gl_Vertex.x - principalPoint.x) * depth / fov.x,
	(gl_Vertex.y - principalPoint.y) * depth / fov.y, depth, 1.0);
    
    //projective texture on the
    gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * pos;
    gl_FrontColor = gl_Color;
    
    mat4 tTex = gl_TextureMatrix[0];
    vec4 texCd = tTex * pos;
    texCd.xyz /= texCd.w;
    
    texCd.y *= -1.;
    texCd.xy += 1.;
    texCd.xy /= 2.;
    
	texCd.xy *= scale;
    texCd.xy += shift;
    
    texCd.xy *= dim;
    gl_TexCoord[0] = texCd;
}
