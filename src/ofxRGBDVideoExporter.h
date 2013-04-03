#pragma once

#include "ofMain.h"
#include "ofxRGBDRenderer.h"
#include "ofxRGBDCPURenderer.h"
#include "ofxRGBDPlayer.h"
#include "ofxXmlSettings.h"

#include "ofxPCL.h"

class ofxRGBDVideoExporter {
  public:
	
	ofxRGBDVideoExporter();
	~ofxRGBDVideoExporter();

	void setRenderer(ofxRGBDCPURenderer* renderer);
	void setPlayer(ofxRGBDPlayer* player);
	
	void render(string outputPath, string clipName);
	
	ofIntRange inoutPoint;
	float minDepth;
	float maxDepth;

  protected:

	ofColor getColorForZDepth(unsigned short z);
	void writeMetaFile(string outputDirectory);
	
	ofxRGBDCPURenderer* renderer;  // It has a mesh, call .getReducedMesh();
	ofxRGBDPlayer*      player;
	
	ofRectangle videoRectangle;
	ofPixels    outputImage;
};