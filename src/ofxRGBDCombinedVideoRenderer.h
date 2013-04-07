#pragma once

#include "ofMain.h"
#include "ofxXmlSettings.h"

class ofxRGBDCombinedVideoRenderer {
public:
    
    ofxRGBDCombinedVideoRenderer();
    ~ofxRGBDCombinedVideoRenderer();
	
    //  SET
    //
	bool setup(string videoPath);
    void setTexture(ofBaseHasTexture& _tex);
	void setShaderPath(string _shaderPath);

    //  Use these to project and draw textured custom geometry
    //
	bool bindRenderer();
	void unbindRenderer();
    
    //  UPDATE
    //
    void update();
    
    //  DRAW
    //
    void drawMesh();
	void drawPointCloud();
	void drawWireFrame();
	void draw(ofPolyRenderMode drawMode);
    
    ofVec3f worldPosition;
	ofVec3f worldRotation;
    
    ofVec2f simplify;
    ofVec2f shift;
	ofVec2f scale;
    
    float edgeClip;
	float farClip;
	float nearClip;
    
    bool bFlipTexture;
    bool bMirror;
    
protected:
    void reloadShader();
    void setupProjectionUniforms();
    void setSimplification(ofVec2f _simplification);
    void setTextureScaleForImage(ofBaseHasTexture& _texture);
    
    ofBaseHasTexture *tex;
	ofShader shader;
    string shaderPath;
    
    ofMesh mesh;
    
    //broken out intrinsics/extrinics for easy access
    //
	ofVec2f     depthPrincipalPoint;
	ofVec2f     depthFOV;
	ofRectangle depthImageSize;
	
	ofVec2f     colorPrincipalPoint;
	ofVec2f     colorFOV;
	ofRectangle colorImageSize;
    
    ofVec2f     textureScale;
	
	//broken out extrinsics
	//
    float       depthToRGBRotation[9];
	ofVec3f     depthToRGBTranslation;
	ofVec3f     distortionK;
	ofVec2f     distortionP;
    
    bool bRendererBound;
    bool bMeshGenerated;
    
};