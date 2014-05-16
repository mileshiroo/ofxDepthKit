/*
 *  ofxRGBDepthFrameProviderFreenect.cpp
 *  ofxRGBDepthCaptureOpenNI
 *
 *  Created by James on 5/15/14.
 *  Copyright 2014 FlightPhase. All rights reserved.
 *
 */

#include "ofxDepthImageProviderKCB.h"


void ofxDepthImageProviderKCB::setup(int deviceId, bool useColor){

	kinect.initSensor();
	kinect.initIRStream(640, 480);
	kinect.initDepthStream(640, 480, false);

	//simple start
	bDeviceFound = kinect.start();

}

void ofxDepthImageProviderKCB::update(){
	kinect.update();
	// there is a new frame and we are connected
	if(kinect.isFrameNew()) {
		bNewFrame = true;
		bDepthImageDirty = true;
		depthPixels = kinect.getRawDepthPixelsRef();
		rawIRImage.setUseTexture(false);
		rawIRImage.setFromPixels(kinect.getColorPixelsRef());
		rawIRImage.setImageType(OF_IMAGE_GRAYSCALE);
		rawIRImage.setUseTexture(true);
		rawIRImage.update();
	}		
}

ofVec3f ofxDepthImageProviderKCB::getWorldCoordinateAt(int x, int y){
	return kinect.mapDepthToSkeleton(ofPoint(x, y) );
}

int ofxDepthImageProviderKCB::maxDepth(){
	return 10000; //taken from looking into how ofxKinect calculates it's look up tables.
}

void ofxDepthImageProviderKCB::close(){
	kinect.stop();
}