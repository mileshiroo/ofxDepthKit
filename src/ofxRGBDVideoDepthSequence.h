/*
 *  ofxRGBDVideoDepthSequence.h
 *  RGBDVisualize
 *
 *  Created by James George on 11/18/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

//TODO somehow account for timestamping

#pragma once

#include "ofMain.h"
#include "ofRange.h"
#include "ofxDepthImageSequence.h"

typedef struct{
	bool isTimeBased;
	int videoFrame;
	int depthFrame;
} VideoDepthPair;

class ofxRGBDVideoDepthSequence {
  public:
	ofxRGBDVideoDepthSequence();
	~ofxRGBDVideoDepthSequence();
	
	void savePairingFile(string pairFileXml);
	bool loadPairingFile(string pairFileXml);
	
	bool ready();
	void reset();
    
    void addAlignedFrames(int videoFrame, int depthFrame);
	void addAlignedTime(int videoMillis, int depthMillis);
	void addAlignedPair(VideoDepthPair pair);
	
	void removeAlignedPair(int index);

    //This will return a time in milliseconds if the frames are time based
	long getDepthFrameForVideoFrame(long videoFrame);
    long getVideoFrameForDepthFrame(long depthFrame);
	bool isSequenceTimebased();
	
    //returns the start and stop points for the video player IN SECONDS for the given sequence and this pairing
    ofRange getStartAndEndTimes(ofVideoPlayer& player, ofxDepthImageSequence& sequence);
	vector<VideoDepthPair> & getPairs();

  protected:
	vector<VideoDepthPair> alignedFrames;
};