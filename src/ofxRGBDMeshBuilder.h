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
#include "ofxDepthHoleFiller.h"

using namespace ofxCv;

class ofxRGBDMeshBuilder {
  public:
    ofxRGBDMeshBuilder();
    ~ofxRGBDMeshBuilder();    
    
    bool setup(string calibrationDirectory);
	void setDepthOnly();
	
	void setTexture(ofBaseHasTexture& texture);
	void setDepthPixels(ofShortPixels& pixels);

	void update();
    void update(ofShortPixels& depthImage);
	
    void draw();
    void draw(ofBaseHasTexture& texture);

	ofVec3f worldPosition;
	ofVec3f worldRotation;
	ofVec3f pivot;
	ofVec3f center;
	void updateCenter();
	void setPivotToMeshCenter();
	
    void setSimplification(ofVec2f simplify);
    ofVec2f getSimplification();
    
	ofxDepthHoleFiller& getHoleFiller();
	
    void setXYShift(ofVec2f shift);
    ofVec2f shift;
	ofVec2f scale;
	ofVec2f angleFov;
    
    //contains un-index vertices
    ofMesh& getMesh();
    //mesh with only valid vertices
    ofMesh getReducedMesh(bool normalizeTextureCoords, ofVec3f vertexScale = ofVec3f(1,1,1), bool flipTextureX = false, bool flipTextureY = false);
    float farClip;
	float nearClip;
	float bottomClip;
	float topClip;
	float rightClip;
	float leftClip;
	
    float edgeClip;
    
	bool mirror;
	
    bool addColors;
    bool undistortDepthImage;
    bool calculateTextureCoordinates;
    bool normalizeTextureCoordinates;
    
    void setTextureScaleForImage(ofBaseHasTexture& texture);
	Calibration depthCalibration, rgbCalibration;
    ofVec2f textureScale;
	
	ofVec3f getWorldPoint(float x, float y);
    ofVec3f getWorldPoint(float x, float y, unsigned short z);
    ofVec3f getWorldPoint(float x, float y, ofShortPixels& pixels);

	bool cacheValidVertices;
	vector<ofIndexType> validVertIndices;

	ofBaseHasTexture* currentTexture;

	//after this call, texCoords will be the same size as points, full of texture coords;
	void generateTextureCoordinates(vector<ofVec3f>& points, vector<ofVec2f>& texCoords);
	ofVec2f getTextureCoordinateForPoint(ofVec3f point);
	
	void drawRGBProjection();
	
  private:
    ofMesh mesh;
    Mat rotationDepthToRGB, translationDepthToRGB;
	ofShortPixels* currentDepthPixels;
	
    Point2d principalPoint;
    cv::Size imageSize;
	
	void setupDrawMatrices();
	
	ofVec2f fov;
    ofVec2f simplify;
    bool hasTriangles;
    bool calibrationSetup;
    bool depthOnly;
	
	ofxDepthHoleFiller holeFiller;
    void generateTextureCoordinates();  
    vector<ofIndexType> baseIndeces;    
};

