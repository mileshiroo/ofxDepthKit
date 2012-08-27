/**
 * ofxRGBDMeshBuilder
 *
 * Utility to build stitched ofMesh out of a depth image
 * Creates texture coords using the calibration image supplied.
 *
 */

#pragma once

#include "ofMain.h"
#include "ofxCv.h"

using namespace ofxCv;

class ofxRGBDMeshBuilder {
  public:
    ofxRGBDMeshBuilder();
    ~ofxRGBDMeshBuilder();    
    
    bool setup(string calibrationDirectory);                       
    void updateMesh(ofShortPixels& depthImage);
    void draw(ofBaseHasTexture& texture);
    void setSimplification(int simplifyLevel);
    int getSimplification();
    
    void setXYShift(ofVec2f shift);
    ofVec2f shift;
    ofMesh& getMesh();
    
    float farClip;
	float nearClip;
    float edgeSnip;
    
    bool addColors;
    bool undistortDepthImage;
    bool calculateTextureCoordinates;
    bool normalizeTextureCoordinates;
    
    void setTextureScaleForImage(ofBaseHasTexture& texture);

	
    ofVec2f textureScale;
    ofVec3f getWorldPoint(float x, float y, unsigned short z);
    ofVec3f getWorldPoint(float x, float y, ofShortPixels& pixels);

  private:
    ofMesh mesh;
    Calibration depthCalibration, rgbCalibration;	
    Mat rotationDepthToRGB, translationDepthToRGB;
    
    Point2d principalPoint;
    cv::Size imageSize;
    float fx,fy;
    int simplify;
    bool hasTriangles;
    bool calibrationSetup;
    
	
    void generateTextureCoordinates();  
    vector<ofIndexType> baseIndeces;
    
};

