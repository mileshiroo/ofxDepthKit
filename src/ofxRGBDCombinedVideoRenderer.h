#pragma once

#include "ofMain.h"
#include "ofxXmlSettings.h"

class ofxRGBDCombinedVideoRenderer {
  public:
	ofxRGBDCombinedVideoRenderer();
	~ofxRGBDCombinedVideoRenderer();
	
	void setup(string videoPath);

	void setShaderPath(string shaderPath);
	void reloadShader();
	
  protected:
	ofVideoPlayer player;
	ofShader shader;
};