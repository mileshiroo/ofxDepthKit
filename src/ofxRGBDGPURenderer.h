/**
 * ofxRGBDepth addon
 *
 * Core addon for programming against the RGBDToolkit API
 * http://www.rgbdtoolkit.com
 *
 * (c) James George 2011-2013
 *  http://www.jamesgeorge.org
 *  http://github.com/obviousjim
 *
 * Developed with support from:
 *      Frank-Ratchy STUDIO for Creative Inquiry http://studioforcreativeinquiry.org
 *      YCAM InterLab http://interlab.ycam.jp/en
 *      Eyebeam http://eyebeam.org
 */

#pragma once
#include "ofxRGBDRenderer.h"

class ofxRGBDGPURenderer : public ofxRGBDRenderer {
  public:
	ofxRGBDGPURenderer();
	virtual ~ofxRGBDGPURenderer();
	
//    bool setup(string rgbIntrinsicsPath, string depthIntrinsicsPath, string rotationPath, string translationPath);
//	bool setup(string calibrationDirectory);

//    void setRGBTexture(ofBaseHasTexture& tex);
    virtual void setDepthImage(ofShortPixels& pix);

//    ofBaseHasTexture& getRGBTexture();
    virtual void update();

//    //fudge factors to apply during alignment
//    void setXYShift(ofVec2f shift);
//    void setXYScale(ofVec2f scale);
//    
//    float xshift;
//	float yshift;
//	float xscale;
//    float yscale;
//	float edgeClip;
//	float farClip;
//    bool addColors;
//	bool mirror;
//    bool calibrationSetup;
    
    //ofVec3f meshRotate;

    bool bindRenderer();
    void unbindRenderer();
    
    //called inside of bind/unbind
    void setupProjectionUniforms();

    //fun way of visualizing the calibration
    void drawProjectionDebug(bool showDepth, bool showRGB, float rgbTexturePosition);
    
    void setShaderPath(string path);
	void reloadShader();

//    ofShader& getShader();
//    void setShader(ofShader& shader);

	//sets a level of simplification,
	//should be either 1 for none
	//2 for half, or 4 for quarter;
	virtual void setSimplification(ofVec2f simplification);
	
    void draw(ofPolyRenderMode drawMode);

//  void drawMesh(ofShader& customShader);
//	void drawPointCloud(ofShader& customShader);
//	void drawWireFrame(ofShader& customShader);
    
    
	//populated with vertices, texture coords, and indeces
//	ofVboMesh& getMesh();
	
//	Calibration& getRGBCalibration();
//	Calibration& getDepthCalibration();
//	ofMatrix4x4& getDepthToRGBTransform();
//	ofMatrix4x4& getRGBMatrix();
	ofTexture& getDepthTexture();
    
//	bool useTexture;
	bool flipTexture;
		
  protected:	
	ofVec2f simplify;

    //bool shaderBound;
    ofShader meshShader;
    string shaderPath;
    bool rendererBound;
	ofTexture depthTexture;


//    Point2d principalPoint;
//    cv::Size imageSize;
//	Calibration depthCalibration, rgbCalibration;    
//	Mat rotationDepthToRGB, translationDepthToRGB;
//    float fx, fy;

//	bool hasDepthImage;
//	bool hasRGBImage;

    
//	ofBaseHasTexture* currentRGBImage;
//	ofShortPixels* currentDepthImage;
//    ofImage undistortedRGBImage;
//	ofShortPixels undistortedDepthImage;

//    ofVboMesh mesh; 
    
//	ofMatrix4x4 depthToRGBView;
//	ofMatrix4x4 rgbProjection;
//    ofMatrix4x4 rgbMatrix;
//	ofMatrix4x4 depthProjection;
    
};
