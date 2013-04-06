
#version 110
#extension GL_ARB_texture_rectangle : enable

//This shader takes a combined video / depth / normal texture and projects it onto the given geometry
//The video files are generated using the RGBDCombinedVideoExporter

//TEXTURE INFORMATION
uniform sampler2DRect texture;
uniform int useTexture;
uniform vec2 depthSubrectOffset;
uniform vec2 depthDimensions; //size of the depth image in the video frame
uniform vec2 depthRange; //min and max depth values for this video

//GEOMETRY INFORMATION
uniform float farClip;
uniform float nearClip;
uniform float edgeClip;
uniform vec2 simplify;

//DEPTH INTRINSICS
uniform vec2 depthPP;
uniform vec2 depthFOV;

//COLOR INTRINSICS
uniform vec2 colorFOV;
uniform vec2 colorPP;
uniform vec3 dK;
uniform vec2 dP;

//CORRESPONDENCE EXTRINSICS
uniform vec2 colorDimensions;
uniform mat3 colorRotate;
uniform vec3 colorTranslate;

//ADJUSTMENTS
uniform vec2 textureScale;
uniform vec2 shift;
uniform vec2 scale;

varying float VZPositionValid0;

const float epsilon = 1e-6;

vec2 rgb2hsl( vec2 input ) {
	float h = 0.0;
	float s = 0.0;
	float l = 0.0;
	float r = input.r;
	float g = input.g;
	float b = input.b;
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
	return vec2( h, s, l );
}

void main(void)
{
    //align to texture
    vec2 halfvec = vec2(.5,.5);
    float depth = texture2DRect(depthTex, floor(gl_Vertex.xy) + halfvec);
    float right = texture2DRect(depthTex, floor(gl_Vertex.xy  + vec2(simplify.x,0.0)) + halfvec );
    float down  = texture2DRect(depthTex, floor(gl_Vertex.xy  + vec2(0.0,simplify.y)) + halfvec );
    float left  = texture2DRect(depthTex, floor(gl_Vertex.xy + vec2(-simplify.x,0.0)) + halfvec );
    float up    = texture2DRect(depthTex, floor(gl_Vertex.xy + vec2(0.0,-simplify.y)) + halfvec );
    float bl    = texture2DRect(depthTex, vec2(floor(gl_Vertex.x - simplify.x),floor( gl_Vertex.y + simplify.y)) + halfvec );
    float ur    = texture2DRect(depthTex, vec2(floor(gl_Vertex.x  + simplify.x),floor(gl_Vertex.y - simplify.y)) + halfvec );

    //cull invalid verts
    VZPositionValid0 = (depth < farClip &&
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
						) ? 1.0 : 0.0;


	vec4 pos = vec4((gl_Vertex.x - depthPP.x) * depth / depthFOV.x,
                    (gl_Vertex.y - depthPP.y) * depth / depthFOV.y, depth, 1.0);

    //projective texture on the geometry
    if(useTexture == 1){
        vec4 texCd;
		//http://opencv.willowgarage.com/documentation/camera_calibration_and_3d_reconstruction.html
		vec3 projection = colorRotate * pos.xyz + colorTranslate + vec3(shift * colorDimensions / textureScale,0);
		//vec3 projection = pos.xyz + colorTranslate + vec3(shift*dim,0);
		if(projection.z != 0.0) {

			vec2 xyp = projection.xy / projection.z;
			float r2 = pow(xyp.x, 2.0) + pow(xyp.y, 2.0);
			float r4 = r2*r2;
			float r6 = r4*r2;
			vec2 xypp = xyp;
			xypp.x = xyp.x * (1.0 + dK.x*r2 + dK.y*r4 + dK.z*r6) + 2.0*dP.x * xyp.x * xyp.y + dP.y*(r2 + 2.0 * pow(xyp.x,2.0) );
			xypp.y = xyp.y * (1.0 + dK.x*r2 + dK.y*r4 + dK.z*r6) + dP.x * (r2 + 2.0*pow(xyp.y, 2.0) ) + 2.0*dP.y*xyp.x*xyp.y;
			vec2 uv = (colorFOV * xypp + colorPP) * textureScale;
			texCd.xy = ((uv-dim/2.0) * scale) + dim/2.0;
		}

        gl_TexCoord[0] = texCd;
    }
    
    gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * pos;
    gl_FrontColor = gl_Color;
}
