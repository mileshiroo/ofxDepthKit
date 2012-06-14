/**
 * ofxRGBDMeshBuilder
 *
 * Utility to build stitched ofMesh out of a depth image
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
    
    void setup(string calibrationDirectory);                       
    void updateMesh(ofShortPixels& depthImage);
    void draw(ofBaseHasTexture& texture);
    void setSimplification(int simplifyLevel);

    void setXYShift(ofVec2f shift);
    ofVec2f shift;
    ofMesh& getMesh();
    
    float farClip;
    float edgeSnip;
    
    bool addColors;
    bool undistortDepthImage;
    bool calculateTextureCoordinates;
    bool normalizeTextureCoordinates;
    
    ofVec3f getWorldPoint(float x, float y, unsigned short z);
        
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

