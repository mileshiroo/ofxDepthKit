
#version 110
#extension GL_ARB_texture_rectangle : enable

//This shader takes a combined video / depth / normal texture and projects it onto the given geometry
//The video files are generated using the RGBDCombinedVideoExporter

//TEXTURE INFORMATION
//
uniform sampler2DRect texture;
uniform vec2 textureSize;

//COLOR
uniform vec4 colorRect;
uniform vec2 colorScale;

uniform vec2 colorFOV;
uniform vec2 colorPP;
uniform vec3 dK;
uniform vec2 dP;

uniform mat3 colorRotate;
uniform vec3 colorTranslate;

//DEPTH 
uniform vec2 depthRect;

uniform vec2 depthPP;
uniform vec2 depthFOV;

//NORMAL
uniform vec2 normalRect;

//GEOMETRY
uniform vec2  simplify;

uniform float farClip;
uniform float nearClip;
uniform float edgeClip;

uniform vec2 shift;
uniform vec2 scale;

varying float VZPositionValid0;

const float epsilon = 1e-6;

vec3 rgb2hsl( vec3 _input ){
	float h = 0.0;
	float s = 0.0;
	float l = 0.0;
	float r = _input.r;
	float g = _input.g;
	float b = _input.b;
	float cMin = min( r, min( g, b ) );
	float cMax = max( r, max( g, b ) );
	
	l = ( cMax + cMin ) / 2.0;
	if ( cMax > cMin ) {
		float cDelta = cMax - cMin;
        
		// saturation
		if ( l < 0.5 ) {
			s = cDelta / ( cMax + cMin );
		} else {
			s = cDelta / ( 2.0 - ( cMax + cMin ) );
		}
		
		// hue
		if ( r == cMax ) {
			h = ( g - b ) / cDelta;
		} else if ( g == cMax ) {
			h = 2.0 + ( b - r ) / cDelta;
		} else {
			h = 4.0 + ( r - g ) / cDelta;
		}
		
		if ( h < 0.0) {
			h += 6.0;
		}
		h = h / 6.0;
	}
	return vec3( h, s, l );
}

vec3 xyz( float _x, float _y, float _depth ) {
    float z = _depth * ( farClip - nearClip ) + nearClip;
    return vec3((_x - depthPP.x) * z / depthFOV.x, (_y - depthPP.y) * z / depthFOV.y, z);
}

void main(void){
    
    //align to texture
    vec2  halfvec = vec2(.5,.5);
    
    vec2  depthPos = gl_Vertex.xy + depthRect.xy;
    float depth = rgb2hsl( texture2DRect(texture, floor(depthPos) ).xyz ).r;
    vec4  pos = vec4( xyz(depthPos.x, depthPos.y, depth) ,1.0);

    /*
    float depth = texture2DRect(texture, floor(depthST) + halfvec).r * 65535.;
    float right = texture2DRect(texture, floor(depthST + vec2(simplify.x,0.0) ) + halfvec ).r * 65535.;
    float down  = texture2DRect(texture, floor(depthST + vec2(0.0,simplify.y) ) + halfvec ).r * 65535.;
    float left  = texture2DRect(texture, floor(depthST + vec2(-simplify.x,0.0) ) + halfvec ).r * 65535.;
    float up    = texture2DRect(texture, floor(depthST + vec2(0.0,-simplify.y) ) + halfvec ).r * 65535.;
    float bl    = texture2DRect(texture, vec2(floor(depthST.x - simplify.x),floor( depthST.y + simplify.y)) + halfvec ).r * 65535.;
    float ur    = texture2DRect(texture, vec2(floor(depthST.x  + simplify.x),floor(depthST.y - simplify.y)) + halfvec ).r * 65535.;
     */
    //cull invalid verts
    
    VZPositionValid0 = 1.0;
    /*
                        (depth < farClip &&
                        right < farClip &&
                        down < farClip &&
                        left < farClip &&
                        up < farClip &&
                        bl < farClip &&
                        ur < farClip &&
                        
                        depth > nearClip &&
                        right > nearClip &&
                        down > nearClip &&
                        left > nearClip &&
                        up > nearClip &&
                        bl > nearClip &&
                        ur > nearClip &&
                        
                        abs(down - depth) < edgeClip &&
                        abs(right - depth) < edgeClip &&
                        abs(up - depth) < edgeClip &&
                        abs(left - depth) < edgeClip &&
                        abs(ur - depth) < edgeClip &&
                        abs(bl - depth) < edgeClip
						) ? 1.0 : 0.0;*/

    
    vec4 texCd;
    // http://opencv.willowgarage.com/documentation/camera_calibration_and_3d_reconstruction.html
    //
    vec3 projection = colorRotate * pos.xyz + colorTranslate + vec3(shift * colorRect.zw / colorScale,0);

    if(projection.z != 0.0) {
        
        vec2 xyp = projection.xy / projection.z;
        float r2 = pow(xyp.x, 2.0) + pow(xyp.y, 2.0);
        float r4 = r2*r2;
        float r6 = r4*r2;
        vec2 xypp = xyp;
        xypp.x = xyp.x * (1.0 + dK.x*r2 + dK.y*r4 + dK.z*r6) + 2.0*dP.x * xyp.x * xyp.y + dP.y*(r2 + 2.0 * pow(xyp.x,2.0) );
        xypp.y = xyp.y * (1.0 + dK.x*r2 + dK.y*r4 + dK.z*r6) + dP.x * (r2 + 2.0*pow(xyp.y, 2.0) ) + 2.0*dP.y*xyp.x*xyp.y;
        vec2 uv = (colorFOV * xypp + colorPP) * colorScale;
        
        texCd.xy = ((uv-textureSize/2.0) * scale) + textureSize/2.0;
        
    }
    
    gl_TexCoord[0] = texCd;

    
    gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * pos;
    gl_FrontColor = gl_Color;
}
