#pragma once
#include "ofxCv.h"
#include "ofxDepthImageProvider.h"

class ofxRGBDepthCalibration {
  public:
	
	void refineDepthCalibration(ofxCv::Calibration& initialDepthCalibration,
								ofxCv::Calibration& refinedCalibration,
								ofxDepthImageProvider* depthImageProvider);
};