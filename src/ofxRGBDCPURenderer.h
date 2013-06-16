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
	
	//mesh with only valid vertices
	void getReducedMesh(ofMesh& mesh,
						bool normalizeTextureCoords,
						bool flipTextureX = false,
						bool flipTextureY = false,
						ofMatrix4x4 mat = ofMatrix4x4());
	
//	ofMesh getReducedMesh(bool normalizeTextureCoords,
//						  ofVec3f vertexScale = ofVec3f(1,1,1),
//						  bool flipTextureX = false, bool flipTextureY = false, float texCoordScale = 1.0);
		
	bool hasTriangles;
	bool calculateTextureCoordinates;
	bool normalizeTextureCoordinates;

	ofVec3f getWorldPoint(float x, float y);
	ofVec3f getWorldPoint(float x, float y, unsigned short z);
	ofVec3f getWorldPoint(float x, float y, ofShortPixels& pixels);
	
	bool cacheValidVertices;
	vector<ofIndexType> validVertIndices;
	//returns the x,y coord in the image for the given
	pair<int,int> getPixelLocationForIndex(ofIndexType index);

	//after this call, texCoords will be the same size as points, full of texture coords;
	void generateTextureCoordinates(vector<ofVec3f>& points, vector<ofVec2f>& texCoords);
	ofVec2f getTextureCoordinateForPoint(ofVec3f point);
	
  private:
	void setupDrawMatrices();
	map< ofIndexType, pair<int, int> > indexToPixelCoord;
	
	void generateTextureCoordinates();  
	vector<ofIndexType> baseIndeces;	
};
