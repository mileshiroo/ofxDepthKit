/*
 *  ofxTLDepthImageSequence.h
 *  timelineExampleVideoPlayer
 *
 *  Created by James George on 11/14/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#pragma once

#include "ofMain.h"
#include "ofxTLImageTrack.h"
#include "ofxTLVideoThumb.h"
//#include "ofxDepthImageCompressor.h"
#include "ofxDepthImageSequence.h"

class ofxTLDepthImageSequence : public ofxTLImageTrack, public ofThread {
  public:	
	ofxTLDepthImageSequence();
	virtual ~ofxTLDepthImageSequence();

	void setup();
	void draw();

	void enable();
	void disable();

	virtual void mouseDragged(ofMouseEventArgs& args, long millis);
	virtual void mouseReleased(ofMouseEventArgs& args, long millis);

	virtual void keyPressed(ofKeyEventArgs& args);

	bool loadSequence();
	bool loadSequence(string sequenceDirectory);
    
    void setSequence(ofPtr<ofxDepthImageSequence> newSequence);
    void setSequence(ofxDepthImageSequence& newSequence);
    ofPtr<ofxDepthImageSequence> getDepthImageSequence();
	ofImage currentDepthImage;
    
	bool isLoaded();
	bool isFrameNew();
    
	void playbackStarted(ofxTLPlaybackEventArgs& args);
	void playbackEnded(ofxTLPlaybackEventArgs& args);
	void playbackLooped(ofxTLPlaybackEventArgs& args);
	
	
	
	int getSelectedFrame();
	
	int frameForTime(long timeInMillis);
	void selectFrame(int frame);
	void selectTimeInSeconds(float timeInSeconds);
	void selectTimeInMillis(long timeStampInMillis);
	
	//only works if doFramesHaveMillis is true
	long getSelectedTimeInMillis();
	
	void toggleThumbs();
	
	bool doFramesHaveTimestamps();


  protected:
    ofPtr<ofxDepthImageSequence> depthImageSequence;
    
    //width and height of image elements
    float getContentWidth();
    float getContentHeight();
	void framePositionsUpdated(vector<ofxTLVideoThumb>& newThumbs);
    
    ofShortPixels thumbnailDepthRaw;

	//only called during playback
	void update(ofEventArgs& args);

	ofMutex backLock; // to protect backThumbs
    vector<ofxTLVideoThumb> backThumbs; //used to generate thumbs on the back thread, then copies them onto the main thread

	void threadedFunction();
    void exit(ofEventArgs& args);
	
//    ofRange thumbnailUpdatedZoomLevel;
//    float thumbnailUpdatedWidth;
//    float thumbnailUpdatedHeight;
    

	
	//void generateVideoThumbnails();
//	void generateThumbnailForFrame(int index);
	bool frameIsNew;
	
	string sequenceDirectory;
    
};
