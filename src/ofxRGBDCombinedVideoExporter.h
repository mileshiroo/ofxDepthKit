
#pragma once

#include "ofMain.h"
#include "ofxRGBDCPURenderer.h"

class ofxRGBDCombinedVideoExporter {
  public:
	ofxRGBDCombinedVideoExporter();
	~ofxRGBDCombinedVideoExporter();
	
	int minDepth;
	int maxDepth;
	
	void updatePixels(ofxRGBDCPURenderer& mesh, ofBaseHasPixels& colorPixels);
	
	ofPixelsRef getPixels();
	
  protected:
	
	ofPixels pixels;
	ofColor huePixelForDepth(unsigned short x);
};