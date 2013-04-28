
#include "ofxRGBDCombinedVideoRenderer.h"

ofxRGBDCombinedVideoRenderer::ofxRGBDCombinedVideoRenderer(){
	combinedImage = NULL;
}

ofxRGBDCombinedVideoRenderer::~ofxRGBDCombinedVideoRenderer(){
	
}

void ofxRGBDCombinedVideoRenderer::setupDepthProperties(string depthPropertiesXml){
	ofxXmlSettings settings;
	if(settings.loadFile(depthPropertiesXml)){
		settings.pushTag("depth");
		
		settings.popTag();//depth
	}
}

void ofxRGBDCombinedVideoRenderer::setShaderPath(string shaderPath){
	this->shaderPath = shaderPath;
	reloadShader();
}

void ofxRGBDCombinedVideoRenderer::reloadShader(){
	shader.load(shaderPath);
}

void ofxRGBDCombinedVideoRenderer::setCombinedImage(ofBaseHasTexture& texture){
	combinedImage = &texture;
}

