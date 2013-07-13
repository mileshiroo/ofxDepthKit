
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
	bool oneToOne;
	int frameSize;
  protected:
	
	ofPixels pixels;

	ofColor huePixelForDepth(unsigned short x);
};