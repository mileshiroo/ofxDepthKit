#pragma once

#include "ofMain.h"
#include "ofxDepthImageProvider.h"
#include "ofxRSSDK.h"

class ofxDepthImageProviderRSSDK : public ofxDepthImageProvider {
	public:
		void setup(int deviceId = 0, bool useColor = false);
		void update();
		ofVec3f getWorldCoordinateAt(int x, int y);
		int maxDepth();
		void close();

		ofShortPixels& getRawDepth(); 
		ofImage& getColorImage();

	protected:
		ofxRSSDK mRSSDK;
};