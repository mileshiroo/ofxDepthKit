/*
 *  ofxRGBDepthFrameProviderOpenNI.cpp
 *  ofxRGBDepthCaptureOpenNI
 *
 *  Created by Jim on 3/13/12.
 *  Copyright 2012 FlightPhase. All rights reserved.
 *
 */

#include "ofxDepthImageProviderOpenNI.h"

void ofxDepthImageProviderOpenNI::setup(){
	bDeviceFound  = recordContext.setup();	// all nodes created by code -> NOT using the xml config file at all
	bDeviceFound &= recordDepth.setup(&recordContext);
	bDeviceFound &= recordImage.setup(&recordContext);
    
    if(!bDeviceFound){
    	ofLogError("ofxDepthImageProviderOpenNI -- OpenNI Device Failed");
    }
}

void ofxDepthImageProviderOpenNI::update(){
	recordContext.update();
	recordImage.update();
	rawIRImage.setFromPixels(recordImage.getIRPixels(), 640,480, OF_IMAGE_GRAYSCALE);
	recordDepth.update();

	if(recordDepth.isFrameNew()){
		bNewFrame = true;
//		bDepthImageDirty = true;
		depthPixels.setFromPixels( (unsigned short*) recordDepth.getRawDepthPixels(), 640, 480, OF_IMAGE_GRAYSCALE);
	}
}


ofVec3f ofxDepthImageProviderOpenNI::getWorldCoordinateAt(int x, int y){
	
	vector<ofVec2f> c;
	vector<ofVec3f> w;
	
	c.push_back(ofVec2f(x,y));

	const int nPoints = c.size();
	w.resize(nPoints);
	
	
//	if (!g_bIsDepthRawOnOption) {
//		ofLogError(LOG_NAME) << "ofxOpenNI::cameraToWorld - cannot perform this function if g_bIsDepthRawOnOption is false. You can enabled g_bIsDepthRawOnOption by calling getDepthRawPixels(..).";
//		return;
//	}
	
	vector<XnPoint3D> projective(nPoints);
	XnPoint3D *out = &projective[0];
	
//	if(threaded) lock();
	const XnDepthPixel* d = depthPixels.getPixels();
	unsigned int pixel;
	for (int i=0; i<nPoints; ++i) {
		pixel  = (int)c[i].x + (int)c[i].y * 640;
		if (pixel >= 640*480)
			continue;
		
		projective[i].X = c[i].x;
		projective[i].Y = c[i].y;
		projective[i].Z = float(d[pixel]) / 1000.0f;
	}
//	if(threaded) unlock();

	recordDepth.getXnDepthGenerator().ConvertProjectiveToRealWorld(nPoints, &projective[0], (XnPoint3D*)&w[0]);
//	cout << "World coord for " << x << " " << y << " is " << w[0] << endl;
	return w[0];
}

int ofxDepthImageProviderOpenNI::maxDepth(){
	return recordDepth.getMaxDepth();
}

void ofxDepthImageProviderOpenNI::close(){
	recordContext.shutdown();
}