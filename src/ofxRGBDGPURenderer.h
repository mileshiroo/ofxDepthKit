/**
 * ofxRGBDepth addon
 *
 * Core addon for programming against the RGBDToolkit API
 * http://www.rgbdtoolkit.com
 *
 * (c) James George 2011-2013
 *  http://www.jamesgeorge.org
 *  http://github.com/obviousjim
 *
 * Developed with support from:
 *	  Frank-Ratchye STUDIO for Creative Inquiry http://studioforcreativeinquiry.org
 *	  YCAM InterLab http://interlab.ycam.jp/en
 *	  Eyebeam http://eyebeam.org
 */

#pragma once
#include "ofxRGBDRenderer.h"



class ofxRGBDGPURenderer : public ofxRGBDRenderer {
  public:
	ofxRGBDGPURenderer();
	virtual ~ofxRGBDGPURenderer();
	
	virtual void setDepthImage(ofShortPixels& pix);
	virtual void update();

	//use these to project and draw textured custom geometry
	bool bindRenderer();
	void unbindRenderer();
	
	//called inside of bind/unbind
	void setupProjectionUniforms();
	
	void setupDefaultShader();
	void setShaderPath(string path);
	void reloadShader();

	ofShader& getShader();
	ofTexture& getDepthTexture();
	
	//sets a level of simplification,
	//should be either 1 for none
	//2 for half, or 4 for quarter;
	virtual void setSimplification(ofVec2f simplification);
	
	void draw(ofPolyRenderMode drawMode);
	
  protected:
	bool bShaderLoaded;
	ofShader shader;

	void printUniforms();
	
	string shaderPath; // if "" then load default shader
	bool rendererBound;
	ofTexture depthTexture;
	
};
