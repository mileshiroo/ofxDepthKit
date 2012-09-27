/*
 *  ofxTLVideoDepthAlignmentScrubber.h
 *  RGBDPostAlign
 *
 *  Created by James George on 11/16/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "ofMain.h"
#include "ofxTLTrack.h"
#include "ofxTLDepthImageSequence.h"
#include "ofxTLVideoTrack.h"
#include "ofxRGBDVideoDepthSequence.h"

class ofxTLVideoDepthAlignmentScrubber : public ofxTLTrack {
  public:
	
	ofxTLVideoDepthAlignmentScrubber();
	~ofxTLVideoDepthAlignmentScrubber();
	
	void draw();
	
	virtual bool mousePressed(ofMouseEventArgs& args, long millis);
	virtual void mouseMoved(ofMouseEventArgs& args, long millis);
	virtual void mouseDragged(ofMouseEventArgs& args, long millis);
	virtual void mouseReleased(ofMouseEventArgs& args, long millis);
	
	virtual void keyPressed(ofKeyEventArgs& args);
	
	void load();
	void save();
	
	bool ready();
	
	//void addAlignedPair(int videoFrame, int depthFrame);
	void registerCurrentAlignment();
	void removeAlignmentPair(int index);
	
	vector<VideoDepthPair> & getPairs();
	
	ofPtr<ofxRGBDVideoDepthSequence> getPairSequence();
	void setPairSequence(ofPtr<ofxRGBDVideoDepthSequence> newSequence);
	
	ofxTLVideoTrack* videoSequence;
	ofxTLDepthImageSequence* depthSequence;
	
	
  protected:
	ofPtr<ofxRGBDVideoDepthSequence> pairSequence;
	void updateSelection();

	int selectedPairIndex;
	int selectedVideoMillis;
	int selectedDepthMillis;
	int selectedPercent;
	
//	int getDepthFrameForVideoFrame(int videoFrame);
	
//	vector<VideoDepthPair> alignedFrames;
};