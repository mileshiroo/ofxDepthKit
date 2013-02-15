/**
 * ofxRGBDepth addon
 *
 * Core addon for programming against the RGBDToolkit API
 * http://www.rgbdtoolkit.com
 *
 * (c) James George 2011-2013 http://www.jamesgeorge.org
 *
 * Developed with support from:
 *      Frank-Ratchy STUDIO for Creative Inquiry http://studioforcreativeinquiry.org/
 *      YCAM InterLab http://interlab.ycam.jp/en
 *      Eyebeam http://eyebeam.org
 */


#pragma once

#include "ofMain.h"
#include "ofxCv.h"

class ofxRGBDRenderer {
public:
	ofxRGBDRenderer();
	virtual ~ofxRGBDRenderer();
	
	bool setup(string rgbIntrinsicsPath, string depthIntrinsicsPath,
               string rotationPath, string translationPath);
	bool setup(string calibrationDirectory);
    
//    virtual void setDepthOnly();

    void setRGBTexture(ofBaseHasTexture& tex);
    void setDepthImage(ofShortPixels& pix);
    ofBaseHasTexture& getRGBTexture();
    ofShortPixels& getDepthImage();
    
	virtual void update() = 0;
    void drawMesh();
	void drawPointCloud();
	void drawWireFrame();
    virtual void draw(ofPolyRenderMode drawMode) = 0;
    
    //fudge factors to apply during alignment
    void setXYShift(ofVec2f shift);
    void setXYScale(ofVec2f scale);
    ofVec2f shift;
    ofVec2f scale;
    
	float edgeClip;
	float farClip;
    float nearClip;
    
    bool addColors;
	bool mirror;
    bool calibrationSetup;
    bool useTexture;
	bool flipTexture;

    ofVec3f worldPosition;
	ofVec3f worldRotation;
    
    /*
    bool bindRenderer(); //built in shader
    bool bindRenderer(ofShader& customShader); //any custom shader
    void unbindRenderer();
    
    //called inside of bind/unbind
    void setupProjectionUniforms(ofShader& shader);
    void reloadShader();
    */
    
    //fun way of visualizing the calibration
    void drawProjectionDebug(bool showDepth, bool showRGB, float rgbTexturePosition);
    
	//sets a level of simplification,
	//should be either 1 for none
	//2 for half, or 4 for quarter;
	virtual void setSimplification(ofVec2f simplification) = 0;
	ofVec2f getSimplification();
    
//  void drawMesh(ofShader& customShader);
//	void drawPointCloud(ofShader& customShader);
//	void drawWireFrame(ofShader& customShader);
    
	//populated with vertices, texture coords, and indeces
	ofMesh& getMesh();
	
    ofxCv::Calibration& getRGBCalibration();
	ofxCv::Calibration& getDepthCalibration();
	ofMatrix4x4& getDepthToRGBTransform();
	ofMatrix4x4& getRGBMatrix();
	ofTexture& getDepthTexture();
    
  protected:
	ofVec2f simplify;
    
    bool meshGenerated;
    //bool shaderBound;
//    ofShader* currentlyBoundShader;
//    bool rendererBound;
    
    cv::Point2d principalPoint;
    cv::Size imageSize;
	ofxCv::Calibration depthCalibration, rgbCalibration;
    cv::Mat rotationDepthToRGB, translationDepthToRGB;
    float fx, fy;
    
	bool hasDepthImage;
	bool hasRGBImage;
    
	ofBaseHasTexture* currentRGBImage;
	ofShortPixels* currentDepthImage;
	
    //ofVboMesh mesh;
    ofMesh mesh;
	ofMatrix4x4 depthToRGBView;
	ofMatrix4x4 rgbProjection;
    ofMatrix4x4 rgbMatrix;
	ofMatrix4x4 depthProjection;
	
//	ofShader meshShader;
    
};
