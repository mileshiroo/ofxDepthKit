/*
 *  ofxRGBDepthFrameProviderOpenNI.h
 *  ofxRGBDepthCaptureOpenNI
 *
 *  Created by James George on 3/13/12.
 *
 */

#pragma once

#include "ofMain.h"
#include "ofxOpenNI.h"
#include "ofxDepthImageProvider.h"

class ofxDepthImageProviderOpenNI : public ofxDepthImageProvider {
  public:
	ofxDepthImageProviderOpenNI();
	

	void setContext(ofxOpenNIContext* recordContext);
	void setup(int deviceId = 0, bool useColor = false);
	void update();
	ofVec3f getWorldCoordinateAt(int x, int y);
	int maxDepth();	
	void close();
	ofxOpenNIContext* getContext(){ return recordContext; }
	
  protected:
	ofxOpenNIContext*	recordContext;
	ofxDepthGenerator	recordDepth;
	ofxIRGenerator		recordImage;
	ofxImageGenerator	recordColor;
	bool usingColor;

};
