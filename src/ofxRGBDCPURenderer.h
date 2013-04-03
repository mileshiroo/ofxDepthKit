/**
 * ofxRGBDCPURenderer
 *
 * Utility to build stitched ofMesh out of a depth image
 * Creates texture coords using the calibration supplied.
 *
 */

#pragma once

#include "ofMain.h"
#include "ofxCv.h"
#include "ofxRGBDRenderer.h"

//#include "ofxDepthHoleFiller.h"

class ofxRGBDCPURenderer : public ofxRGBDRenderer{
  public:
    
    ofxRGBDCPURenderer();
    virtual ~ofxRGBDCPURenderer();
    
	virtual void update();    
	virtual void draw(ofPolyRenderMode);

    //derived from mesh
	ofVec3f pivot;
	ofVec3f center;
	void updateCenter();
	void setPivotToMeshCenter();
    void setSimplification(ofVec2f simplify);
    
	ofMesh getReducedMesh(bool normalizeTextureCoords = false,
                          bool flipTextureX = false,
						  bool flipTextureY = false,
						  ofMatrix4x4 vertexAdjust = ofMatrix4x4());

	void getReducedMesh(ofMesh& mesh,
						bool normalizeTextureCoords = false,
						bool flipTextureX = false,
						bool flipTextureY = false,
						ofMatrix4x4 vertexAdjust = ofMatrix4x4());
	
    //TODO: bring to super class
	float bottomClip;
	float topClip;
	float rightClip;
	float leftClip;
	
    bool hasTriangles;
    bool calculateTextureCoordinates;
    bool normalizeTextureCoordinates;    
    void setTextureScaleForImage(ofBaseHasTexture& texture);
    
    ofVec2f textureScale;
	ofVec3f getWorldPoint(float x, float y);
    ofVec3f getWorldPoint(float x, float y, unsigned short z);
    ofVec3f getWorldPoint(float x, float y, ofShortPixels& pixels);
    
	bool cacheValidVertices;
	bool isIndexValid(ofIndexType index);
	
	map<ofIndexType, ofIndexType> reducedMeshIndex;
	vector<ofIndexType> validVertIndices;
	
    //returns the x,y coord in the image for the given
    ofVec2f getPixelLocationForIndex(ofIndexType index);

	//after this call, texCoords will be the same size as points, full of texture coords;
	void generateTextureCoordinates(vector<ofVec3f>& points, vector<ofVec2f>& texCoords);
	ofVec2f getTextureCoordinateForPoint(ofVec3f point);
	int vertsPerRow;
	int vertsPerCol;
	
  private:
    
	void setupDrawMatrices();
	map< ofIndexType, ofVec2f > indexToPixelCoord;
    map< ofIndexType, bool > validIndeces;
	
    void generateTextureCoordinates();  
    vector<ofIndexType> baseIndeces;    
};
