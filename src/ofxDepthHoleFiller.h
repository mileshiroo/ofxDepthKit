/*
 *  DepthHoleFiller.h
 *  ofxKinect
 *
 *  Created by James George 4/6/12
 *  Inspired by Golan's depth hole filler example
 *
 */

#pragma once

#include "ofMain.h"
#include "ofxCv.h"


class ofxDepthHoleFiller {
	
  public:
	
	ofxDepthHoleFiller();
	
	void close(ofShortPixels& depthPixels);

	int setKernelSize(int kernelSize);
	int setIterations(int iterations);
	int getKernelSize();
	int getIterations();
	void setThreshold(float treshold);
    
	bool enable;
  protected:
    float threshold;
	int kernelSize;
	int iterations;
	
};
