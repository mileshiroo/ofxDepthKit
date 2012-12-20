/*
 *  ofxRGBDepthFrameProviderFreenect.cpp
 *  ofxRGBDepthCaptureOpenNI
 *
 *  Created by Jim on 3/13/12.
 *  Copyright 2012 FlightPhase. All rights reserved.
 *
 */

#include "ofxDepthImageProviderFreenect.h"


void ofxDepthImageProviderFreenect::setup(){
	bDeviceFound  = kinect.init(true, true); // shows infrared instead of RGB video image
	bDeviceFound &= kinect.open();
}

void ofxDepthImageProviderFreenect::update(){
	kinect.update();
	// there is a new frame and we are connected
	if(kinect.isFrameNew()) {
		bNewFrame = true;
		bDepthImageDirty = true;
		depthPixels.setFromPixels(kinect.getRawDepthPixels(), kinect.getWidth(), kinect.getHeight(), OF_IMAGE_GRAYSCALE);
		rawIRImage.setUseTexture(false);
		rawIRImage.setFromPixels(kinect.getPixelsRef());
		rawIRImage.setImageType(OF_IMAGE_GRAYSCALE);
		rawIRImage.setUseTexture(true);
		rawIRImage.update();
	}		
}

ofVec3f ofxDepthImageProviderFreenect::getWorldCoordinateAt(int x, int y){
	return kinect.getWorldCoordinateAt(x, y);
}

int ofxDepthImageProviderFreenect::maxDepth(){
	return 10000; //taken from looking into how ofxKinect calculates it's look up tables.
}

void ofxDepthImageProviderFreenect::close(){
	kinect.close();
}