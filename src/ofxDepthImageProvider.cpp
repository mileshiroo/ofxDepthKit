/*
 *  ofxRGBDepthFrameProvider.cpp
 *  ofxRGBDepthCaptureOpenNI
 *
 *  Created by Jim on 3/13/12.
 *  Copyright 2012 FlightPhase. All rights reserved.
 *
 */

#include "ofxDepthImageProvider.h"

ofxDepthImageProvider::ofxDepthImageProvider(){
	bUseRainbow = true;
	
	bDepthImageDirty = false;
	bDeviceFound = false;
	bNewFrame = false;
	
	depthPixels.allocate(640, 480, OF_IMAGE_GRAYSCALE);
	rawIRImage.allocate(640, 480, OF_IMAGE_GRAYSCALE);
	depthImage.allocate(640, 480, OF_IMAGE_COLOR);
}

//void ofxDepthImageProvider::setDepthModeRainbow(bool useRainbow){
//	if(useRainbow != bUseRainbow){
//		bUseRainbow = useRainbow;
//		bDepthImageDirty = true;
//	}
//}

bool ofxDepthImageProvider::isFrameNew(){
	bool ret = bNewFrame;
	bNewFrame = false;
	return ret;	
}

bool ofxDepthImageProvider::deviceFound(){
	return bDeviceFound;
}

ofShortPixels& ofxDepthImageProvider::getRawDepth(){
	return depthPixels;
}

ofImage& ofxDepthImageProvider::getRawIRImage(){
	return rawIRImage;	
}

ofImage& ofxDepthImageProvider::getColorImage(){
	return colorImage;
}


