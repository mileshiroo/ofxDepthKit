#pragma once

#include "ofMain.h"
#include "ofxXmlSettings.h"
typedef struct {
	ofVec2f fov;
	ofVec2f principalPoint;
	float minDepth;
	float maxDepth;
	
} DepthProperties;

class ofxRGBDCombinedVideoRenderer {
  public:
	ofxRGBDCombinedVideoRenderer();
	~ofxRGBDCombinedVideoRenderer();
	
	//exported by combined video renderer
	void setupDepthProperties(string depthPropertiesXml);

	void setCombinedImage(ofBaseHasTexture& texture);
	void setShaderPath(string shaderPath);
	void reloadShader();
	
  protected:
	ofShader shader;
	string shaderPath;
	ofBaseHasTexture* combinedImage;
	
};