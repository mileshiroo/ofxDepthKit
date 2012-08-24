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

class ofxTLDepthImageSequence : public ofxTLImageTrack {
  public:	
	ofxTLDepthImageSequence();
	~ofxTLDepthImageSequence();

	void setup();
	void draw();

	void enable();
	void disable();

//	vector<ofxTLVideoThumb> videoThumbs;

//	virtual void mousePressed(ofMouseEventArgs& args, long millis);
//	virtual void mouseMoved(ofMouseEventArgs& args, long millis);
	virtual void mouseDragged(ofMouseEventArgs& args, long millis);
	virtual void mouseReleased(ofMouseEventArgs& args, long millis);

	virtual void keyPressed(ofKeyEventArgs& args);
	
//	virtual void zoomStarted(ofxTLZoomEventArgs& args);
//	virtual void zoomDragged(ofxTLZoomEventArgs& args);
//	virtual void zoomEnded(ofxTLZoomEventArgs& args);

//	virtual void drawRectChanged();

	bool loadSequence();
	bool loadSequence(string sequenceDirectory);
    
    void setSequence(ofPtr<ofxDepthImageSequence> newSequence);
    void setSequence(ofxDepthImageSequence& newSequence);
    
	bool isLoaded();
	bool isFrameNew();
    
	void playbackStarted(ofxTLPlaybackEventArgs& args);
	void playbackEnded(ofxTLPlaybackEventArgs& args);
	void playbackLooped(ofxTLPlaybackEventArgs& args);
	
	ofImage currentDepthImage;
//    ofShortPixels currentDepthRaw;
//    ofShortPixels thumbnailDepthRaw;
	
	int getSelectedFrame();
	
	int frameForTime(long timeInMillis);
	void selectFrame(int frame);
	void selectTime(float timeInSeconds);
	void selectTime(long timeStampInMillis);
	
	//only works if doFramesHaveMillis is true
	long getSelectedTimeInMillis();
	
	void toggleThumbs();
	
	bool doFramesHaveTimestamps();

  protected:
    ofPtr<ofxDepthImageSequence> depthImageSequence;
    
    //bool framesHaveTimestamps;
//    bool canCalculateThumbs() = 0;
    
    //width and height of image elements
    float getContentWidth();
    float getContentHeight();
	void framePositionsUpdated(vector<ofxTLVideoThumb>& newThumbs);
    
    ofShortPixels thumbnailDepthRaw;

	//only called during playback
	void update(ofEventArgs& args);
	
    ofRange thumbnailUpdatedZoomLevel;
    float thumbnailUpdatedWidth;
    float thumbnailUpdatedHeight;
    
	bool thumbsEnabled; //TODO: move to super
	
//	void calculateFramePositions();
	void generateVideoThumbnails();
	void generateThumbnailForFrame(int index);
	bool frameIsNew;
	
	string sequenceDirectory;
//	string thumbDirectory;
	
//	ofxDepthImageCompressor decoder;
    
};
