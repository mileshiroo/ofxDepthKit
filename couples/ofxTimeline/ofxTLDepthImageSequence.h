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

	virtual bool mousePressed(ofMouseEventArgs& args, long millis);
	virtual void mouseDragged(ofMouseEventArgs& args, long millis);
	virtual void mouseReleased(ofMouseEventArgs& args, long millis);
	virtual void keyPressed(ofKeyEventArgs& args);

	bool loadSequence();
	bool loadSequence(string sequenceDirectory);
    
    void setSequence(ofPtr<ofxDepthImageSequence> newSequence);
    void setSequence(ofxDepthImageSequence& newSequence);
    ofPtr<ofxDepthImageSequence> getDepthImageSequence();
	ofImage& getCurrentDepthImage();
	

	
	bool isLoaded();
	bool isFrameNew();

	//will update with scrubbing and playing
	//turn this off if you are syncing it to a video source
	void setAutoUpdate(bool autoUpdate);
	bool getAutoUpdate();
	
	void playbackStarted(ofxTLPlaybackEventArgs& args);
	void playbackEnded(ofxTLPlaybackEventArgs& args);
	void playbackLooped(ofxTLPlaybackEventArgs& args);
	
	int getSelectedFrame();
	
	int frameForTime(long timeInMillis);
	void selectFrame(int frame);
	void selectTimeInSeconds(float timeInSeconds);
	void selectTimeInMillis(long timeStampInMillis);
	
	//only works if doFramesHaveMillis is true
	unsigned long getSelectedTimeInMillis();
	unsigned long getDurationInMillis();
	
	void toggleThumbs();
	
	bool doFramesHaveTimestamps();
	
	//sets the time offset to the difference in the playhead,
	//so if you hit play after this, the current relationship will be maintained
	void setTimeOffsetToPlayhead();
	void setTimeOffsetInMillis(unsigned long millis);
	unsigned long getTimeOffsetInMillis();
	
  protected:
    ofPtr<ofxDepthImageSequence> depthImageSequence;

    //width and height of image elements
    float getContentWidth();
    float getContentHeight();
	void framePositionsUpdated(vector<ofxTLVideoThumb>& newThumbs);
    
	bool autoUpdate;
	unsigned long timeOffsetInMillis;
    ofShortPixels thumbnailDepthRaw;

	//only called during playback
	void update(ofEventArgs& args);

	ofMutex backLock; // to protect backThumbs
    vector<ofxTLVideoThumb> backThumbs; //used to generate thumbs on the back thread, then copies them onto the main thread

	void threadedFunction();
    void exit(ofEventArgs& args);
	ofImage currentDepthImage;
	bool frameIsNew;
	bool depthImageIsDirty;
	string sequenceDirectory;
    
};
