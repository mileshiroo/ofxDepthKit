#include "ofxDepthImageProviderRSSDK.h"

void ofxDepthImageProviderRSSDK::setup(int deviceId, bool useColor) {
	if(!mRSSDK.init(useColor, true)) {
		ofLogError("Unable to create ofxRSSDK object");
	}
}

void ofxDepthImageProviderRSSDK::update() {
	mRSSDK.update();
}

ofVec3f ofxDepthImageProviderRSSDK::getWorldCoordinateAt(int x, int y) {
	return mRSSDK.getWorldCoordinateAt(x,y);
}

int ofxDepthImageProviderRSSDK::maxDepth() {
	//not sure.. 
	return 10000;
}

void ofxDepthImageProviderRSSDK::close() {
	return mRSSDK.close();
}

ofShortPixels& ofxDepthImageProviderRSSDK::getRawDepth() {
	return mRSSDK.getRawDepthPixelsRef();
}

ofImage& ofxDepthImageProviderRSSDK::getColorImage() {
	ofImage im;
	im.setFromPixels(mRSSDK.getPixelsRef());
	return im;
}