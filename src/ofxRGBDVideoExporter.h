#pragma once

#include "ofMain.h"
#include "ofxRGBDRenderer.h"
#include "ofxRGBDPlayer.h"
#include "ofxXmlSettings.h"

class ofxRGBDVideoExporter {
  public:
	
	ofxRGBDVideoExporter();
	~ofxRGBDVideoExporter();

	void setRenderer(ofxRGBDRenderer* renderer);
	void setPlayer(ofxRGBDPlayer* player);
	
	void render(string outputPath, string clipName);
	
	ofIntRange inoutPoint;
	float minDepth;
	float maxDepth;

  protected:

	ofColor getColorForZDepth(unsigned short z);
	void writeMetaFile(string outputDirectory);
	
	ofxRGBDRenderer* renderer;
	ofxRGBDPlayer* player;
	
	ofRectangle videoRectangle;
	ofPixels outputImage;
};